#ifndef PLAYER_INTERFACE_H
#define PLAYER_INTERFACE_H

#include <string>

// Item identifiers[cite: 6]
const int ITEM_HEALING_POTION = 0;[cite: 6]
const int ITEM_POISON_POTION = 1;[cite: 6]
const int ITEM_BUFF_POTION = 2;[cite: 6]
const int ITEM_RELIC = 3;[cite: 6]
const int ITEM_REAL_KEY = 4;[cite: 6]
const int ITEM_DECOY_KEY = 5;[cite: 6]
const int ITEM_TYPE_COUNT = 6;[cite: 6]

// Difficulty level values[cite: 6]
const int DIFF_EASY = 0;[cite: 6]
const int DIFF_NORMAL = 1;[cite: 6]
const int DIFF_HARD = 2;[cite: 6]

// Read player stats[cite: 6]
int getPlayerHP();[cite: 6]
int getPlayerMaxHP();[cite: 6]
int getplayerAtk();[cite: 6]
int getplayerAtkLevel();[cite: 6]
int getPlayerMaxAttackLevel();[cite: 7]
int getDifficulty();[cite: 7]

// Update player stats[cite: 7]
void takeDamage(int amount);[cite: 7]
void healPlayer(int amount);[cite: 7]
void addExperience(int amount);[cite: 7]
void increasePlayerMaxHP(int amount);[cite: 7]
void increaseAttackLevel();[cite: 7]

// Inventory (holds 8 items at once)[cite: 7]
const int INVENTORY_SLOTS = 8;[cite: 7]
int getSlotItemId(int slot);[cite: 7]
int getSlotItemCount(int slot);[cite: 7]
bool hasItem(int itemId);[cite: 7]
int getItemCount(int itemId);[cite: 7]
bool useItem(int itemId);[cite: 7]
bool addItem(int itemId);[cite: 7]
std::string getItemName(int itemId);[cite: 8]

// Member 2 Additional Functions (EXP, Level, & Backup System)
int getPlayerExp();
int getPlayerLevel();
void setDifficulty(int diff);
bool saveGameBackup(const std::string& filename = "save_backup.txt");
bool loadGameBackup(const std::string& filename = "save_backup.txt");
void resetPlayerToDefault();

#endif