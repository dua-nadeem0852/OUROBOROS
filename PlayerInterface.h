//integrate zarnib's inventory eventually


#ifndef PLAYER_INTERFACE_H
#define PLAYER_INTERFACE_H
#include <string>

//item identifiers
const int ITEM_HEALING_POTION = 0;
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

//read player stats
int getPlayerHP();
int getPlayerMaxHP();
int getplayerAtk();          //dmg a normal atk of the player deals depending on their atk level
int getplayerAtkLevel();     
int getPlayerMaxAttackLevel();  //to be decided
int getDifficulty();

//update player stats                            (zarnib should keep HP within 0 to max hp inside these)
void takeDamage(int amount);
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
bool useItem(int itemId);        // removes one and affects hp (healing or poison). buffs are handled in combat. it will return false if item unavailable
bool addItem(int itemId);        //returns false if inventory is full
std::string getItemName(int itemId);    // both keys return "Key"

#endif
