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
 * LAST MODIFIED BY: Rameez Ahmed
 * DESCRIPTION: Handles file read/write operations for user registration,
 *              login validation, and persistence across players.txt and scores.txt.
 *=============================================================================*/


#include "backend.h"
#include <stdio.h>
#include <string.h>

//function definition
int registerUser(const char *username, const char *password){
    if (strlen(username) == 0 || strlen(password) == 0){
        return 0; //Invalid input
    }//end if

    //checking if user exists
    FILE *pFile = fopen("players.txt", "r");
    if (pFile != NULL){
        char existingUser[50], existingPass[50];
        while (fscanf(pFile, "%49s %49s", existingUser, existingPass) == 0){
            if (strcmp(existingUser, username) == 0){
                fclose(pFile);
                return -1; //Username taken error
            }//endif
        }//end while
        fclose(pFile);
    }//endif

    //Saving new credentials
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
