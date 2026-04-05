#include <iostream>
#include <cstdlib>
#include <ctime>

using namespace std;

int main(){

    char input;
    int catch_attempt = 0;

    while(true){
        srand(time(0)); // seed for true randomness
        cout << "Start Catching? (y/n): ", cin >> input;
        if(input == 'y'){
            catch_attempt = rand() % 100 + 1; // rand num between 1 and 100
            if(catch_attempt <= 10){
                cout << "Four wiggles.... Pokemon Caught!\n";
                break;
            } else if(catch_attempt <= 20){
                cout << "Three wiggles... Escaped!\n";
            } else if(catch_attempt <= 40){
                cout << "Two wiggles.. Escaped!\n";
            } else if(catch_attempt <= 80){
                cout << "One wiggle. Escaped!\n";
            } else {
                cout << "No wiggles it escaped!\n";
            }
            cout << catch_attempt << "\n";
        } else{
            cout << "Exiting...\n";
            exit(0);
        }
    }
    
    
    return 0;

}