#include <Arduino.h>            // Required for PlatformIO to access Arduino functions
#include <SoftwareSerial.h>     // Library to create a serial port on any digital pins
#include <DFRobotDFPlayerMini.h> // Library to control the DFPlayer Mini module
#include <Servo.h>              // Library to control servo motors

SoftwareSerial mySoftwareSerial(10, 11); // Create software serial on pin 10 (RX) and 11 (TX)
DFRobotDFPlayerMini myDFPlayer;          // Create DFPlayer object to send commands to
Servo myServo;                           // Create servo object to control the servo

#define LED_PIN    9  // LED is connected to pin 9
#define BUTTON_PIN 2  // Button is connected to pin 2
#define SERVO_PIN  6  // Servo is connected to pin 6

bool lastButtonState = HIGH;
bool buttonState     = HIGH;
unsigned long lastDebounceTime = 0;
unsigned long playStartTime = 0;
const unsigned long DEBOUNCE_DELAY = 50;
const unsigned long SOUND_DURATION = 1000;

const int FADE_DURATION = 500;
int ledBrightness = 0;

enum LedState { LED_OFF, LED_FADING_IN, LED_ON, LED_FADING_OUT };
enum ServoState {SERVO_IDLE, SERVO_RIGHT, SERVO_LEFT};
LedState ledState = LED_OFF;
ServoState servoState = SERVO_IDLE;

unsigned long fadeStartTime = 0; //tracks current fade start time
unsigned long servoMoveTime = 0;
const unsigned long SERVO_MOVE_DURATION = 600;

int wigglesRemaining = 0;
bool wiggleInProgress = false;

void handleLED();
void handleButton();
void wiggle();

void setup(){
    randomSeed(analogRead(0));
    mySoftwareSerial.begin(9600);
    Serial.begin(115200);
    delay(2000);

    pinMode(LED_PIN, OUTPUT); // set led pin as output, controls brightness
    pinMode(BUTTON_PIN, INPUT_PULLUP); // sets button pin as input, reads HIGH when not pressed
    analogWrite(LED_PIN, 0); // sets led to start at zero brightness

    myServo.attach(SERVO_PIN);  // attaches servo obj to pin 6
    myServo.write(0); //moves servo to 0 on startup

    Serial.println(F("Initializing DFPlayer..."));
    if (!myDFPlayer.begin(mySoftwareSerial, false, false)) { // checks DFPLayer init, if no ack and no reset
        Serial.println(F("Not initialized\n"));
        while(true); // freeze program if DFPLayer fails
    }

    Serial.println(F("DFPlayer works"));
    myDFPlayer.volume(20); 

}

void loop(){
    handleButton(); // checks if button is pressed every loop
    handleLED(); // update brightness
    
    if(ledState == LED_ON && (millis() - playStartTime >= SOUND_DURATION)){
        ledState = LED_FADING_OUT;
        fadeStartTime = millis(); // records time fade ou startss
        Serial.println(F("Track finished\n"));
    }
    
    if(servoState == SERVO_RIGHT && millis() - servoMoveTime >= SERVO_MOVE_DURATION){
        myServo.write(0);
        servoState = SERVO_IDLE;
        
        
    }
    if(wigglesRemaining > 0 && servoState == SERVO_IDLE && ledState == LED_OFF){
        wiggle();
        wigglesRemaining--;
    }
}

void handleLED(){
    if(ledState == LED_FADING_IN){
        float progress = (float)(millis() - fadeStartTime) / FADE_DURATION; // calculates fade progress
        
        if(progress >= 1.0){
            progress = 1.0; // caps progress, so brightness does not exceed 255
            ledState = LED_ON; // sets led to on when fade progress is finished
        }

        ledBrightness = (int)(progress * 255);
        analogWrite(LED_PIN, ledBrightness);

    }

    if(ledState == LED_FADING_OUT){
        float progress = (float)(millis() - fadeStartTime) / FADE_DURATION;
        if(progress >= 1.0){
            progress = 1.0;
            ledState = LED_OFF;
        }
        ledBrightness = (int)((1.0 - progress) * 255);
        analogWrite(LED_PIN, ledBrightness);
    }

}

void handleButton(){
    bool reading = digitalRead(BUTTON_PIN); // reads current button state

    if(reading != lastButtonState){
        lastDebounceTime = millis(); // records time of button state change for deb
    }

    if((millis() - lastDebounceTime) > DEBOUNCE_DELAY){
        if(reading != buttonState){
            buttonState = reading; // updates button state if changed and debounce time has passed, debounce handles button bounce from a single press
            
            if(buttonState == LOW && ledState == LED_OFF){ // checks if button is pressed and led is not already on to prevent multiple triggers from a single press
                //Serial.println(F("Button pressed - playing track 1\n"));
                    int catch_attempt = rand() % 100 + 1;
                    
                    if(catch_attempt <= 10) {
                        wigglesRemaining = 4;
                    } else if(catch_attempt <= 20){
                        wigglesRemaining = 3;
                    } else if(catch_attempt <= 40){
                        wigglesRemaining = 2;
                    } else if(catch_attempt <= 80){
                        wigglesRemaining = 1;
                    } else {
                        wigglesRemaining = 0;
                    }

                    if(wigglesRemaining > 0){
                        wiggle();
                        wigglesRemaining--;
                    }
            }
        }
    }

    lastButtonState = reading; // updates last button state for next loop
}

void wiggle(){
    myDFPlayer.play(1);
    playStartTime = millis(); //records time sounds start for timing LED
    ledState = LED_FADING_IN; //starts fding the led in
    fadeStartTime = millis();
    myServo.write(180);
    servoMoveTime = millis();
    servoState = SERVO_RIGHT;
}