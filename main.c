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

//Game state variables
char secretWord[50] = "";
char displayWord[50] = "";
bool guessedLetters[26] = { false };
int wrongAttempts = 0;
const int maxAttempts = 6;
bool gameOver = false;
bool gameWon = false;

//reset board state and fetch a fresh word from backend [DATE: 10/20/2026][MODIFIED BY: Rameez Ahmed]
void startNewGame(void){
    if(!getRandomWord(secretWord)){
        strcpy(secretWord, "HANGMAN"); //Fallback if words.txt fails to open
    }//endif

    int len = strlen(secretWord);
    for(int i = 0;i<len;i++){
        displayWord[i] = '_';
    }//endfor
    displayWord[len] = '\0';

    for(int i=0;i<26;i++){
        guessedLetters[i] = false;
    }//endfor
    
    wrongAttempts = 0;
    gameOver = false;
    gameWon = false;

}//end function


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
    Rectangle playBtn = {310,290,180,50};
    Rectangle backBtn = {310,480,180,40};
    
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
        else if(currentScreen == SCREEN_MENU){
            bool isHoverPlay = CheckCollisionPointRec(mousePoint, playBtn);
            if(isHoverPlay){
                SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);
                if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)){
                    startNewGame();
                    currentScreen = SCREEN_GAME;
                }
            }else{
                SetMouseCursor(MOUSE_CURSOR_DEFAULT);
            }
        }
        else if (currentScreen == SCREEN_GAME){
            bool isHoverBack = CheckCollisionPointRec(mousePoint,backBtn);
            if(isHoverBack){
                SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);
                if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)){
                    currentScreen = SCREEN_MENU;
                }
            }else{
                SetMouseCursor(MOUSE_CURSOR_DEFAULT);
            }

            //game input processing
            if(!gameOver){
                int key = GetCharPressed();
                while(key>0){
                    if(key>='a' && key <= 'z') key -= 32; //converting to uppercase
                    
                    if(key>='A'&& key<= 'Z'){
                        int letterIdx = key - 'A';
                        if (!guessedLetters[letterIdx]){
                            guessedLetters[letterIdx] = true;

                            bool match = false;
                            int len = strlen(secretWord);
                            for(int i = 0;i<len;i++){
                                if(secretWord[i] == key){
                                    displayWord[i] = key;
                                    match = true;
                                }//endif
                            }//endfor

                            if (!match){
                                wrongAttempts++;
                                if(wrongAttempts >= maxAttempts){
                                    gameOver = true;
                                    gameWon = false;
                                }//endif
                            }else if(strcmp(secretWord, displayWord) == 0){
                                gameOver = true;
                                gameWon = true;
                            }
                        }//endif
                    }//endif
                    key = GetCharPressed();
                }//endwhile
            }else{
                //press 'R' to restart game after win or loss
                if(IsKeyPressed(KEY_R)){
                    startNewGame();
                }//endif
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
        //Main menu rendering
        else if(currentScreen == SCREEN_MENU){
            //Main menu state - active player profile display 
            const char *menuTitle = "MAIN MENU";
            DrawText(menuTitle, (screenWidth-MeasureText(menuTitle,32))/2, 150,32, DARKBLUE);

            const char *welcomeStr = TextFormat("Welcome, %s!", loggedInPlayer.username);
            DrawText(welcomeStr,(screenWidth-MeasureText(welcomeStr, 22))/2,220,22,DARKGRAY);
            
            const char *statsStr = TextFormat("Wins: %d  |  Losses: %d  |  Score: %d",loggedInPlayer.wins,loggedInPlayer.losses,loggedInPlayer.score);
            DrawText(statsStr,(screenWidth-MeasureText(statsStr, 20))/2,260,20,GRAY);
            
            const char *exitHint = "Press ESC to exit application";
            DrawText(exitHint, (screenWidth-MeasureText(exitHint,18))/2, 400, 18, LIGHTGRAY);

            //Play game action button
            DrawRectangleRec(playBtn, DARKBLUE);
            const char *btnText = "PLAY GAME";
            int btnTextWidth = MeasureText(btnText,20);
            DrawText(btnText, (int)playBtn.x + ((int)playBtn.width-btnTextWidth)/2, (int)playBtn.y + 15,20,WHITE);

        }
        //Gameplay screen rendering
        else if(currentScreen == SCREEN_GAME){
            const char *gameTitle = "HANGMAN GAMEPLAY";
            DrawText(gameTitle, (screenWidth-MeasureText(gameTitle,28))/2,50,28,DARKBLUE);

            //Display secret word with space padding (e.g. "_ _ A _ _")
            char formattedWord[100] = "";
            int len = strlen(displayWord);
            for (int i = 0; i<len;i++){
                char temp[3] = { displayWord[i], ' ', '\0'};
                strcat(formattedWord, temp);
            }//endfor
            DrawText(formattedWord, (screenWidth-MeasureText(formattedWord, 36))/2,150,36,BLACK);

            //Attempts counter
            const char *attemptsStr = TextFormat("Wrong Attempts: %d / %d", wrongAttempts, maxAttempts);
            DrawText(attemptsStr, (screenWidth-MeasureText(attemptsStr,20))/2,230,20,(wrongAttempts >=4)?RED:DARKGRAY);

            //Display Guessed Keyboard Letters
            char guessedStr[100] = "Guessed: ";
            for(int i = 0; i<26;i++){
                if(guessedLetters[i]){
                    char ltr[3] = { (char)('A' + i ), ' ', '\0'};
                    strcat(guessedStr, ltr);
                }//endif
            }//endfor
            DrawText(guessedStr, (screenWidth-MeasureText(guessedStr, 18))/2,290,18,GRAY);

            //Game over/victory banner
            if(gameOver){
                if(gameWon){
                    const char *winText = "YOU WON! Press 'R' to play again.";
                    DrawText(winText, (screenWidth-MeasureText(winText,22))/2,350,22,GREEN);
                }else{
                    const char *lossText = TextFormat("GAME OVER! Word was: %s. Press 'R' to try again.",secretWord);
                    DrawText(lossText, (screenWidth-MeasureText(lossText,20))/2,350,20,RED);
                }
            }else{
                DrawText("Type any letter A-Z on your keyboard to guess!", (screenWidth-MeasureText("Type any letter A-Z on your keyboard to guess!",16))/2,350,16,LIGHTGRAY);
            }

            //Back to Menu Button
            DrawRectangleRec(backBtn, GRAY);
            const char *backText = "BACK TO MENU";
            int backTextWidth = MeasureText(backText,16);
            DrawText(backText, (int)backBtn.x + ((int)backBtn.width-backTextWidth)/2, (int)backBtn.y + 12,16,WHITE);
        }
        //output rendered frame to screen
        EndDrawing();
    }
    
    //clean up window memory before program termination
    CloseWindow();
    return 0;
}
