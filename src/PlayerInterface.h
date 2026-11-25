//ZARNIB RAUF 26L-0769 member 2
//PlayerInterface.h ------- WHAT THIS FILE DOES: the list of player/bag functions everyone else calls (code is in Player.cpp)
//it only DECLARES functions (and item / difficulty constants), so Map, Battle and Screens never touch Player.cpp's variables directly

#ifndef PLAYER_INTERFACE_H     // include guard: stops this file from being included twice
#define PLAYER_INTERFACE_H
#include <string>

//  Item identifiers 
// instead of using the numbers we use their names and it will automatically give it a number
const int ITEM_HEALING_POTION = 0;   //constant terms are capitalised....
const int ITEM_POISON_POTION = 1;
const int ITEM_BUFF_POTION = 2;
const int ITEM_RELIC = 3;
const int ITEM_REAL_KEY = 4;   //both keys are shown to the player as key, they dont know which ones fake which ones real
const int ITEM_DECOY_KEY = 5;  
const int ITEM_TYPE_COUNT = 6;

//difficulty level vals
const int DIFF_EASY = 0;
const int DIFF_NORMAL = 1;
const int DIFF_HARD = 2;

//progress switches (each one is a single bit, so they can be mixed in one number with | )
//Player.cpp turns the first two on by itself when the item is picked up

//bit operations are safer iwthout sign
const unsigned int PROGRESS_HAS_REAL_KEY = 1;     //bit 0 (001) is set one(. it will be turned on when he got real key)
const unsigned int PROGRESS_HAS_RELIC    = 2;     //bit 1 (010) when the relic is picked up
const unsigned int PROGRESS_BEAT_GAME    = 4;     //bit 2, itll be on when the chest is opened

// functions for reading player stats.....

int getPlayerHP();
int getPlayerMaxHP();
int getplayerAtk();          //dmg a normal atk of the player deals depending on their atk level
int getplayerAtkLevel();     
int getPlayerMaxAttackLevel();  //the highest attack level the player can reach (3)
int getDifficulty();

//update player stats                            (one should keep HP within 0 to max hp inside these)
void takeDamage(int amount);            //the difficulty % is applied inside, so the HP lost is not always 'amount'
int damageAfterDifficulty(int amount);  //what a hit of that size really costs on this difficulty (min 1), without hurting anyone
void healPlayer(int amount);
void addExperience(int amount);
void increasePlayerMaxHP(int amount);   //combat calls this with (boss exp / 2)
void increaseAttackLevel();             //combat calls this after scientist and bat

//inventory (holds 8 things at once and copies of an item in one slot)
const int INVENTORY_SLOTS = 8;
int getSlotItemId(int slot);      //item in slot 0 to 7. -1 if the slot is empty
int getSlotItemCount(int slot);   //counts copies of anitem that are stacked in that slot

bool hasItem(int itemId);
int getItemCount(int itemId);
bool useItem(int itemId);        // removes one and applies it: healing restores HP, poison costs HP, a buff raises the attack level by one for good
                                 // (Battle.cpp adds the extra x1.5 for a few rounds on top). false if it is not in the bag, or healing at full HP. the buff is always used: at max attack level only the permanent +1 is skipped
bool addItem(int itemId);        //returns false if inventory is full
void sortInventory();            //bubble sort: puts the bag in A-Z order by item name (empty slots go last)
std::string getItemName(int itemId);    // both keys return "Key"

int  getPlayerExp();
int  getPlayerLevel();
int  getExpNeededForNextLevel();   // for the exp bar
void setDifficulty(int diff);
bool isPlayerDead();
int  getTotalItems();                      // total items carried (adds up the slots with a loop)

const char* getItemDescription(int itemId);

// using c strings for player naem
void setPlayerName(const char* newName);
const char* getPlayerName();

// flagss

void setProgressFlag(unsigned int flag);     //  to turn a awitch on...

// starting the game from start ....
void resetPlayerToDefault();

// Saving / loading                                                      //we writing string herre cuz we are expecting a text sentence here...
                                                                       //the game saves from the pause menu (S), see SaveSystem.cpp
bool saveGameBackup(const std::string& filename ="save_backup.txt");   // it writes readable text backup for game state and returns true if its saving was completed
bool loadGameBackup(const std::string& filename ="save_backup.txt");  //reads back the txt file so that layer can continue...
const char* getLastFileError();       // instead of just showing that the file has failed to load we shwo the error message why its failed
#endif
