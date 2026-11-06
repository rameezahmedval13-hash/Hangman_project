/*=============================================================================
 * PROJECT: File Based Hangman Game with Player Profiles & Score History
 * COURSE: Programming Fundamentals
 * SECTION: BDS-1A
 * 
 * GROUP MEMBERS:
 *   1. Rameez Ahmed    (26k-2527)
 *   2. Muhammad Usaid  (26k-2500)
 * 
 * FILE: backend.c
 * LAST MODIFIED BY: Rameez Ahmed, Muhammad Usaid
 * DESCRIPTION: Handles file read/write operations for user registration,
 *              login validation, player statistics updates, and persistence
 *              across players.txt and scores.txt.
 *=============================================================================*/


#include "backend.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

//function definition
int registerUser(const char *username, const char *password){
    if (username == NULL || password == NULL || strlen(username) == 0 || strlen(password) == 0){
        return 0; //Invalid input
    }//end if

    //checking existing usernames
    FILE *pFile = fopen("players.txt", "r");
    if (pFile != NULL){
        char existingUser[50], existingPass[50];
        while (fscanf(pFile, "%49s %49s", existingUser, existingPass) ==  2){ //fixed wrong condition as fscanf reads 2 variables here not 0  (10/14/2026)
            if (strcmp(existingUser, username) == 0){
                fclose(pFile);
                return -1; //Username taken error
            }//endif
        }//end while
        fclose(pFile);
    }//endif

    //appending new credentials 
    pFile = fopen("players.txt", "a");
    if (pFile == NULL){
        return 0;
    }//endif
    fprintf(pFile, "%s %s\n", username, password);
    fclose(pFile);

    //Initializing scores
    FILE *sFile = fopen("scores.txt", "a");
    if (sFile != NULL){
        fprintf(sFile, "%s 0 0 0 0\n", username);
        fclose(sFile);
    }//endif

return 1;
}//end function

//function definition
int loginUser(const char *username, const char *password, Player *player){
    FILE *pFile = fopen("players.txt", "r");
    if (pFile == NULL){
        return 0; //File not Found error
    }

    char uBuf[50], pBuf[50];
    int authenticated = 0;

    while (fscanf(pFile, "%49s %49s", uBuf, pBuf) == 2){
        if (strcmp(uBuf, username) == 0 && strcmp(pBuf, password) == 0){
            authenticated = 1;
            break;
        }//endif
    }//end while
    fclose(pFile);

    if (!authenticated){
        return 0; //Invalid credentials error
    }//endif

    //setting active player info
    strcpy(player->username, username);
    player->total_games = 0;
    player->wins = 0;
    player->losses = 0;
    player->score = 0;

    //fetching stored stats
    FILE *sFile = fopen("scores.txt", "r");
    if (sFile != NULL){
        char sUser[50];
        int tg, w, l, s;
        while (fscanf(sFile, "%49s %d %d %d %d", sUser, &tg, &w, &l, &s) == 5){
            if (strcmp(sUser, username) == 0){
                player->total_games = tg;
                player->wins = w;
                player->losses = l;
                player->score = s;
                break;
            }//endif
        }//endwhile
        fclose(sFile);

    }//endif

    return 1;
}//end function

//function definition [DATE: 11/05/2026][MODIFIED BY: MUHAMMAD USAID]
int updatePlayerStats(Player *player, int won){

    if(player == NULL){
        return 0; //Invalid pointer
    }//endif

    //Opening scores file to fetch existing player records
    FILE *sFile = fopen("scores.txt", "r");

    if(sFile == NULL){
        return 0; //File read error
    }//endif

    Player records[1000];
    int count = 0;
    int playerIndex = -1;

    char username[50];
    int tg, w, l, s;

    //Reading all existing player records
    while(fscanf(sFile, "%49s %d %d %d %d",
                 username, &tg, &w, &l, &s) == 5){

        if(count >= 1000){
            fclose(sFile);
            return 0; //Record limit reached
        }//endif

        strcpy(records[count].username, username);
        records[count].total_games = tg;
        records[count].wins = w;
        records[count].losses = l;
        records[count].score = s;

        //Finding active player and updating stats
        if(strcmp(username, player->username) == 0){

            playerIndex = count;
            records[count].total_games++;

            if(won){
                records[count].wins++;
                records[count].score += 100;
            }else{
                records[count].losses++;
            }//endif

        }//endif

        count++;

    }//endwhile

    fclose(sFile);

    if(playerIndex == -1){
        return 0; //Player not found
    }//endif

    //Opening scores file to save updated records
    sFile = fopen("scores.txt", "w");

    if(sFile == NULL){
        return 0; //File write error
    }//endif

    //Writing all player records back to file
    for(int i = 0; i < count; i++){

        fprintf(sFile, "%s %d %d %d %d\n",
                records[i].username,
                records[i].total_games,
                records[i].wins,
                records[i].losses,
                records[i].score);

    }//endfor

    fclose(sFile);

    //Updating active player profile in memory
    player->total_games = records[playerIndex].total_games;
    player->wins = records[playerIndex].wins;
    player->losses = records[playerIndex].losses;
    player->score = records[playerIndex].score;

    return 1; //Success

}//end function

//function definition [DATE: 10/20/2026][MODIFIED BY: RAMEEZ AHMED]
int getRandomWord(char *outWord){
    if(outWord == NULL){
        return 0; // invalid pointer
    }//endif

    FILE *pFile = fopen("words.txt","r");
    if (pFile == NULL){
        return 0; //file read error
    }//endif

    char words[100][50];
    int count = 0;

    //Read all words from words.txt into memory
    while(fscanf(pFile, "%49s", words[count]) == 1 && count < 100){
        count++;
    }//endwhile
    fclose(pFile);

    if(count==0){
        return 0; //empty file error
    }//endif

    //seed rng and select a random word index
    srand((unsigned int)time(NULL));
    int randomIndex = rand() % count;

    strcpy(outWord, words[randomIndex]);
    return 1; //success
}//endfunction


