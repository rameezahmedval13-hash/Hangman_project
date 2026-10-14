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

    //defining boxes for UI 
    Rectangle usernameBox = { 300, 200, 250, 40 };
    Rectangle passwordBox = { 300, 260, 250, 40 };
    Rectangle loginBtn    = { 300, 330, 115, 45 };
    Rectangle registerBtn = { 435, 330, 115, 45 };
    
    //main application loop
    while (!WindowShouldClose()) {
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
            DrawRectangleLines((int)usernameBox.x, (int)usernameBox.y, (int)usernameBox.width, (int)usernameBox.height, GRAY);
            //Password input box container
            DrawRectangleRec(passwordBox, LIGHTGRAY);
            DrawRectangleLines((int)passwordBox.x, (int)passwordBox.y, (int)passwordBox.width, (int)passwordBox.height, GRAY);

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
