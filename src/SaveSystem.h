//SaveSystem.h      handles saving and loading the game + the high score list
//WHAT THIS FILE DOES: the functions Game.cpp calls to write/read save files.
//using mine  real save functions so the player's LEVEL is saved too, and also save the map)
#ifndef SAVE_SYSTEM_H
#define SAVE_SYSTEM_H
#include <string>

//save/load are only meant to be used while exploring, never in the middle of a battle.
//player stuff goes to <filename>, map stuff goes to <filename>.map
bool saveGame(const std::string& filename = "save.txt");
bool loadGame(const std::string& filename = "save.txt");
bool saveExists(const std::string& filename = "save.txt");   //used to grey out "Continue"

//keeps the best score ever in a text file
bool saveHighScore(int score, const std::string& filename = "highscores.txt");   // only writes if it beats the old one
int  getHighestScore(const std::string& filename = "highscores.txt");            // 0 if there is no file yet

#endif