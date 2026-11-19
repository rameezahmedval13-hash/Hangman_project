/*=============================================================================
 * PROJECT: File Based Hangman Game with Player Profiles & Score History
 * COURSE: Programming Fundamentals
 * SECTION: BDS-1A
 *
 * GROUP MEMBERS:
 *   1. Rameez Ahmed    (26K-2527)
 *   2. Muhammad Usaid  (26K-2500)
 *
 * FILE: main.c
 * LAST MODIFIED BY: Rameez Ahmed, Muhammad Usaid
 * DESCRIPTION: Main file of the Hangman game. Handles the Raylib window,
 *              game screens, player input, and user interface. Includes
 *              category and difficulty selection, Hangman gameplay, hints
 *              with recharge system, sound effects, pause menu, account
 *              switching, leaderboard, and player statistics display.
 *=============================================================================*/

#include "raylib.h"
#include "backend.h"
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>

// Custom Inter font
Font gameFont;
Font semiBoldFont;
int gameFontLoaded = 0;
int semiBoldFontLoaded = 0;

//enum datatype to manage screen state transitions
typedef enum {
    SCREEN_AUTH = 0,
    SCREEN_MENU,
    SCREEN_CATEGORY,
    SCREEN_DIFFICULTY,
    SCREEN_GAME,
    SCREEN_PAUSE,
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

// Draw text using the custom Inter font
void DrawGameText(const char *text, int posX, int posY,int fontSize, Color color){
    DrawTextEx(gameFont,text,(Vector2){(float)posX, (float)posY},(float)fontSize,0.0f,color);
}

// Measure text using the custom Inter font
int MeasureGameText(const char *text, int fontSize){
    Vector2 textSize = MeasureTextEx(gameFont,text,(float)fontSize,0.0f);
    return (int)textSize.x;
}

void DrawBoldText(const char *text, int posX, int posY,int fontSize, Color color){
    DrawTextEx(semiBoldFont,text,(Vector2){(float)posX, (float)posY},(float)fontSize,0.0f,color);
}

// Redirect existing text calls to the custom font
#define DrawText DrawGameText
#define MeasureText MeasureGameText

int main(void) {
    //window initialization
    const int screenWidth = 800;
    const int screenHeight = 600;

    InitWindow(screenWidth, screenHeight, "Hangman Game");
    // Load custom Inter fonts
    if(FileExists("assets/fonts/Inter-Regular.ttf")){
        gameFont = LoadFontEx("assets/fonts/Inter-Regular.ttf", 32, NULL, 0);
        gameFontLoaded = 1;
        SetTextureFilter(gameFont.texture, TEXTURE_FILTER_BILINEAR);
    }
    else{
        gameFont = GetFontDefault();
        TraceLog(LOG_WARNING, "Inter Regular font not found.");
    }
    if(FileExists("assets/fonts/Inter-SemiBold.ttf")){
        semiBoldFont = LoadFontEx("assets/fonts/Inter-SemiBold.ttf", 48, NULL, 0);
        semiBoldFontLoaded = 1;
    }
    else{
        semiBoldFont = gameFont;
        TraceLog(LOG_WARNING, "Inter SemiBold font not found.");
    }
    SetExitKey(KEY_NULL);
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
    GameScreen previousScreen = SCREEN_AUTH;
    bool quitRequested = false;
    bool showQuitConfirmation = false;
    bool showSwitchConfirmation = false;

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

    // Pause Menu and Account Switching buttons
    Rectangle switchAccountBtn = {310, 420, 180, 45};
    Rectangle pauseResumeBtn = {310, 245, 180, 45};
    Rectangle pauseSwitchBtn = {310, 310, 180, 45};
    Rectangle pauseMenuBtn = {310, 375, 180, 45};
    Rectangle pauseQuitBtn = {310, 440, 180, 45};
    Rectangle confirmYesBtn = {240, 330, 140, 42};
    Rectangle confirmNoBtn = {420, 330, 140, 42};

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
    while (!WindowShouldClose() && !quitRequested) {
        //tracking real time mouse position
        Vector2 mousePoint = GetMousePosition();

        // ESC key handling
        if(IsKeyPressed(KEY_ESCAPE)){
            if(currentScreen == SCREEN_MENU){
                quitRequested = true;
            }
            else if(currentScreen == SCREEN_PAUSE){
                if(showSwitchConfirmation){
                    showSwitchConfirmation = false;
                }
                else if(showQuitConfirmation){
                    showQuitConfirmation = false;
                }
                else{
                    currentScreen = previousScreen;
                }
            }
            else{
                previousScreen = currentScreen;
                currentScreen = SCREEN_PAUSE;
                showSwitchConfirmation = false;
                showQuitConfirmation = false;
            }
        }

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
            bool isHoverSwitch = CheckCollisionPointRec(mousePoint, switchAccountBtn);
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
            else if(isHoverSwitch){
                SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);
                if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)){
                    usernameBuf[0] = '\0';
                    passwordBuf[0] = '\0';
                    usernameLetterCount = 0;
                    passwordLetterCount = 0;
                    activeField = 0;
                    authStatusMsg[0] = '\0';
                    memset(&loggedInPlayer, 0, sizeof(loggedInPlayer));
                    currentScreen = SCREEN_AUTH;
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

        // Pause Menu input handling
        else if(currentScreen == SCREEN_PAUSE){
            if(gameOver && !statsUpdated){
                if(!updatePlayerStats(&loggedInPlayer, gameWon)){
                    TraceLog(LOG_ERROR, "Failed to save player stats!");
                }
                statsUpdated = true;
            }
            bool hoverResume = CheckCollisionPointRec(mousePoint, pauseResumeBtn);
            bool hoverSwitch = CheckCollisionPointRec(mousePoint, pauseSwitchBtn);
            bool hoverMenu = CheckCollisionPointRec(mousePoint, pauseMenuBtn);
            bool hoverQuit = CheckCollisionPointRec(mousePoint, pauseQuitBtn);
            bool hoverYes = CheckCollisionPointRec(mousePoint, confirmYesBtn);
            bool hoverNo = CheckCollisionPointRec(mousePoint, confirmNoBtn);

            if(showSwitchConfirmation || showQuitConfirmation){
                SetMouseCursor((hoverYes || hoverNo) ?MOUSE_CURSOR_POINTING_HAND :MOUSE_CURSOR_DEFAULT);
                if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)){
                    if(hoverYes){
                        if(showSwitchConfirmation){
                            usernameBuf[0] = '\0';
                            passwordBuf[0] = '\0';
                            usernameLetterCount = 0;
                            passwordLetterCount = 0;
                            activeField = 0;
                            authStatusMsg[0] = '\0';
                            authStatusColor = RED;
                            memset(&loggedInPlayer, 0, sizeof(loggedInPlayer));
                            secretWord[0] = '\0';
                            displayWord[0] = '\0';

                            for(int i = 0; i < 26; i++){
                                guessedLetters[i] = false;
                            }
                            wrongAttempts = 0;
                            gameOver = false;
                            gameWon = false;
                            statsUpdated = false;
                            availableHints = 3;
                            nextHintRecharge = 0;
                            showSwitchConfirmation = false;
                            currentScreen = SCREEN_AUTH;
                        }
                        else if(showQuitConfirmation){
                            quitRequested = true;
                        }
                    }
                    else if(hoverNo){
                        showSwitchConfirmation = false;
                        showQuitConfirmation = false;
                    }
                }
            }
            else{
                SetMouseCursor((hoverResume || hoverSwitch ||hoverMenu || hoverQuit) ?MOUSE_CURSOR_POINTING_HAND :MOUSE_CURSOR_DEFAULT);

                if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)){
                    if(hoverResume){
                        currentScreen = previousScreen;
                    }
                    else if(hoverSwitch){
                        showSwitchConfirmation = true;
                    }
                    else if(hoverMenu){
                        currentScreen = SCREEN_MENU;
                    }
                    else if(hoverQuit){
                        showQuitConfirmation = true;
                    }
                }
            }
        }

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
            Color pageBg = {245, 244, 239, 255};
            Color panelBg = {255, 255, 252, 255};
            Color navy = {36, 52, 71, 255};
            Color teal = {70, 132, 125, 255};
            Color mutedText = {112, 119, 122, 255};
            Color border = {224, 224, 216, 255};

            ClearBackground(pageBg);

            // Main panel
            Rectangle authPanel = {155, 70, 490, 415};
            DrawRectangleRounded(authPanel, 0.035f, 8, panelBg);
            DrawRectangleLinesEx(authPanel, 1.0f, border);

            // Heading
            const char *title = "HANGMAN GAME";
            DrawBoldText(title,(screenWidth - MeasureText(title, 34))/2,105, 34, navy);
            const char *subtitle = "Player Portal";
            DrawText(subtitle,(screenWidth - MeasureText(subtitle, 18))/2,150, 18, mutedText);

            // Input labels
            DrawText("Username", 180, 210, 19, navy);
            DrawText("Password", 180, 270, 19, navy);

            // Username field
            DrawRectangleRec(usernameBox, WHITE);
            DrawRectangleLinesEx(usernameBox,activeField == 1 ? 2.0f : 1.0f,activeField == 1 ? teal : border);
            DrawText(usernameBuf,(int)usernameBox.x + 10,(int)usernameBox.y + 10,20, navy);

            // Password field
            DrawRectangleRec(passwordBox, WHITE);
            DrawRectangleLinesEx(passwordBox,activeField == 2 ? 2.0f : 1.0f,activeField == 2 ? teal : border);

            // Mask password characters
            char maskedPass[50] = "\0";
            for (int i = 0; i < passwordLetterCount; i++) {
                maskedPass[i] = '*';
            }

            maskedPass[passwordLetterCount] = '\0';
            DrawText(maskedPass,(int)passwordBox.x + 10,(int)passwordBox.y + 10,20, navy);

            // Login button
            bool hoverLogin =CheckCollisionPointRec(mousePoint, loginBtn);
            DrawRectangleRounded(loginBtn, 0.12f, 8,hoverLogin ? (Color){55, 112, 106, 255} : teal);
            const char *loginText = "LOGIN";
            DrawBoldText(loginText,(int)loginBtn.x +((int)loginBtn.width - MeasureText(loginText, 18))/2,(int)loginBtn.y + 12,18, WHITE);

            // Register button
            bool hoverRegister =CheckCollisionPointRec(mousePoint, registerBtn);
            DrawRectangleRounded(registerBtn, 0.12f, 8,hoverRegister ? navy : (Color){55, 72, 89, 255});
            const char *registerText = "REGISTER";
            DrawBoldText(registerText,(int)registerBtn.x +((int)registerBtn.width - MeasureText(registerText, 18))/2,(int)registerBtn.y + 12,18, WHITE);

            // Authentication feedback
            if (strlen(authStatusMsg) > 0) {
                int msgWidth = MeasureText(authStatusMsg, 18);
                DrawText(authStatusMsg,(screenWidth - msgWidth)/2,395, 18, authStatusColor);
            }
        }
        
        // Main Menu rendering
        else if(currentScreen == SCREEN_MENU){

            // Theme colours
            Color pageBg = { 245, 244, 239, 255 };
            Color panelBg = { 255, 255, 252, 255 };
            Color navy = { 36, 52, 71, 255 };
            Color teal = { 70, 132, 125, 255 };
            Color tealHover = { 55, 112, 106, 255 };
            Color mutedText = { 112, 119, 122, 255 };
            Color border = { 224, 224, 216, 255 };

            ClearBackground(pageBg);

            // Main content panel
            Rectangle menuPanel = { 155, 75, 490, 455 };
            DrawRectangleRounded(menuPanel, 0.035f, 8, panelBg);
            DrawRectangleRoundedLinesEx(menuPanel, 0.035f, 8, 1.0f, border);

            // Small game identity heading
            const char *brandText = "HANGMAN";
            DrawBoldText(brandText,(screenWidth - MeasureText(brandText, 34))/2,115, 34, navy);
            const char *subtitle = "A classic word guessing game";
            DrawText(subtitle,(screenWidth - MeasureText(subtitle, 17))/2,160, 17, mutedText);

            // Player welcome
            const char *welcomeStr =TextFormat("Welcome, %s!", loggedInPlayer.username);
            DrawBoldText(welcomeStr,(screenWidth - MeasureText(welcomeStr, 22))/2,215, 22, navy);

            // Player statistics
            const char *statsStr = TextFormat("WINS   %d       LOSSES   %d       SCORE   %d",loggedInPlayer.wins,loggedInPlayer.losses,loggedInPlayer.score);
            DrawText(statsStr,(screenWidth - MeasureText(statsStr, 16))/2,253, 16, mutedText);

            // Play button
            bool hoverPlay = CheckCollisionPointRec(mousePoint, playBtn);
            DrawRectangleRounded(playBtn, 0.12f, 8,hoverPlay ? tealHover : teal);
            const char *btnText = "PLAY GAME";
            DrawBoldText(btnText,(int)playBtn.x +((int)playBtn.width - MeasureText(btnText, 19))/2,(int)playBtn.y + 15, 19, WHITE);

            // Leaderboard button
            bool hoverLeaderboard =CheckCollisionPointRec(mousePoint, leaderboardBtn);
            DrawRectangleRounded(leaderboardBtn, 0.12f, 8,hoverLeaderboard ? tealHover : teal);
            const char *leaderboardText = "LEADERBOARD";
            DrawBoldText(leaderboardText,(int)leaderboardBtn.x +((int)leaderboardBtn.width -MeasureText(leaderboardText, 18))/2,(int)leaderboardBtn.y + 15, 18, WHITE);

            // Switch account button
            bool hoverSwitch =CheckCollisionPointRec(mousePoint, switchAccountBtn);
            DrawRectangleRounded(switchAccountBtn, 0.12f, 8,hoverSwitch ? border : pageBg);
            DrawRectangleRoundedLinesEx(switchAccountBtn, 0.12f, 8, 1.0f, border);
            const char *switchText = "SWITCH ACCOUNT";
            DrawBoldText(switchText,(int)switchAccountBtn.x +((int)switchAccountBtn.width -MeasureText(switchText, 16))/2,(int)switchAccountBtn.y + 14, 16, navy);

            // Exit instruction
            const char *exitHint = "Press ESC to exit";
            DrawText(exitHint,(screenWidth - MeasureText(exitHint, 15))/2,555, 15, mutedText);
        }


        // Category Selection
        else if(currentScreen == SCREEN_CATEGORY){

            Color pageBg = {245, 244, 239, 255};
            Color panelBg = {255, 255, 252, 255};
            Color navy = {36, 52, 71, 255};
            Color teal = {70, 132, 125, 255};
            Color tealHover = {55, 112, 106, 255};
            Color mutedText = {112, 119, 122, 255};
            Color border = {224, 224, 216, 255};

            ClearBackground(pageBg);

            // Main panel
            Rectangle categoryPanel = {65, 65, 670, 485};

            DrawRectangleRounded(categoryPanel, 0.025f, 8, panelBg);
            DrawRectangleLinesEx(categoryPanel, 1.0f, border);

            // Heading
            const char *title = "SELECT CATEGORY";
            DrawBoldText(title,(screenWidth - MeasureText(title, 30))/2,100, 30, navy);
            const char *subtitle = "Choose a topic to start playing";
            DrawText(subtitle,(screenWidth - MeasureText(subtitle, 16))/2,140, 16, mutedText);

            // Category buttons
            for(int i = 0; i < 12; i++){
                bool hoverCategory =CheckCollisionPointRec(mousePoint, categoryBtns[i]);
                DrawRectangleRounded(categoryBtns[i],0.10f,8,hoverCategory ? tealHover : navy);
                int textWidth = MeasureText(categoryNames[i], 18);
                DrawBoldText(categoryNames[i],(int)categoryBtns[i].x +((int)categoryBtns[i].width - textWidth)/2,(int)categoryBtns[i].y + 15,18,WHITE);
            }

            // Back button
            bool hoverBack =CheckCollisionPointRec(mousePoint, categoryBackBtn);
            DrawRectangleRounded(categoryBackBtn,0.10f,8,hoverBack ? border : pageBg);
            DrawRectangleLinesEx(categoryBackBtn, 1.0f, border);
            const char *backText = "BACK TO MENU";
            DrawBoldText(backText,(int)categoryBackBtn.x +((int)categoryBackBtn.width - MeasureText(backText, 16))/2,(int)categoryBackBtn.y + 12,16, navy);
        }

        
        // Difficulty Selection 
        else if(currentScreen == SCREEN_DIFFICULTY){

            Color pageBg = {245, 244, 239, 255};
            Color panelBg = {255, 255, 252, 255};
            Color navy = {36, 52, 71, 255};
            Color teal = {70, 132, 125, 255};
            Color tealHover = {55, 112, 106, 255};
            Color mutedText = {112, 119, 122, 255};
            Color border = {224, 224, 216, 255};
            ClearBackground(pageBg);

            // Main panel
            Rectangle difficultyPanel = {155, 65, 490, 485};
            DrawRectangleRounded(difficultyPanel, 0.035f, 8, panelBg);
            DrawRectangleLinesEx(difficultyPanel, 1.0f, border);

            // Heading
            const char *title = "SELECT DIFFICULTY";
            DrawBoldText(title,(screenWidth - MeasureText(title, 30))/2,105, 30, navy);
            const char *subtitle = "Choose your challenge level";
            DrawText(subtitle,(screenWidth - MeasureText(subtitle, 16))/2,145, 16, mutedText);

            // Easy option
            bool hoverEasy = CheckCollisionPointRec(mousePoint, easyBtn);
            DrawRectangleRounded(easyBtn, 0.10f, 8,hoverEasy ? tealHover : teal);
            DrawBoldText("EASY",(int)easyBtn.x + ((int)easyBtn.width - MeasureText("EASY", 20))/2,(int)easyBtn.y + 15, 20, WHITE);

            // Medium option
            bool hoverMedium = CheckCollisionPointRec(mousePoint, mediumBtn);
            DrawRectangleRounded(mediumBtn, 0.10f, 8,hoverMedium ? tealHover : navy);
            DrawBoldText("MEDIUM",(int)mediumBtn.x + ((int)mediumBtn.width - MeasureText("MEDIUM", 20))/2,(int)mediumBtn.y + 15, 20, WHITE);

            // Hard option 
            bool hoverHard = CheckCollisionPointRec(mousePoint, hardBtn);
            DrawRectangleRounded(hardBtn, 0.10f, 8,hoverHard ? tealHover : navy);
            DrawBoldText("HARD",(int)hardBtn.x + ((int)hardBtn.width - MeasureText("HARD", 20))/2,(int)hardBtn.y + 15, 20, WHITE);

            // Back button
            bool hoverBack = CheckCollisionPointRec(mousePoint, difficultyBackBtn);
            DrawRectangleRounded(difficultyBackBtn, 0.10f, 8,hoverBack ? border : pageBg);
            DrawRectangleLinesEx(difficultyBackBtn, 1.0f, border);
            const char *backText = "BACK TO CATEGORY";
            DrawBoldText(backText,(int)difficultyBackBtn.x +((int)difficultyBackBtn.width - MeasureText(backText, 16))/2,(int)difficultyBackBtn.y + 12, 16, navy);
        }


        //Leaderboard screen rendering
        else if(currentScreen == SCREEN_LEADERBOARD){
            Color pageBg = {245, 244, 239, 255};
            Color panelBg = {255, 255, 252, 255};
            Color navy = {36, 52, 71, 255};
            Color teal = {70, 132, 125, 255};
            Color mutedText = {112, 119, 122, 255};
            Color border = {224, 224, 216, 255};
            Color rowBg = {242, 244, 240, 255};
            ClearBackground(pageBg);

            // Main panel
            Rectangle leaderboardPanel = {55, 55, 690, 500};
            DrawRectangleRounded(leaderboardPanel, 0.025f, 8, panelBg);
            DrawRectangleLinesEx(leaderboardPanel, 1.0f, border);

            // Heading
            const char *leaderboardTitle = "PLAYER LEADERBOARD";
            DrawBoldText(leaderboardTitle,(screenWidth - MeasureText(leaderboardTitle, 30))/2,90, 30, navy);
            const char *subtitle = "TOP 5 PLAYERS BY SCORE";
            DrawText(subtitle,(screenWidth - MeasureText(subtitle, 16))/2,135, 16, mutedText);
            // Table header
            DrawRectangleRounded((Rectangle){95, 180, 610, 42},0.08f, 8, navy);
            DrawBoldText("RANK", 120, 193, 16, WHITE);
            DrawBoldText("PLAYER", 220, 193, 16, WHITE);
            DrawBoldText("SCORE", 465, 193, 16, WHITE);
            DrawBoldText("WINS", 565, 193, 16, WHITE);
            DrawBoldText("LOSSES", 650, 193, 16, WHITE);

            if(leaderboardCount == 0){
                const char *emptyMsg = "No player records found.";
                DrawText(emptyMsg,(screenWidth - MeasureText(emptyMsg, 20))/2,285, 20, mutedText);
            }
            else{
                int displayCount = (leaderboardCount < 5)? leaderboardCount : 5;
                for(int i = 0; i < displayCount; i++){
                    int rowY = 235 + (i * 43);
                    Rectangle row = {95, rowY - 5, 610, 36};
                    DrawRectangleRounded(row, 0.06f, 8,(i == 0) ? (Color){225, 238, 232, 255} : rowBg);
                    DrawBoldText(TextFormat("%d", i + 1),130, rowY + 3, 17, navy);
                    DrawText(TextFormat("%.14s", leaderboard[i].username),220, rowY + 3, 17, navy);
                    DrawText(TextFormat("%d", leaderboard[i].score),475, rowY + 3, 17, navy);
                    DrawText(TextFormat("%d", leaderboard[i].wins),575, rowY + 3, 17, navy);
                    DrawText(TextFormat("%d", leaderboard[i].losses),665, rowY + 3, 17, navy);
                }
            }
            // Back button
            bool hoverBack = CheckCollisionPointRec(mousePoint, leaderboardBackBtn);
            DrawRectangleRounded(leaderboardBackBtn, 0.10f, 8,hoverBack ? (Color){55, 112, 106, 255} : teal);
            const char *backText = "BACK TO MENU";
            DrawBoldText(backText,(int)leaderboardBackBtn.x +((int)leaderboardBackBtn.width - MeasureText(backText, 16))/2,(int)leaderboardBackBtn.y + 12,16, WHITE);
        }

        
        // Pause Menu rendering
        else if(currentScreen == SCREEN_PAUSE){
            Color pageBg = {245, 244, 239, 255};
            Color panelBg = {255, 255, 252, 255};
            Color navy = {36, 52, 71, 255};
            Color teal = {70, 132, 125, 255};
            Color tealHover = {55, 112, 106, 255};
            Color mutedText = {112, 119, 122, 255};
            Color border = {224, 224, 216, 255};
            Color lightButton = {242, 244, 240, 255};
            Color danger = {174, 83, 78, 255};
            Color dangerHover = {151, 65, 61, 255};
            ClearBackground(pageBg);

            // Main panel
            Rectangle pausePanel = {100, 55, 600, 500};
            DrawRectangleRounded(pausePanel, 0.025f, 8, panelBg);
            DrawRectangleLinesEx(pausePanel, 1.0f, border);

            // Heading
            const char *pauseTitle = "GAME PAUSED";
            DrawBoldText(pauseTitle,(screenWidth - MeasureText(pauseTitle, 32))/2,95, 32, navy);
            const char *pauseSubtitle = "Take a break. Your game is safely paused.";
            DrawText(pauseSubtitle,(screenWidth - MeasureText(pauseSubtitle, 16))/2,145, 16, mutedText);

            // Button hover detection
            bool hoverResume = CheckCollisionPointRec(mousePoint, pauseResumeBtn);
            bool hoverSwitch = CheckCollisionPointRec(mousePoint, pauseSwitchBtn);
            bool hoverMenu = CheckCollisionPointRec(mousePoint, pauseMenuBtn);
            bool hoverQuit = CheckCollisionPointRec(mousePoint, pauseQuitBtn);

            // Resume button
            DrawRectangleRounded(pauseResumeBtn, 0.10f, 8,hoverResume ? tealHover : teal);
            DrawBoldText("RESUME",(int)pauseResumeBtn.x +((int)pauseResumeBtn.width - MeasureText("RESUME", 18))/2,(int)pauseResumeBtn.y + 13, 18, WHITE);

            // Switch Account button
            DrawRectangleRounded(pauseSwitchBtn, 0.10f, 8,hoverSwitch ? border : lightButton);
            DrawRectangleLinesEx(pauseSwitchBtn, 1.0f, border);
            DrawBoldText("SWITCH ACCOUNT",(int)pauseSwitchBtn.x +((int)pauseSwitchBtn.width - MeasureText("SWITCH ACCOUNT", 16))/2,(int)pauseSwitchBtn.y + 14, 16, navy);

            // Main Menu button
            DrawRectangleRounded(pauseMenuBtn, 0.10f, 8,hoverMenu ? border : lightButton);
            DrawRectangleLinesEx(pauseMenuBtn, 1.0f, border);
            DrawBoldText("MAIN MENU",(int)pauseMenuBtn.x +((int)pauseMenuBtn.width - MeasureText("MAIN MENU", 18))/2,(int)pauseMenuBtn.y + 13, 18, navy);

            // Quit Game button
            DrawRectangleRounded(pauseQuitBtn, 0.10f, 8,hoverQuit ? dangerHover : danger);
            DrawBoldText("QUIT GAME",(int)pauseQuitBtn.x +((int)pauseQuitBtn.width - MeasureText("QUIT GAME", 18))/2,(int)pauseQuitBtn.y + 13, 18, WHITE);

            // Switch Account confirmation popup
            if(showSwitchConfirmation){
                DrawRectangle(0, 0, screenWidth, screenHeight,Fade(BLACK, 0.55f));
                Rectangle dialogBox = {100, 190, 600, 230};
                DrawRectangleRounded(dialogBox, 0.025f, 8, panelBg);
                DrawRectangleLinesEx(dialogBox, 1.5f, border);
                const char *dialogTitle = "SWITCH ACCOUNT?";
                DrawBoldText(dialogTitle,(screenWidth - MeasureText(dialogTitle, 24))/2,215, 24, navy);
                if(previousScreen == SCREEN_GAME && !gameOver){
                    const char *message1 = "You have an active game in progress.";
                    const char *message2 = "Switching accounts will forfeit this match.";
                    const char *message3 = "Continue?";
                    DrawText(message1,(screenWidth - MeasureText(message1, 17))/2,265, 17, mutedText);
                    DrawText(message2,(screenWidth - MeasureText(message2, 17))/2,290, 17, mutedText);
                    DrawText(message3,(screenWidth - MeasureText(message3, 17))/2,315, 17, mutedText);
                }
                else{

                    const char *message1 = "Your completed game result has been saved.";
                    const char *message2 = "Continue switching accounts?";
                    DrawText(message1,(screenWidth - MeasureText(message1, 17))/2,275, 17, mutedText);
                    DrawText(message2,(screenWidth - MeasureText(message2, 17))/2,305, 17, mutedText);
                }

                // Confirmation buttons
                bool hoverYes = CheckCollisionPointRec(mousePoint, confirmYesBtn);
                bool hoverNo = CheckCollisionPointRec(mousePoint, confirmNoBtn);
                DrawRectangleRounded(confirmYesBtn, 0.10f, 8,hoverYes ? dangerHover : danger);
                DrawBoldText("YES, SWITCH",(int)confirmYesBtn.x +((int)confirmYesBtn.width - MeasureText("YES, SWITCH", 16))/2,(int)confirmYesBtn.y + 12, 16, WHITE);
                DrawRectangleRounded(confirmNoBtn, 0.10f, 8,hoverNo ? border : lightButton);
                DrawRectangleLinesEx(confirmNoBtn, 1.0f, border);
                DrawBoldText("CANCEL",(int)confirmNoBtn.x +((int)confirmNoBtn.width - MeasureText("CANCEL", 16))/2,(int)confirmNoBtn.y + 12, 16, navy);
            }

            // Quit Game confirmation popup
            if(showQuitConfirmation){
                DrawRectangle(0, 0, screenWidth, screenHeight,Fade(BLACK, 0.55f));
                Rectangle dialogBox = {100, 190, 600, 230};
                DrawRectangleRounded(dialogBox, 0.025f, 8, panelBg);
                DrawRectangleLinesEx(dialogBox, 1.5f, border);
                const char *dialogTitle = "QUIT GAME?";
                DrawBoldText(dialogTitle,(screenWidth - MeasureText(dialogTitle, 24))/2,230, 24, navy);
                const char *message = "Are you sure you want to exit?";
                DrawText(message,(screenWidth - MeasureText(message, 18))/2,285, 18, mutedText);
                bool hoverYes = CheckCollisionPointRec(mousePoint, confirmYesBtn);
                bool hoverNo = CheckCollisionPointRec(mousePoint, confirmNoBtn);
                DrawRectangleRounded(confirmYesBtn, 0.10f, 8,hoverYes ? dangerHover : danger);
                DrawBoldText("YES, QUIT",(int)confirmYesBtn.x +((int)confirmYesBtn.width - MeasureText("YES, QUIT", 16))/2,(int)confirmYesBtn.y + 12, 16, WHITE);
                DrawRectangleRounded(confirmNoBtn, 0.10f, 8,hoverNo ? border : lightButton);
                DrawRectangleLinesEx(confirmNoBtn, 1.0f, border);
                DrawBoldText("CANCEL",(int)confirmNoBtn.x +((int)confirmNoBtn.width - MeasureText("CANCEL", 16))/2,(int)confirmNoBtn.y + 12, 16, navy);
            }
        }

        
        //Gameplay screen rendering
        else if(currentScreen == SCREEN_GAME){

            Color pageBg = {245, 244, 239, 255};
            Color panelBg = {255, 255, 252, 255};
            Color navy = {36, 52, 71, 255};
            Color teal = {70, 132, 125, 255};
            Color tealHover = {55, 112, 106, 255};
            Color mutedText = {112, 119, 122, 255};
            Color border = {224, 224, 216, 255};
            Color lightButton = {242, 244, 240, 255};
            Color danger = {174, 83, 78, 255};
            Color success = {70, 132, 100, 255};

            ClearBackground(pageBg);

            // Main heading
            const char *gameTitle = "HANGMAN";
            DrawBoldText(gameTitle,(screenWidth - MeasureText(gameTitle, 30))/2,25, 30, navy);

            // Difficulty and category information
            const char *difficultyName =(selectedDifficulty == DIFFICULTY_EASY) ? "EASY" :(selectedDifficulty == DIFFICULTY_MEDIUM) ? "MEDIUM" : "HARD";

            DrawText(TextFormat("DIFFICULTY: %s", difficultyName),30, 80, 16, navy);
            DrawText(TextFormat("CATEGORY: %s", categoryNames[selectedCategory]),30, 105, 16, mutedText);

            // Left gameplay panel
            Rectangle wordPanel = {25, 145, 440, 225};
            DrawRectangleRounded(wordPanel, 0.025f, 8, panelBg);
            DrawRectangleLinesEx(wordPanel, 1.0f, border);

            // Word heading
            DrawBoldText("GUESS THE WORD",50, 170, 17, navy);

            // Display hidden and revealed letters
            char formattedWord[100] = "";
            int len = strlen(displayWord);

            for(int i = 0; i < len; i++){
                char temp[3] = {displayWord[i], ' ', '\0'};
                strcat(formattedWord, temp);
            }

            int wordFontSize = (len > 12) ? 25 : 32;
            int wordWidth = MeasureText(formattedWord, wordFontSize);

            while(wordWidth > (int)wordPanel.width - 40 && wordFontSize > 18){
                wordFontSize--;
                wordWidth = MeasureText(formattedWord, wordFontSize);
            }
            int wordX = (int)wordPanel.x + ((int)wordPanel.width - wordWidth)/2;
            DrawBoldText(formattedWord, wordX, 225, wordFontSize, navy);

            // Attempts counter
            const char *attemptsStr =TextFormat("WRONG ATTEMPTS: %d / %d",wrongAttempts, maxAttempts);
            DrawBoldText(attemptsStr,50, 295, 16,(wrongAttempts >= 4) ? danger : mutedText);

            // Guessed letters
            char guessedStr[100] = "GUESSED: ";
            for(int i = 0; i < 26; i++){
                if(guessedLetters[i]){
                    char ltr[3] = {(char)('A' + i), ' ', '\0'};
                    strcat(guessedStr, ltr);
                }
            }
            DrawText(guessedStr, 50, 330, 15, mutedText);

            // Hangman drawing
            DrawHangman(wrongAttempts);

            // Hint balance
            DrawBoldText(TextFormat("HINTS: %d / 3", availableHints),30, 390, 16, navy);

            // Hint button hover
            bool hoverHint = CheckCollisionPointRec(mousePoint, hintBtn);
            Color hintColor =(!gameOver && availableHints > 0)? (hoverHint ? tealHover : teal): border;
            DrawRectangleRounded(hintBtn, 0.10f, 8, hintColor);
            DrawBoldText("USE HINT",(int)hintBtn.x +((int)hintBtn.width - MeasureText("USE HINT", 16))/2,(int)hintBtn.y + 12, 16,(!gameOver && availableHints > 0) ? WHITE : mutedText);

            // Hint recharge countdown
            if(availableHints < 3 && nextHintRecharge > 0){
                long long secondsLeft =nextHintRecharge - (long long)time(NULL);
                if(secondsLeft < 0) secondsLeft = 0;
                int hoursLeft = (int)(secondsLeft / 3600);
                int minutesLeft = (int)((secondsLeft % 3600) / 60);
                DrawText(TextFormat("Next hint: %02d:%02d",hoursLeft, minutesLeft),30, 455, 15, mutedText);
            }
            // Game status message
            if(gameOver){
                if(gameWon){
                    const char *winText = "YOU WON!";
                    DrawBoldText(winText,(screenWidth - MeasureText(winText, 23))/2,405, 23, success);
                    DrawText("Press R to play again",(screenWidth - MeasureText("Press R to play again", 15))/2,432, 15, mutedText);
                }
                else{
                    const char *lossText = TextFormat("GAME OVER! Word was: %s", secretWord);
                    DrawBoldText(lossText,(screenWidth - MeasureText(lossText, 19))/2,405, 19, danger);
                    DrawText("Press R to try again",(screenWidth - MeasureText("Press R to try again", 15))/2,432, 15, mutedText);
                }
            }
            else{
                const char *instruction ="Type any letter A-Z to guess";
                DrawText(instruction,(screenWidth - MeasureText(instruction, 16))/2,465, 16, mutedText);
            }
            // Back to Menu button
            bool hoverBack = CheckCollisionPointRec(mousePoint, backBtn);
            DrawRectangleRounded(backBtn, 0.10f, 8,hoverBack ? border : lightButton);
            DrawRectangleLinesEx(backBtn, 1.0f, border);
            const char *backText = "BACK TO MENU";
            DrawBoldText(backText,(int)backBtn.x +((int)backBtn.width - MeasureText(backText, 16))/2,(int)backBtn.y + 12, 16, navy);

            // Updating player stats after game completion
            if(gameOver && !statsUpdated){
                if(!updatePlayerStats(&loggedInPlayer, gameWon)){
                    TraceLog(LOG_ERROR, "Failed to save player stats!");
                }
                statsUpdated = true;
            }
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

    if(gameFontLoaded){
        UnloadFont(gameFont);
    }
    if(semiBoldFontLoaded){
        UnloadFont(semiBoldFont);
    }
    //clean up window memory before program termination
    CloseWindow();
    return 0;
}