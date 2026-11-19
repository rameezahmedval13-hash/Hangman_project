/*=============================================================================
 * PROJECT: File Based Hangman Game with Player Profiles & Score History
 * COURSE: Programming Fundamentals
 * SECTION: BDS-1A
 *
 * GROUP MEMBERS:
 *   1. Rameez Ahmed    (26K-2527)
 *   2. Muhammad Usaid  (26K-2500)
 *
 * FILE: backend.h
 * LAST MODIFIED BY: Rameez Ahmed, Muhammad Usaid
 * DESCRIPTION: Contains the Player structure and function declarations
 *              used by main.c and backend.c. Includes functions for
 *              registration, login, random word selection, player
 *              statistics updates, and leaderboard records.
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

//Player Stats Prototype [DATE: 11/05/2026][MODIFIED BY: MUHAMMAD USAID]
int updatePlayerStats(Player *player, int won);

//Leaderboard Prototype [DATE: 11/06/2026][MODIFIED BY: MUHAMMAD USAID]
int getLeaderboard(Player players[], int maxPlayers);

#endif  //end of #ifndef
