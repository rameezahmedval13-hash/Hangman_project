/*=============================================================================
 * PROJECT: File Based Hangman Game with Player Profiles & Score History
 * COURSE: Programming Fundamentals
 * SECTION: BDS-1A
 * 
 * GROUP MEMBERS:
 *   1. Rameez Ahmed    (26k-2527)
 *   2. Muhammad Usaid  (26k-2500)
 * 
 * FILE: backend.h
 * LAST MODIFIED BY: Rameez Ahmed
 * DESCRIPTION: Declarations for player structure, file operations, and 
 *              authentication prototypes shared between backend.c and main.c.
 *=============================================================================*/

#ifndef BACKEND_H
#define BACKEND_H

// Player structure for credentials and stats
typedef struct {
    char username[50];
    int total_games;
    int wins;
    int losses;
    int score;
} Player;

//Authentication prototypes
int registerUser(const char *username, const char *password);
int loginUser(const char *username, const char *password, Player *player);

//Word Engine Prototypes [DATE: 10/20/2026 MODIFIED BY: RAMEEZ AHMED]
int getRandomWord(char *outWord);

#endif  //end of #ifndef
