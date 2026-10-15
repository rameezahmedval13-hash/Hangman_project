/*=============================================================================
 * PROJECT: Hangman Game
 * COURSE: Programming Fundamentals (LAB)
 * SECTION: BDS-1A
 * 
 * GROUP MEMBERS:
 *   1. Rameez Ahmed    (26K-2527)
 *   2. Muhammad Usaid  (26K-2500
 * 
 * FILE: main.c
 * DESCRIPTION: Main entry point and GUI interface using Raylib. Handles window
 *              rendering, user input, screen navigation state machine, and 
 *              bridges the front-end with backend data persistence.
 *=============================================================================*/

#include "raylib.h"
#include "backend.h"
#include <string.h>

//enum datatype to manage screen state transitions
typedef enum {
    SCREEN_AUTH = 0,
    SCREEN_MENU,
    SCREEN_GAME
} GameScreen;

int main(void) {
    //window initialization
    const int screenWidth = 800;
    const int screenHeight = 600;

    //creating display window with dimensions and title
    InitWindow(screenWidth, screenHeight, "Hangman Game");
    SetTargetFPS(60); //capping rendering speed to 60 FPS

    //Track active application screen: default authentication portal
    GameScreen currentScreen = SCREEN_AUTH;

    //text input buffers and char counters
    char usernameBuf[50] = "\0";
    char passwordBuf[50] = "\0";
    int usernameLetterCount = 0;
    int passwordLetterCount = 0;

    //field focus state 0: none 1: username box 2: password box
    int activeField = 0;

    //defining boxes for UI 
    Rectangle usernameBox = { 300, 200, 250, 40 };
    Rectangle passwordBox = { 300, 260, 250, 40 };
    Rectangle loginBtn    = { 300, 330, 115, 45 };
    Rectangle registerBtn = { 435, 330, 115, 45 };
    
    //main application loop
    while (!WindowShouldClose()) {
        //tracking real time mouse position
        Vector2 mousePoint = GetMousePosition();

        if (currentScreen == SCREEN_AUTH){
            //field selection via mouse
            if(IsMouseButtonPressed((MOUSE_BUTTON_LEFT))){
                if(CheckCollisionPointRec(mousePoint,usernameBox)){
                    activeField = 1;
                }
                else if(CheckCollisionPointRec(mousePoint, passwordBox)){
                    activeField = 2;
                }
                else{
                    activeField = 0; //deselects if clicking outside
                }//end inner most if
            }//end inner if

            //toggle via tab key
            if (IsKeyPressed(KEY_TAB)){
                activeField = (activeField == 1)?2:1;
            }//end  if

            //keyboard character capture
            int key = GetCharPressed();
            while(key>0){
                //allow printable ASCII characters
                if((key>=32)&&(key<=125)){
                    if(activeField == 1 && usernameLetterCount < 48){
                        usernameBuf[usernameLetterCount] = (char)key;
                        usernameBuf[usernameLetterCount + 1] = '\0';
                        usernameLetterCount++;
                    }
                    else if (activeField == 2 && passwordLetterCount < 48){
                        passwordBuf[passwordLetterCount] = (char)key;
                        passwordBuf[passwordLetterCount + 1] = '\0';
                        passwordLetterCount++;
                    }// end inner if
                }//end if
                key = GetCharPressed(); //Drain buffer queue
            }//endwhile

            //backspace deletion
            if(IsKeyPressed(KEY_BACKSPACE)){
                if(activeField == 1 && usernameLetterCount > 0){
                    usernameLetterCount--;
                    usernameBuf[usernameLetterCount] = '\0';
                }
                else if(activeField == 2 && passwordLetterCount>0){
                    passwordLetterCount--;
                    passwordBuf[passwordLetterCount] = '\0';
                }//end inner if
            }//endif

        }//endif

        //begin drawing frame buffer
        BeginDrawing();
        ClearBackground(RAYWHITE);

        //authentication screen rendering
        if (currentScreen == SCREEN_AUTH) {
            // main title and section header
            DrawText("HANGMAN GAME", 280, 80, 34, DARKBLUE);
            DrawText("Player Portal", 345, 125, 20, GRAY);

            //Text input field labels
            DrawText("Username:", 180, 210, 20, DARKGRAY);
            DrawText("Password:", 180, 270, 20, DARKGRAY);

            //Username input box container
            DrawRectangleRec(usernameBox, LIGHTGRAY);
            DrawRectangleLinesEx(usernameBox, (activeField == 1)? 3:1, (activeField == 1) ? BLUE : GRAY);
            DrawText(usernameBuf, (int)usernameBox.x + 10, (int)usernameBox.y + 10, 20, BLACK);
            //Password input box container
            DrawRectangleRec(passwordBox, LIGHTGRAY);
            DrawRectangleLinesEx(passwordBox, (activeField==2)?3:1, (activeField == 2)?BLUE:GRAY);

            //masking password characters with *
            char maskedPass[50] = "\0";
            for (int i = 0; i<passwordLetterCount; i++){
                maskedPass[i] = '*';
            }//endfor
            maskedPass[passwordLetterCount] = '\0';
            DrawText(maskedPass, (int)passwordBox.x + 10, (int)passwordBox.y + 10, 20, BLACK);

            //Login action button
            DrawRectangleRec(loginBtn, BLUE);
            DrawText("LOGIN", (int)loginBtn.x + 28, (int)loginBtn.y + 12, 18, WHITE);
            //Register action button
            DrawRectangleRec(registerBtn, GREEN);
            DrawText("REGISTER", (int)registerBtn.x + 12, (int)registerBtn.y + 12, 18, WHITE);

        }
        //output rendered frame to screen
        EndDrawing();
    }
    
    //clean up window memory before program termination
    CloseWindow();
    return 0;
}