#include <iostream>
#include <cstdlib>

using namespace std;

int main(){

    char input;
    int catch_attempt = 0;

    while(true){
        cout << "Start Catching? (y/n): ", cin >> input;
        if(input == 'y'){
            catch_attempt = rand() % 100 + 1; // rand num between 1 and 100
            if(catch_attempt <= 5){
                cout << "Four wiggles.... Pokemon Caught!\n";
                break;
            } else if(catch_attempt <= 15){
                cout << "Three wiggles... Escaped!\n";
            } else if(catch_attempt <= 30){
                cout << "Two wiggles.. Escaped!\n";
            } else if(catch_attempt <= 75){
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