//SaveSystem.cpp - saving and loading the game + the high scores
// ......................TO SAVE THE PROGRESS OF PLAYER ..................
//the overall save file.......
//WHAT THIS FILE DOES: joins the three saving parts together..........
//  Player.cpp (Zarnib)  saves hp, level, exp, bag, difficulty  -> save.txt
//  Map.cpp    (Amina)   saves tiles, position, beaten bosses   -> save.txt.map
//  this file            keeps the high score                   -> highscores.txt
//the add-on version used getPlayerEXP() and restorePlayerSaveState(), which don't exist in the
//real Player.cpp (it would not compile) , so this one calls the real ones.
#include "SaveSystem.h"
#include "PlayerInterface.h"
#include "Map.h"
#include <fstream>

bool saveGame(const std::string& filename)
{
    // both should be true otherwise one would have an old data and one new dat
    
    if (!saveGameBackup(filename)) return false;       //z's function
    if (!saveMapState(filename + ".map")) return false;    //a's function
    return true;
}

bool loadGame(const std::string& filename)
{
    //load the map first. it checks its file before changing anything.
    //if the player file is broken afterwards the player is unchanged (Player.cpp checks first too)
    //its bcz to never end up with a half data
    if (!loadMapState(filename + ".map")) return false;
    if (!loadGameBackup(filename)) return false;
    return true;
}

bool saveExists(const std::string& filename)
{
    std::ifstream a(filename.c_str());              //a and b are objects or name of stream...
    std::ifstream b((filename + ".map").c_str());  // whatever the filename is , just add .map into it to show map file
    return a.is_open() && b.is_open();      // to check whether they exist or not
}

int getHighestScore(const std::string& filename)
              //using .c_str() cuz OLDER c++ streams cant use strings , they accept plain array of characters so use this to convert it into that
{
    std::ifstream in(filename.c_str());     //in>>best means read the content from file and store it in best
    int best = 0;
    if (!(in >> best) || best < 0) return 0;     // ((!) only for first expression) , if reading failed 
    return best;
}

bool saveHighScore(int score, const std::string& filename)
{
    if (score <= getHighestScore(filename)) return true;    //not a new record so nothing to do  thats why return true
    std::ofstream out(filename.c_str());    // by default it erases the whole content to write new one
    if (!out.is_open()) return false;
    out << score << "\n";
    return out.good();   //returns true if everything wokred fine
}