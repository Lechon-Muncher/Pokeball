#include <Arduino.h>
#include <SoftwareSerial.h>
#include <DFRobotDFPlayerMini.h>
#include <Servo.h>

SoftwareSerial mySoftwareSerial(10, 11);
DFRobotDFPlayerMini myDFPlayer;
Servo myServo;

#define LED_PIN    9
#define BUTTON_PIN 2
#define SERVO_PIN  6

bool lastButtonState = HIGH;
bool buttonState     = HIGH;
unsigned long lastDebounceTime = 0;
unsigned long playStartTime = 0;
const unsigned long DEBOUNCE_DELAY = 50;
const unsigned long LED_ON_DURATION = 1500;

const unsigned long CATCH_SOUND_DURATION = 1500;
bool playingCatchSound = false;
unsigned long catchSoundStart = 0;

const unsigned long ESCAPE_SOUND_DURATION = 5000;
bool playingEscapeSound = false;
unsigned long escapeSoundStart = 0;
bool attemptMade = false;

const int FADE_DURATION = 500;
int ledBrightness = 0;

enum LedState { LED_OFF, LED_FADING_IN, LED_ON, LED_FADING_OUT };
enum ServoState { SERVO_IDLE, SERVO_WIGGLING };
LedState ledState = LED_OFF;
ServoState servoState = SERVO_IDLE;

unsigned long fadeStartTime = 0;

bool caught = false;

unsigned long wiggleWaitStart = 0;
const unsigned long WIGGLE_WAIT = 500;
bool waitingBetweenWiggles = false;
int wigglesRemaining = 0;

// Wiggle internals
unsigned long wiggleStepTime = 0;        // tracks when the last wiggle step happened
const unsigned long WIGGLE_STEP = 100;   // how long each wiggle position is held in ms
int wiggleStepsRemaining = 0;            // how many individual steps are left in this wiggle
bool wiggleGoingRight = true;            // tracks which direction servo is currently moving

void handleLED();
void handleButton();
void startWiggle();
void handleWiggle();

void setup() {
    randomSeed(analogRead(0));
    mySoftwareSerial.begin(9600);
    Serial.begin(115200);
    delay(2000);

    pinMode(LED_PIN, OUTPUT);
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    analogWrite(LED_PIN, 0);

    myServo.attach(SERVO_PIN);
    myServo.write(90); // start at center position

    Serial.println(F("Initializing DFPlayer..."));
    if (!myDFPlayer.begin(mySoftwareSerial, false, false)) {
        Serial.println(F("Not initialized"));
        while (true);
    }

    Serial.println(F("DFPlayer works"));
    myDFPlayer.volume(20);
}

void loop() {
    handleButton();
    handleLED();
    handleWiggle(); // handles the wiggle steps over time

    if (ledState == LED_ON && (millis() - playStartTime >= LED_ON_DURATION)) {
        ledState = LED_FADING_OUT;
        fadeStartTime = millis();
        Serial.println(F("Track finished"));
    }

    if (wigglesRemaining > 0 && servoState == SERVO_IDLE && ledState == LED_OFF) {
        if (!waitingBetweenWiggles) {
            wiggleWaitStart = millis();
            waitingBetweenWiggles = true;
        } else if (millis() - wiggleWaitStart >= WIGGLE_WAIT) {
            startWiggle();
            wigglesRemaining--;
            waitingBetweenWiggles = false;
        }
    }

    if (playingCatchSound && millis() - catchSoundStart >= CATCH_SOUND_DURATION) {
        playingCatchSound = false;
    }

    if (caught && wigglesRemaining == 0 && servoState == SERVO_IDLE && ledState == LED_OFF && !playingCatchSound) {
        myDFPlayer.play(2);
        catchSoundStart = millis();
        playingCatchSound = true;
        caught = false;
        attemptMade = false;
    }

    if (playingEscapeSound && millis() - escapeSoundStart >= ESCAPE_SOUND_DURATION) {
        playingEscapeSound = false;
    }

    if (attemptMade && !caught && wigglesRemaining == 0 && servoState == SERVO_IDLE && ledState == LED_OFF && !playingEscapeSound){
        myDFPlayer.play(3);
        escapeSoundStart = millis();
        playingEscapeSound = true;
        attemptMade = false;
    }
}

void handleLED() {
    if (ledState == LED_FADING_IN) {
        float progress = (float)(millis() - fadeStartTime) / FADE_DURATION;
        if (progress >= 1.0) { progress = 1.0; ledState = LED_ON; }
        ledBrightness = (int)(progress * 255);
        analogWrite(LED_PIN, ledBrightness);
    }

    if (ledState == LED_FADING_OUT) {
        float progress = (float)(millis() - fadeStartTime) / FADE_DURATION;
        if (progress >= 1.0) { progress = 1.0; ledState = LED_OFF; }
        ledBrightness = (int)((1.0 - progress) * 255);
        analogWrite(LED_PIN, ledBrightness);
    }
}

void handleButton() {
    bool reading = digitalRead(BUTTON_PIN);

    if (reading != lastButtonState) {
        lastDebounceTime = millis();
    }

    if ((millis() - lastDebounceTime) > DEBOUNCE_DELAY) {
        if (reading != buttonState) {
            buttonState = reading;

            if (buttonState == LOW && ledState == LED_OFF && servoState == SERVO_IDLE && wigglesRemaining == 0) {
                attemptMade = true;
                int catch_attempt = rand() % 100 + 1;

                if (catch_attempt <= 10) {
                    wigglesRemaining = 4;
                    caught = true;
                } else if (catch_attempt <= 20) {
                    wigglesRemaining = 3;
                } else if (catch_attempt <= 40) {
                    wigglesRemaining = 2;
                } else if (catch_attempt <= 80) {
                    wigglesRemaining = 1;
                } else {
                    wigglesRemaining = 0;
                }
            }
        }
    }

    lastButtonState = reading;
}

void startWiggle() {
    myDFPlayer.play(1);                // play wiggle sound
    playStartTime = millis();          // record sound start time
    ledState = LED_FADING_IN;          // start fading LED in
    fadeStartTime = millis();          // record fade start time
    servoState = SERVO_WIGGLING;       // mark servo as wiggling
    wiggleStepsRemaining = 6;          // 6 steps = 3 full back and forth swings (increase for more wiggles)
    wiggleGoingRight = true;           // start by going right to 120
    wiggleStepTime = millis();         // record when first step starts
    myServo.write(120);                // move to first position immediately
}

void handleWiggle() {
    if (servoState != SERVO_WIGGLING) return; 

    if (millis() - wiggleStepTime >= WIGGLE_STEP) { // ff enough time has passed for next step
        wiggleStepTime = millis();                   // reset step timer

        if (wiggleStepsRemaining > 0) {
            wiggleGoingRight = !wiggleGoingRight;        // alternate direction
            myServo.write(wiggleGoingRight ? 130 : 50);  // move to 120 or 60 depending on direction
            wiggleStepsRemaining--;                      // count down steps
        } else {
            myServo.write(90);       // return to center when done
            servoState = SERVO_IDLE; // mark servo as idle
        }
    }
}