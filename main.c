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
  * LAST MODIFIED BY: Rameez Ahmed, Muhammad Usaid
 * DESCRIPTION: Main entry point and GUI interface using Raylib. Handles window
 *              rendering, user input, screen navigation, and player statistics
 *              updates through backend data persistence.
 *=============================================================================*/

#include "raylib.h"
#include "backend.h"
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>

//enum datatype to manage screen state transitions
typedef enum {
    SCREEN_AUTH = 0,
    SCREEN_MENU,
    SCREEN_CATEGORY,
    SCREEN_DIFFICULTY,
    SCREEN_GAME,
    SCREEN_LEADERBOARD
} GameScreen;

// Difficulty levels [DATE: 11/06/2026][MODIFIED BY: MUHAMMAD USAID]
typedef enum {
    DIFFICULTY_EASY = 0,
    DIFFICULTY_MEDIUM,
    DIFFICULTY_HARD
} Difficulty;

// Category Selection [DATE: 11/07/2026][MODIFIED BY: MUHAMMAD USAID]
const char *categoryNames[12] = {
    "TECH", "MATH", "SCIENCE", "ANIMALS",
    "NATURE", "GEOGRAPHY", "PLACES", "OBJECTS",
    "PEOPLE", "JOBS", "SOCIETY", "ARTS"
};

int selectedCategory = 0;
Difficulty selectedDifficulty = DIFFICULTY_MEDIUM;

//Game state variables
char secretWord[50] = "";
char displayWord[50] = "";
bool guessedLetters[26] = { false };
int wrongAttempts = 0;
const int maxAttempts = 6;
bool gameOver = false;
bool gameWon = false;

//Player stats update tracking [DATE: 11/05/2026 MODIFIED BY: MUHAMMAD USAID]
bool statsUpdated = false;

// Hint balance for the currently logged in player
int availableHints = 3;
long long nextHintRecharge = 0;

//function definition [DATE: 11/06/2026][MODIFIED BY: MUHAMMAD USAID]
void DrawHangman(int strikes){

    int baseX = 520;
    int baseY = 150;

    //Drawing gallows
    DrawLine(baseX, baseY + 300, baseX + 150, baseY + 300, DARKBROWN);
    DrawLine(baseX + 75, baseY + 300, baseX + 75, baseY, DARKBROWN);
    DrawLine(baseX + 75, baseY, baseX + 200, baseY, DARKBROWN);
    DrawLine(baseX + 200, baseY, baseX + 200, baseY + 30, BROWN);

    //Drawing hangman based on wrong attempts
    if(strikes >= 1) DrawCircle(baseX + 200, baseY + 50, 20, BLACK);
    if(strikes >= 2) DrawLine(baseX + 200, baseY + 70, baseX + 200, baseY + 150, BLACK);
    if(strikes >= 3) DrawLine(baseX + 200, baseY + 90, baseX + 160, baseY + 130, BLACK);
    if(strikes >= 4) DrawLine(baseX + 200, baseY + 90, baseX + 240, baseY + 130, BLACK);
    if(strikes >= 5) DrawLine(baseX + 200, baseY + 150, baseX + 170, baseY + 220, BLACK);
    if(strikes >= 6) DrawLine(baseX + 200, baseY + 150, baseX + 230, baseY + 220, BLACK);

}//end function

// Select word by category and difficulty
// [DATE: 11/07/2026][MODIFIED BY: MUHAMMAD USAID]
int getWordByCategoryAndDifficulty(char *outWord,Difficulty difficulty,const char *category){
    if(outWord == NULL || category == NULL){
        return 0;
    }
    int minLength, maxLength;
    if(difficulty == DIFFICULTY_EASY){
        minLength = 4;
        maxLength = 5;
    }
    else if(difficulty == DIFFICULTY_MEDIUM){
        minLength = 6;
        maxLength = 8;
    }
    else{
        minLength = 9;
        maxLength = 49;
    }
    FILE *file = fopen("words.txt", "r");
    if(file == NULL){
        return 0;
    }
    char word[50];
    char wordCategory[20];
    char selectedWord[50];
    int count = 0;
    
    while(fscanf(file, "%49s %19s", word, wordCategory) == 2){
        int length = strlen(word);
        if(strcmp(wordCategory, category) == 0 &&
           length >= minLength && length <= maxLength){
            count++;
            if(rand() % count == 0){
                strcpy(selectedWord, word);
            }
        }
    }
    fclose(file);
    if(count == 0){
        return 0;
    }
    strcpy(outWord, selectedWord);
    return 1;
}

//Generate sound effects in memory [DATE: 11/06/2026][MODIFIED BY: MUHAMMAD USAID]
Sound GenerateTone(int frequency, float duration, float volume){

    Wave wave = {0};

    wave.frameCount = (unsigned int)(44100 * duration);
    wave.sampleRate = 44100;
    wave.sampleSize = 16;
    wave.channels = 1;

    short *samples = malloc(wave.frameCount * sizeof(short));

    if(samples == NULL){
        return (Sound){0};
    }

    int samplesPerCycle = 44100 / frequency;
    for(unsigned int i = 0; i < wave.frameCount; i++){
        int amplitude = (int)(volume * 12000);
        if(i > wave.frameCount * 0.8f){
            amplitude = (int)(amplitude * (wave.frameCount - i) /
                       (wave.frameCount * 0.2f));
        }
        samples[i] = ((i / samplesPerCycle) % 2 == 0)
                     ? (short)amplitude : (short)-amplitude;
    }//endfor
    wave.data = samples;
    Sound generatedSound = LoadSoundFromWave(wave);
    free(samples);
    return generatedSound;
}//end function

//reset board state and fetch a fresh word from backend [DATE: 10/20/2026][MODIFIED BY: Rameez Ahmed]
void startNewGame(void){
    if(!getWordByCategoryAndDifficulty(secretWord,selectedDifficulty,categoryNames[selectedCategory])){
        if(selectedDifficulty == DIFFICULTY_EASY){
            strcpy(secretWord, "APPLE");
        }
        else if(selectedDifficulty == DIFFICULTY_MEDIUM){
            strcpy(secretWord, "POINTER");
        }
        else{
            strcpy(secretWord, "PROGRAMMING");
        }
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

    //Reset stats update flag for new game [DATE: 11/05/2026][MODIFIED BY: MUHAMMAD USAID]
    statsUpdated = false;

}//end function

// Save and update each player's hint balance in hints.txt
typedef struct {
    char username[50];
    int remainingHints;
    long long nextRechargeTime;
} HintRecord;

int processHintData(const char *username, int consumeHint,int *balanceOut, long long *nextRechargeOut){
    HintRecord records[1000];
    int recordCount = 0;
    int playerIndex = -1;

    FILE *file = fopen("hints.txt", "r");

    if(file != NULL){
        char line[128];
        while(fgets(line, sizeof(line), file) != NULL){
            HintRecord record;
            if(sscanf(line, "%49s %d %lld", record.username,&record.remainingHints, &record.nextRechargeTime) == 3){
                if(record.remainingHints < 0) record.remainingHints = 0;
                if(record.remainingHints > 3) record.remainingHints = 3;
                if(record.remainingHints == 3){
                    record.nextRechargeTime = 0;
                }
                if(recordCount < 1000){
                    records[recordCount] = record;
                    if(strcmp(record.username, username) == 0){
                        playerIndex = recordCount;
                    }
                    recordCount++;
                }
            }
        }
        fclose(file);
    }

    // A new player starts with three hints.
    if(playerIndex == -1){
        if(recordCount >= 1000) return 0;
        playerIndex = recordCount;
        strcpy(records[playerIndex].username, username);
        records[playerIndex].remainingHints = 3;
        records[playerIndex].nextRechargeTime = 0;
        recordCount++;
    }

    HintRecord *playerRecord = &records[playerIndex];
    long long currentTime = (long long)time(NULL);
    const long long rechargeInterval = 8LL * 60LL * 60LL;

    // Recharge one hint for every completed eight-hour interval.
    if(playerRecord->remainingHints < 3){
        if(playerRecord->nextRechargeTime <= 0){
            playerRecord->nextRechargeTime = currentTime + rechargeInterval;
        }
        else if(currentTime >= playerRecord->nextRechargeTime){
            long long gained = 1 +
                (currentTime - playerRecord->nextRechargeTime) / rechargeInterval;
            playerRecord->remainingHints += (int)gained;
            if(playerRecord->remainingHints >= 3){
                playerRecord->remainingHints = 3;
                playerRecord->nextRechargeTime = 0;
            }
            else{
                playerRecord->nextRechargeTime += gained * rechargeInterval;
            }
        }
    }

    // Consume only one hint when available.
    if(consumeHint && playerRecord->remainingHints > 0){
        playerRecord->remainingHints--;
        if(playerRecord->remainingHints < 3 &&
           playerRecord->nextRechargeTime <= 0){
            playerRecord->nextRechargeTime = currentTime + rechargeInterval;
        }
    }

    // Save all players' records.
    file = fopen("hints.txt", "w");
    if(file == NULL) return 0;

    for(int i = 0; i < recordCount; i++){
        if(fprintf(file, "%s %d %lld\n", records[i].username,records[i].remainingHints,records[i].nextRechargeTime) < 0){
            fclose(file);
            return 0;
        }
    }
    fclose(file);
    *balanceOut = playerRecord->remainingHints;
    *nextRechargeOut = playerRecord->nextRechargeTime;
    
    return 1;
}

int main(void) {
    //window initialization
    const int screenWidth = 800;
    const int screenHeight = 600;

    //creating display window with dimensions and title
    InitWindow(screenWidth, screenHeight, "Hangman Game");
    SetTargetFPS(60); //capping rendering speed to 60 FPS
    srand((unsigned int)time(NULL));

    //Initialize audio and generate game sounds [DATE: 11/06/2026][MODIFIED BY: MUHAMMAD USAID]
    InitAudioDevice();
    Sound correctSound = GenerateTone(750, 0.12f, 0.5f);
    Sound wrongSound   = GenerateTone(180, 0.20f, 0.5f);
    Sound winSound     = GenerateTone(1000, 0.35f, 0.5f);
    Sound loseSound    = GenerateTone(120, 0.40f, 0.5f);

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

    //Leaderboard data [DATE: 11/06/2026][MODIFIED BY: MUHAMMAD USAID]
    Player leaderboard[1000];
    int leaderboardCount = 0;

    //defining boxes for UI 
    Rectangle usernameBox = { 300, 200, 250, 40 };
    Rectangle passwordBox = { 300, 260, 250, 40 };
    Rectangle loginBtn    = { 300, 330, 115, 45 };
    Rectangle registerBtn = { 435, 330, 115, 45 };
    Rectangle playBtn = {310,290,180,50};
    Rectangle leaderboardBtn = {310,355,180,50};
    Rectangle backBtn = {310,480,180,40};
    Rectangle hintBtn = {30,405,140,40};
    Rectangle leaderboardBackBtn = {310,480,180,40};

    // Difficulty selection buttons
    Rectangle easyBtn = {310, 230, 180, 50};
    Rectangle mediumBtn = {310, 300, 180, 50};
    Rectangle hardBtn = {310, 370, 180, 50};
    Rectangle difficultyBackBtn = {310, 460, 180, 40};

    // Category selection buttons
    // [DATE: 11/07/2026][MODIFIED BY: MUHAMMAD USAID]
    Rectangle categoryBtns[12];

    for(int i = 0; i < 12; i++){
        int col = i % 3;
        int row = i / 3;
        categoryBtns[i] = (Rectangle){100 + col * 210,170 + row * 75,180,50};
    }
    Rectangle categoryBackBtn = {310, 490, 180, 40};

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
                        if(processHintData(loggedInPlayer.username, 0,&availableHints, &nextHintRecharge)){
                            strcpy(authStatusMsg,"Login Successful!");
                            authStatusColor = GREEN;
                            currentScreen = SCREEN_MENU;
                        }
                        else{
                            strcpy(authStatusMsg,"Could not load hints data.");
                            authStatusColor = RED;
                        }
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
            bool isHoverLeaderboard = CheckCollisionPointRec(mousePoint, leaderboardBtn);
            if(isHoverPlay){
                SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);
                if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)){
                    currentScreen = SCREEN_CATEGORY;
                }
            }
            else if(isHoverLeaderboard){
                SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);
                if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)){
                    leaderboardCount = getLeaderboard(leaderboard, 1000);
                    currentScreen = SCREEN_LEADERBOARD;
                }
            }
            else{
                SetMouseCursor(MOUSE_CURSOR_DEFAULT);
            }
        }

        // Category selection screen input handling
        else if(currentScreen == SCREEN_CATEGORY){
            bool hoverCategory = false;
            for(int i = 0; i < 12; i++){
                if(CheckCollisionPointRec(mousePoint, categoryBtns[i])){
                    hoverCategory = true;
                    if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)){
                        selectedCategory = i;
                        currentScreen = SCREEN_DIFFICULTY;
                    }
                }
            }
            bool hoverBack = CheckCollisionPointRec(mousePoint, categoryBackBtn);
            if(hoverBack){
                hoverCategory = true;
                if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)){
                    currentScreen = SCREEN_MENU;
                }
            }
            SetMouseCursor(hoverCategory ?MOUSE_CURSOR_POINTING_HAND : MOUSE_CURSOR_DEFAULT);
        }

        // Difficulty selection screen input handling
        else if(currentScreen == SCREEN_DIFFICULTY){
            bool hoverEasy = CheckCollisionPointRec(mousePoint, easyBtn);
            bool hoverMedium = CheckCollisionPointRec(mousePoint, mediumBtn);
            bool hoverHard = CheckCollisionPointRec(mousePoint, hardBtn);
            bool hoverBack = CheckCollisionPointRec(mousePoint, difficultyBackBtn);
            if(hoverEasy || hoverMedium || hoverHard || hoverBack){
                SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);
            }
            else{
                SetMouseCursor(MOUSE_CURSOR_DEFAULT);
            }
            if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)){
                if(hoverEasy){
                    selectedDifficulty = DIFFICULTY_EASY;
                    startNewGame();
                    currentScreen = SCREEN_GAME;
                }
                else if(hoverMedium){
                    selectedDifficulty = DIFFICULTY_MEDIUM;
                    startNewGame();
                    currentScreen = SCREEN_GAME;
                }
                else if(hoverHard){
                    selectedDifficulty = DIFFICULTY_HARD;
                    startNewGame();
                    currentScreen = SCREEN_GAME;
                }
                else if(hoverBack){
                    currentScreen = SCREEN_CATEGORY;
                }
            }
        }
        else if (currentScreen == SCREEN_GAME){

            // Refresh hints when recharge time is completed.
            if(availableHints < 3 && nextHintRecharge > 0 &&(long long)time(NULL) >= nextHintRecharge){
                if(!processHintData(loggedInPlayer.username, 0,&availableHints, &nextHintRecharge)){
                    TraceLog(LOG_ERROR, "Failed to refresh hint data!");
                }
            }
            bool isHoverBack = CheckCollisionPointRec(mousePoint, backBtn);
            bool isHoverHint = CheckCollisionPointRec(mousePoint, hintBtn);

            if(isHoverBack || isHoverHint){
                SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);
            }
            else{
                SetMouseCursor(MOUSE_CURSOR_DEFAULT);
            }

            // Back button
            if(isHoverBack && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)){

    currentScreen = SCREEN_MENU;
            }

            // Hint button
            else if(isHoverHint && IsMouseButtonPressed(MOUSE_BUTTON_LEFT) &&
        !gameOver && availableHints > 0){

    int hiddenPositions[50];
    int hiddenCount = 0;
    int wordLength = strlen(secretWord);

    // Find all hidden letter positions.
    for(int i = 0; i < wordLength; i++){

        if(displayWord[i] == '_'){
            hiddenPositions[hiddenCount] = i;
            hiddenCount++;
        }
    }

    if(hiddenCount > 0){

        if(processHintData(loggedInPlayer.username, 1,
                           &availableHints, &nextHintRecharge)){

            // Randomly select one hidden position.
            int chosenPosition = hiddenPositions[rand() % hiddenCount];

            char hintLetter = secretWord[chosenPosition];

            guessedLetters[hintLetter - 'A'] = true;

            // Reveal all occurrences of that letter.
            for(int i = 0; i < wordLength; i++){

                if(secretWord[i] == hintLetter){
                    displayWord[i] = hintLetter;
                }
            }

            // Check if the word is completed.
            if(strcmp(secretWord, displayWord) == 0){

                gameOver = true;
                gameWon = true;

                PlaySound(winSound);
            }
            else{
                PlaySound(correctSound);
            }
        }
        else{
            TraceLog(LOG_ERROR, "Failed to update hint data!");
        }
    }
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
                                    PlaySound(loseSound);
                                }
                                else{
                                    PlaySound(wrongSound);
                                }
                            }
                            else if(strcmp(secretWord, displayWord) == 0){
                                gameOver = true;
                                gameWon = true;
                                PlaySound(winSound);
                            }
                            else{
                                PlaySound(correctSound);
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
        
        //Leaderboard screen input handling
        else if(currentScreen == SCREEN_LEADERBOARD){
            bool isHoverBack = CheckCollisionPointRec(mousePoint, leaderboardBackBtn);
            if(isHoverBack){
                SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);
                if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)){
                    currentScreen = SCREEN_MENU;
                }
            }
            else{
                SetMouseCursor(MOUSE_CURSOR_DEFAULT);
            }
        }

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
            DrawText(exitHint, (screenWidth-MeasureText(exitHint,18))/2, 440, 18, LIGHTGRAY);

            //Play game action button
            DrawRectangleRec(playBtn, DARKBLUE);
            const char *btnText = "PLAY GAME";
            int btnTextWidth = MeasureText(btnText,20);
            DrawText(btnText, (int)playBtn.x + ((int)playBtn.width-btnTextWidth)/2, (int)playBtn.y + 15,20,WHITE);

            //Leaderboard action button
            DrawRectangleRec(leaderboardBtn, DARKBLUE);
            const char *leaderboardBtnText = "LEADERBOARD";
            int leaderboardTextWidth = MeasureText(leaderboardBtnText, 20);
            DrawText(leaderboardBtnText,(int)leaderboardBtn.x + ((int)leaderboardBtn.width-leaderboardTextWidth)/2,(int)leaderboardBtn.y + 15,20, WHITE);
        }

        // Category selection screen rendering
        else if(currentScreen == SCREEN_CATEGORY){
            const char *title = "SELECT CATEGORY";
            DrawText(title, (screenWidth - MeasureText(title, 30))/2,100, 30, DARKBLUE);
            for(int i = 0; i < 12; i++){
                DrawRectangleRec(categoryBtns[i], DARKBLUE);
                int textWidth = MeasureText(categoryNames[i], 18);
                DrawText(categoryNames[i],(int)categoryBtns[i].x +((int)categoryBtns[i].width - textWidth)/2,(int)categoryBtns[i].y + 15,18, WHITE);
            }
            DrawRectangleRec(categoryBackBtn, GRAY);
            DrawText("BACK TO MENU", 335, 502, 16, WHITE);
        }

        // Difficulty selection screen rendering
        else if(currentScreen == SCREEN_DIFFICULTY){

            const char *title = "SELECT DIFFICULTY";
            DrawText(title,(screenWidth - MeasureText(title, 30))/2,130, 30, DARKBLUE);

            // Easy button
            DrawRectangleRec(easyBtn, GREEN);
            DrawText("EASY", 370, 245, 20, WHITE);

            // Medium button
            DrawRectangleRec(mediumBtn, BLUE);
            DrawText("MEDIUM", 360, 315, 20, WHITE);

            // Hard button
            DrawRectangleRec(hardBtn, RED);
            DrawText("HARD", 370, 385, 20, WHITE);

            // Back button
            DrawRectangleRec(difficultyBackBtn, GRAY);
            DrawText("BACK TO CATEGORY", 325, 472, 16, WHITE);
            DrawText("Easy: 4-5 letters",320, 205, 16, DARKGRAY);
            DrawText("Medium: 6-8 letters",320, 275, 16, DARKGRAY);
            DrawText("Hard: 9+ letters",320, 345, 16, DARKGRAY);
        }
        //Leaderboard screen rendering
        else if(currentScreen == SCREEN_LEADERBOARD){
            const char *leaderboardTitle = "PLAYER LEADERBOARD";
            DrawText(leaderboardTitle,(screenWidth - MeasureText(leaderboardTitle, 30))/2,90, 30, DARKBLUE);
            DrawText("TOP 5 PLAYERS BY SCORE",(screenWidth - MeasureText("TOP 5 PLAYERS BY SCORE", 18))/2,145, 18, GRAY);

            //Leaderboard column headings
            DrawText("RANK", 120, 190, 18, DARKGRAY);
            DrawText("PLAYER", 210, 190, 18, DARKGRAY);
            DrawText("SCORE", 450, 190, 18, DARKGRAY);
            DrawText("WINS", 560, 190, 18, DARKGRAY);
            DrawText("LOSSES", 660, 190, 18, DARKGRAY);

            if(leaderboardCount == 0){
                const char *emptyMsg = "No player records found.";
                DrawText(emptyMsg,(screenWidth - MeasureText(emptyMsg, 20))/2,270, 20, GRAY);
            }
            else{
                //Displaying top five players
                int displayCount = (leaderboardCount < 5) ? leaderboardCount : 5;
                for(int i = 0; i < displayCount; i++){
                    int rowY = 230 + (i * 45);
                    DrawText(TextFormat("%d", i + 1),130, rowY, 20, BLACK);
                    DrawText(TextFormat("%.14s", leaderboard[i].username),210, rowY, 20, BLACK);
                    DrawText(TextFormat("%d", leaderboard[i].score),450, rowY, 20, BLACK);
                    DrawText(TextFormat("%d", leaderboard[i].wins),560, rowY, 20, BLACK);
                    DrawText(TextFormat("%d", leaderboard[i].losses),670, rowY, 20, BLACK);
                }//endfor
            }//endif

            //Back to Menu Button
            DrawRectangleRec(leaderboardBackBtn, GRAY);
            const char *backText = "BACK TO MENU";
            int backTextWidth = MeasureText(backText, 16);
            DrawText(backText,(int)leaderboardBackBtn.x + ((int)leaderboardBackBtn.width - backTextWidth)/2,(int)leaderboardBackBtn.y + 12,16, WHITE);
        }
        //Gameplay screen rendering
        else if(currentScreen == SCREEN_GAME){
            const char *gameTitle = "HANGMAN GAMEPLAY";
            DrawText(gameTitle, (screenWidth-MeasureText(gameTitle,28))/2,50,28,DARKBLUE);
            const char *difficultyName =
                (selectedDifficulty == DIFFICULTY_EASY) ? "EASY" :
                (selectedDifficulty == DIFFICULTY_MEDIUM) ? "MEDIUM" : "HARD";

            DrawText(TextFormat("Difficulty: %s", difficultyName),30, 100, 18, DARKBLUE);

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

            // Hint balance and recharge countdown
            DrawText(TextFormat("Hints: %d/3", availableHints),30, 380, 18, DARKBLUE);
            DrawRectangleRec(hintBtn,(!gameOver && availableHints > 0) ? BLUE : GRAY);
            DrawText("USE HINT",(int)hintBtn.x + 30,(int)hintBtn.y + 12,16, WHITE);

            // Display remaining recharge time.
            if(availableHints < 3 && nextHintRecharge > 0){
                long long secondsLeft = nextHintRecharge - (long long)time(NULL);
                if(secondsLeft < 0) secondsLeft = 0;
                int hoursLeft = (int)(secondsLeft / 3600);
                int minutesLeft = (int)((secondsLeft % 3600) / 60);
                DrawText(TextFormat("Next hint: %02d:%02d",hoursLeft, minutesLeft), 30, 455, 16, GRAY);
            }

            // Display selected category
            // [DATE: 11/07/2026][MODIFIED BY: MUHAMMAD USAID]
            DrawText(TextFormat("Category: %s", categoryNames[selectedCategory]),30, 125, 18, DARKBLUE);

            //Drawing hangman according to wrong attempts
            DrawHangman(wrongAttempts);

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

            //Updating player stats after game completion [DATE: 11/05/2026][MODIFIED BY: MUHAMMAD USAID]
            if(gameOver && !statsUpdated){

                if(!updatePlayerStats(&loggedInPlayer, gameWon)){
                    TraceLog(LOG_ERROR, "Failed to save player stats!");
                }//endif
               statsUpdated = true;

            }//endif
        }
        //output rendered frame to screen
        EndDrawing();
    }
    
    //Unload audio resources [DATE: 11/06/2026][MODIFIED BY: MUHAMMAD USAID]
    UnloadSound(correctSound);
    UnloadSound(wrongSound);
    UnloadSound(winSound);
    UnloadSound(loseSound);
    CloseAudioDevice();

    //clean up window memory before program termination
    CloseWindow();
    return 0;
}