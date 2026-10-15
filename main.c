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

    //On screen feedback status message & color
    char authStatusMsg[100] = "\0";
    Color authStatusColor = RED;

    //active loggin in player  profile struct
    Player loggedInPlayer = {0};

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
            //collision detection flags
            bool isHoverUsername = CheckCollisionPointRec(mousePoint, usernameBox);
            bool isHoverPassword = CheckCollisionPointRec(mousePoint, passwordBox); 
            bool isHoverLogin = CheckCollisionPointRec(mousePoint, loginBtn);
            bool isHoverRegister = CheckCollisionPointRec(mousePoint, registerBtn);

            //changing cursor visual state over interactive elements
            if (isHoverUsername || isHoverPassword){
                SetMouseCursor(MOUSE_CURSOR_IBEAM);
            }
            else if(isHoverLogin || isHoverRegister){
                SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);
            }
            else{
                SetMouseCursor(MOUSE_CURSOR_DEFAULT);
            }

            if(GetTime() > 0.3 && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)){

                //text field selection
                if(isHoverUsername){
                    activeField = 1;
                    authStatusMsg[0] = '\0'; //clear status message on interaction
                }
                else if(isHoverPassword){
                    activeField = 2;
                    authStatusMsg[0] = '\0';
                }
                //login button logic
                else if(isHoverLogin){
                    if(usernameLetterCount == 0 || passwordLetterCount == 0){
                        strcpy(authStatusMsg,"Please fill in all fields!");
                        authStatusColor = RED;
                    }else if(loginUser(usernameBuf,passwordBuf, &loggedInPlayer)){
                        strcpy(authStatusMsg,"Login Successful!");
                        authStatusColor = GREEN;
                        currentScreen = SCREEN_MENU;
                    }else{
                        strcpy(authStatusMsg,"Invalid username or password.");
                        authStatusColor = RED;
                    }
                }
                //register button logic
                else if (isHoverRegister){
                    if(usernameLetterCount == 0 || passwordLetterCount==0){
                        strcpy(authStatusMsg, "Please fill in all fields!");
                        authStatusColor = RED;
                    }else{
                        int regStatus = registerUser(usernameBuf, passwordBuf);
                        if (regStatus == 1){
                            strcpy(authStatusMsg, "Account created! You can now login.");
                            authStatusColor = GREEN;
                        }else if(regStatus == -1){
                            strcpy(authStatusMsg, "Username already exists!");
                            authStatusColor = RED;
                        }else{
                            strcpy(authStatusMsg, "Error saving account!");
                            authStatusColor = RED;
                        }
                    }
                }
                else{
                    activeField = 0; //deselects if clicking outside
                }//end inner most if
            }//endif

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

        //----BEGIN DRAWING FRAME BUFFER-----
        BeginDrawing();
        ClearBackground(RAYWHITE);

        //authentication screen rendering
        if (currentScreen == SCREEN_AUTH) {
            // main title and section header
            const char *title = "HANGMAN GAME";
            DrawText(title, (screenWidth-MeasureText(title,34))/2, 80, 34, DARKBLUE);
            const char *subtitle = "Player Portal";
            DrawText(subtitle, (screenWidth-MeasureText(subtitle,20))/2, 125, 20, GRAY);

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

            //On-screen feedback msg
            if(strlen(authStatusMsg)> 0){
                int msgWidth = MeasureText(authStatusMsg,18);
                DrawText(authStatusMsg, (screenWidth-msgWidth)/2, 395, 18, authStatusColor);
            }
        }
        else if(currentScreen == SCREEN_MENU){
            //Main menu state - active player profile display 
            const char *menuTitle = "MAIN MENU";
            DrawText(menuTitle, (screenWidth-MeasureText(menuTitle,32))/2, 150,32, DARKBLUE);
            const char *welcomeStr = TextFormat("Welcome, %s!", loggedInPlayer.username);
            DrawText(welcomeStr,(screenWidth-MeasureText(welcomeStr, 22))/2,220,22,DARKGRAY);
            const char *statsStr = TextFormat("Wins: %d  |  Losses: %d  |  Score: %d",loggedInPlayer.wins,loggedInPlayer.losses,loggedInPlayer.score);
            DrawText(statsStr,(screenWidth-MeasureText(statsStr, 20))/2,260,20,GRAY);
            const char *exitHint = "Press ESC to exit application";
            DrawText(exitHint, (screenWidth-MeasureText(exitHint,18))/2, 340, 18, LIGHTGRAY);
        }
        //output rendered frame to screen
        EndDrawing();
    }
    
    //clean up window memory before program termination
    CloseWindow();
    return 0;
}
