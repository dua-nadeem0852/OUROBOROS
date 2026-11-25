//ZARNIB RAUF 26L-0769 member 2
//SaveSystem.cpp - saving and loading the game + the high score
//WHAT THIS FILE DOES: joins the three saving parts together
//  Player.cpp (Zarnib)  saves hp, level, exp, bag, difficulty  -> save.txt
//  Map.cpp    (Amina)   saves tiles, position, beaten bosses   -> save.txt.map
//  this file            keeps the high score                   -> highscores.txt
#include "SaveSystem.h"
#include "PlayerInterface.h"
#include "Map.h"
#include <fstream>

bool saveGame(const std::string& filename)
{
    //both parts must work, if not then we would have a save that is half old and half new
    if (!saveGameBackup(filename)) return false;
    if (!saveMapState(filename + ".map")) return false;
    return true;
}

bool loadGame(const std::string& filename)
{
    //load the map first. it checks its file before changing anything.
    //NOTE: if the map loads but the player file is then broken, the map stays changed and the player does not
    //(only possible with a corrupted or hand-edited save; Player.cpp checks its file before changing anything)
    if (!loadMapState(filename + ".map")) return false;
    if (!loadGameBackup(filename)) return false;
    return true;
}

bool saveExists(const std::string& filename)
{
    std::ifstream a(filename.c_str());
    std::ifstream b((filename + ".map").c_str());
    return a.is_open() && b.is_open();
}

int getHighestScore(const std::string& filename)
{
    std::ifstream in(filename.c_str());
    int best = 0;
    if (!(in >> best) || best < 0) return 0;     //missing or broken file counts as no score
    return best;
}

bool saveHighScore(int score, const std::string& filename)
{
    if (score <= getHighestScore(filename)) return true;    //not a new record so nothing to do 
    std::ofstream out(filename.c_str());
    if (!out.is_open()) return false;
    out << score << "\n";
    return out.good();
}
