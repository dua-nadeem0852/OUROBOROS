

#include "PlayerInterface.h"
#include <fstream>      // for reading and writing files
#include <cstring>    
using namespace std;

// -(numbers that never change)

const int ID = 0;           // in the inventory table, column 0 = which item
const int COUNT = 1;        // column 1 = how many of that item
const int MAX_STACK = 99;   // we cannot carry more than 99 of one item
const int NAME_LENGTH = 20; // the player's name can have at most 19 letters


// 2D array for levels where rows represent levels and columns represent the % attack it can bear and enemy strength accoriding to level
static const int difficultyTable[3][2] = {
    { 75, 80 },     //  easy
    { 100, 100 },   //  normal
    { 150, 130 }    //  hard
};

// 2D array of letters:

static const char itemNames[6][16] = {
    "Healing Potion", "Poison Potion", "Buff Potion", "Relic", "Key", "Key"
};


static const char itemDescriptions[6][60] = {
    "Restores 40 HP.",
    "Looks like medicine... it is not. Costs 15 HP.",
    "Raises your attack level by one.",
    "An old relic. Someone may want it.",
    "An old key. Does it fit a lock?",
    "An old key. Does it fit a lock?"
};


// Player information
// using static so that only this file can use these variables directly.
// Other files will  use the functions from PlayerInterface.h. header file
// ------------------------------------------------------------
static int hp = 100;
static int maxHp = 100;
static int level = 1;
static int atkLevel = 1;
static int maxAtkLevel = 3;
static int expPoints = 0;
static int difficulty = DIFF_NORMAL;
static unsigned int progressFlags = 0;      // many yes/no switches stored in one number
static char playerName[NAME_LENGTH] = "Hunter";   // a C-string (array of characters)

// The inventory: 8 rows (slots) and 2 columns (item id, item count).
//   slot[0][ID] = 0 and slot[0][COUNT] = 2 means "2 healing potions in slot 0".
// id of -1 means the slot is empty.
static int slot[INVENTORY_SLOTS][2] = {
    {-1, 0}, {-1, 0}, {-1, 0}, {-1, 0}, {-1, 0}, {-1, 0}, {-1, 0}, {-1, 0}
};

static char lastError[100] = "";    // remembers the last file problem

// Small helper functions only for this


// Looks through all slots and returns the slot number that holds this item.
// Returns -1 if the item is not in the bag.
static int findSlot(int itemId) {
    for (int i = 0; i < INVENTORY_SLOTS; i++) {
        if (slot[i][ID] == itemId && slot[i][COUNT] > 0) {
            return i;
        }
    }
    return -1;
}

// Returns the number of the first empty slot, or -1 if the bag is full.
static int findEmptySlot() {
    for (int i = 0; i < INVENTORY_SLOTS; i++) {
        if (slot[i][COUNT] == 0) {
            return i;
        }
    }
    return -1;
}

// This function receives a 2D array. When we pass a 2D array to a function,
// we must write the number of columns ([2]). The number of rows is sent separately.
static int totalItemsInBag(const int bag[][2], int rows) {
    int total = 0;
    for (int i = 0; i < rows; i++) {
        total = total + bag[i][COUNT];
    }
    return total;
}

// RECURSION: this function calls itself.
// EXP needed to leave a level = level * 100.
// level 1 -> 100, level 2 -> 100 + 100 = 200, level 3 -> 200 + 100 = 300 ...
// It stops when level is 1 (this is called the "base case").
static int expNeeded(int lvl) {
    if (lvl <= 1) {
        return 100;
    }
    return expNeeded(lvl - 1) + 100;
}

// ------------------------------------------------------------
// Reading the player's stats
// ------------------------------------------------------------
int getPlayerHP() { return hp; }
int getPlayerMaxHP() { return maxHp; }
int getplayerAtk() { return 10 + (atkLevel - 1) * 5; }   // * is done before +
int getplayerAtkLevel() { return atkLevel; }
int getPlayerMaxAttackLevel() { return maxAtkLevel; }
int getDifficulty() { return difficulty; }
int getPlayerExp() { return expPoints; }
int getPlayerLevel() { return level; }
int getExpNeededForNextLevel() { return expNeeded(level); }
bool isPlayerDead() { return hp <= 0; }
int getEnemyStrengthPercent() { return difficultyTable[difficulty][1]; }
int getTotalItems() { return totalItemsInBag(slot, INVENTORY_SLOTS); }

// ------------------------------------------------------------
// Changing the player's stats
// ------------------------------------------------------------

// Difficulty changes the damage. Example on hard: 10 * 150 / 100 = 15.
void takeDamage(int amount) {
    if (amount <= 0) {
        return;
    }
    int realDamage = amount * difficultyTable[difficulty][0] / 100;
    if (realDamage < 1) {
        realDamage = 1;                 // a hit always hurts at least 1
    }
    hp = hp - realDamage;
    if (hp < 0) {
        hp = 0;                         // HP can never go below 0
    }
}

// FUNCTION OVERLOADING: two functions with the same name but different inputs.
// healPlayer(20) uses the first one, healPlayer() uses the second one.
void healPlayer(int amount) {
    if (amount <= 0) {
        return;
    }
    hp = hp + amount;
    if (hp > maxHp) {
        hp = maxHp;                     // HP can never go above the maximum
    }
}

void healPlayer() {
    hp = maxHp;                         // full heal
}

void addExperience(int amount) {
    if (amount <= 0) {
        return;
    }
    expPoints = expPoints + amount;

    // "while" is used instead of "if" because a big reward can give 2 levels at once.
    while (expPoints >= expNeeded(level)) {
        expPoints = expPoints - expNeeded(level);   // the extra EXP is kept
        level++;
        maxHp = maxHp + 20;
        hp = maxHp;                                 // level up heals the player fully
    }
}

void increasePlayerMaxHP(int amount) {
    if (amount <= 0) {
        return;
    }
    maxHp = maxHp + amount;
    hp = hp + amount;
}

void increaseAttackLevel() {
    if (atkLevel < maxAtkLevel) {
        atkLevel++;
    }
}

void setDifficulty(int diff) {
    if (diff >= DIFF_EASY && diff <= DIFF_HARD) {   // ignore wrong numbers
        difficulty = diff;
    }
}

// ------------------------------------------------------------
// Inventory
// ------------------------------------------------------------
int getSlotItemId(int s) {
    if (s >= 0 && s < INVENTORY_SLOTS && slot[s][COUNT] > 0) {
        return slot[s][ID];
    }
    return -1;
}

int getSlotItemCount(int s) {
    if (s >= 0 && s < INVENTORY_SLOTS) {
        return slot[s][COUNT];
    }
    return 0;
}

bool hasItem(int itemId) {
    return findSlot(itemId) != -1;
}

int getItemCount(int itemId) {
    int s = findSlot(itemId);
    if (s == -1) {
        return 0;
    }
    return slot[s][COUNT];
}

// Only potions can be used. Keys and relics are only checked with hasItem().
bool useItem(int itemId) {
    int s = findSlot(itemId);
    if (s == -1) {
        return false;                   // we do not have this item
    }

    if (itemId == ITEM_HEALING_POTION) {
        if (hp == maxHp) {
            return false;               // do not waste a potion at full HP
        }
        healPlayer(40);
    }
    else if (itemId == ITEM_POISON_POTION) {
        takeDamage(15);
    }
    else if (itemId == ITEM_BUFF_POTION) {
        if (atkLevel >= maxAtkLevel) {
            return false;               // attack is already at maximum
        }
        increaseAttackLevel();
    }
    else {
        return false;                   // keys and relics cannot be used up
    }

    // If we reach this line, the item was really used, so remove one.
    slot[s][COUNT]--;
    if (slot[s][COUNT] == 0) {
        slot[s][ID] = -1;               // the slot is empty again
    }
    return true;
}

// Adds many items at once. Returns false if the bag has no space.
bool addItem(int itemId, int quantity) {
    if (itemId < 0 || itemId >= ITEM_TYPE_COUNT) {
        return false;
    }
    if (quantity <= 0) {
        return false;
    }

    int s = findSlot(itemId);
    if (s == -1) {                      // not in the bag yet, so we need an empty slot
        s = findEmptySlot();
        if (s == -1) {
            return false;               // bag is full
        }
        slot[s][ID] = itemId;
        slot[s][COUNT] = 0;
    }

    slot[s][COUNT] = slot[s][COUNT] + quantity;
    if (slot[s][COUNT] > MAX_STACK) {
        slot[s][COUNT] = MAX_STACK;
    }

    // Picking up these items turns on a progress switch (see the bit part below).
    if (itemId == ITEM_REAL_KEY) {
        setProgressFlag(PROGRESS_HAS_REAL_KEY);
    }
    if (itemId == ITEM_RELIC) {
        setProgressFlag(PROGRESS_HAS_RELIC);
    }
    return true;
}

// OVERLOADING again: this one adds just one item.
bool addItem(int itemId) {
    return addItem(itemId, 1);
}

// ------------------------------------------------------------
// C-strings (text stored in character arrays)
// ------------------------------------------------------------
string getItemName(int itemId) {
    if (itemId < 0 || itemId >= ITEM_TYPE_COUNT) {
        return "Empty";
    }
    return itemNames[itemId];           // the char array becomes a string automatically
}

const char* getItemDescription(int itemId) {
    if (itemId < 0 || itemId >= ITEM_TYPE_COUNT) {
        return "";
    }
    return itemDescriptions[itemId];
}

void setPlayerName(const char* newName) {
    int length = strlen(newName);       // strlen counts the letters
    if (length < 1 || length >= NAME_LENGTH) {
        strcpy(playerName, "Hunter");   // wrong length, so use the default name
    }
    else {
        strcpy(playerName, newName);    // copy the text into our array
    }
}

const char* getPlayerName() {
    return playerName;
}

// strcmp gives 0 when two texts are exactly the same.
// Both keys are called "Key", so the real key is found first.
int findItemIdByName(const char* name) {
    for (int i = 0; i < ITEM_TYPE_COUNT; i++) {
        if (strcmp(itemNames[i], name) == 0) {
            return i;
        }
    }
    return -1;                          // not found
}

// ------------------------------------------------------------
// Bit manipulation (progress switches)
// "progressFlags" is one number, but we use its bits (0 or 1) like light switches.
//   |=      turns a switch ON
//   &= ~    turns a switch OFF
//   &       checks if a switch is ON
//   >>      moves all bits one place to the right
// ------------------------------------------------------------
void setProgressFlag(unsigned int flag) {
    progressFlags = progressFlags | flag;
}

void clearProgressFlag(unsigned int flag) {
    progressFlags = progressFlags & ~flag;
}

bool hasProgressFlag(unsigned int flag) {
    // The brackets are needed because != is done before &.
    return (progressFlags & flag) != 0;
}

// Counts how many switches are ON.
int countProgressFlags() {
    unsigned int temp = progressFlags;
    int count = 0;
    while (temp != 0) {
        if (temp & 1) {                 // is the last bit ON?
            count++;
        }
        temp = temp >> 1;               // throw away the last bit
    }
    return count;
}

// ------------------------------------------------------------
// New game
// ------------------------------------------------------------
void resetPlayerToDefault() {
    hp = 100;
    maxHp = 100;
    level = 1;
    atkLevel = 1;
    expPoints = 0;
    difficulty = DIFF_NORMAL;
    progressFlags = 0;
    strcpy(playerName, "Hunter");

    for (int i = 0; i < INVENTORY_SLOTS; i++) {     // empty every slot
        slot[i][ID] = -1;
        slot[i][COUNT] = 0;
    }

    addItem(ITEM_HEALING_POTION, 2);    // two healing potions
    addItem(ITEM_RELIC);                // one relic
}

// ------------------------------------------------------------
// SAVING AND LOADING
//
// EXCEPTIONS: if something goes wrong we "throw" a message.
// Then "catch" picks the message up, so the program does not crash.
//     try { ...risky code... }
//     catch (const char* message) { ...handle the problem... }
// ------------------------------------------------------------

// Checks if loaded numbers make sense. Throws a message if they do not.
// (bag is a 2D array again, so we must write [2])
static void checkLoadedData(int h, int mh, int lv, int atk, int ex, int diff, const int bag[][2]) {
    if (mh < 1 || h < 0 || h > mh) {
        throw "The save file has wrong HP values";
    }
    if (lv < 1 || ex < 0) {
        throw "The save file has wrong level or EXP values";
    }
    if (atk < 1 || atk > maxAtkLevel) {
        throw "The save file has a wrong attack level";
    }
    if (diff < DIFF_EASY || diff > DIFF_HARD) {
        throw "The save file has a wrong difficulty";
    }
    for (int i = 0; i < INVENTORY_SLOTS; i++) {
        if (bag[i][ID] < -1 || bag[i][ID] >= ITEM_TYPE_COUNT) {
            throw "The save file has a wrong item";
        }
        if (bag[i][COUNT] < 0 || bag[i][COUNT] > MAX_STACK) {
            throw "The save file has a wrong item count";
        }
    }
}

// Copies the checked numbers into the real game variables.
static void useLoadedData(int h, int mh, int lv, int atk, int ex, int diff,
                          unsigned int flags, const int bag[][2]) {
    hp = h;
    maxHp = mh;
    level = lv;
    atkLevel = atk;
    expPoints = ex;
    difficulty = diff;
    progressFlags = flags;
    for (int i = 0; i < INVENTORY_SLOTS; i++) {
        slot[i][ID] = bag[i][ID];
        slot[i][COUNT] = bag[i][COUNT];
        if (slot[i][COUNT] == 0) {
            slot[i][ID] = -1;
        }
    }
}

// ----- Save to an ASCII text file (you can open it in Notepad) -----
bool saveGameBackup(const string& filename) {
    try {
        ofstream out(filename.c_str());
        if (!out.is_open()) {
            throw "Could not open the file for saving";
        }

        out << hp << " " << maxHp << " " << level << " " << atkLevel << " "
            << expPoints << " " << difficulty << " " << progressFlags << "\n";
        for (int i = 0; i < INVENTORY_SLOTS; i++) {
            out << slot[i][ID] << " " << slot[i][COUNT] << "\n";
        }
        out.close();

        lastError[0] = '\0';            // no problem
        return true;
    }
    catch (const char* message) {
        strcpy(lastError, message);
        return false;
    }
}

// ----- Load from the text file -----
bool loadGameBackup(const string& filename) {
    try {
        ifstream in(filename.c_str());
        if (!in.is_open()) {
            throw "Save file not found";
        }

        // First read into temporary variables, NOT into the real game.
        // That way a bad file cannot ruin the game that is running.
        int h, mh, lv, atk, ex, diff;
        unsigned int flags;
        int bag[INVENTORY_SLOTS][2];

        in >> h >> mh >> lv >> atk >> ex >> diff >> flags;
        for (int i = 0; i < INVENTORY_SLOTS; i++) {
            in >> bag[i][ID] >> bag[i][COUNT];
        }
        if (in.fail()) {
            throw "The save file is damaged or too short";
        }
        in.close();

        checkLoadedData(h, mh, lv, atk, ex, diff, bag);     // may throw
        useLoadedData(h, mh, lv, atk, ex, diff, flags, bag);

        lastError[0] = '\0';
        return true;
    }
    catch (const char* message) {
        strcpy(lastError, message);
        return false;
    }
}

// ----- Save to a BINARY file (raw bytes, Notepad shows strange symbols) -----
// write() needs a char pointer, so we write (char*) before the variable's address (&).
// sizeof tells how many bytes the variable uses.
bool saveGameBinary(const string& filename) {
    try {
        ofstream out(filename.c_str(), ios::binary);
        if (!out.is_open()) {
            throw "Could not open the file for saving";
        }

        out.write((char*)&hp, sizeof(hp));
        out.write((char*)&maxHp, sizeof(maxHp));
        out.write((char*)&level, sizeof(level));
        out.write((char*)&atkLevel, sizeof(atkLevel));
        out.write((char*)&expPoints, sizeof(expPoints));
        out.write((char*)&difficulty, sizeof(difficulty));
        out.write((char*)&progressFlags, sizeof(progressFlags));
        out.write((char*)slot, sizeof(slot));           // the whole inventory table at once
        out.write(playerName, NAME_LENGTH);
        out.close();

        lastError[0] = '\0';
        return true;
    }
    catch (const char* message) {
        strcpy(lastError, message);
        return false;
    }
}

// ----- Load from the binary file (must read in the SAME order we wrote) -----
bool loadGameBinary(const string& filename) {
    try {
        ifstream in(filename.c_str(), ios::binary);
        if (!in.is_open()) {
            throw "Save file not found";
        }

        int h, mh, lv, atk, ex, diff;
        unsigned int flags;
        int bag[INVENTORY_SLOTS][2];
        char newName[NAME_LENGTH];

        in.read((char*)&h, sizeof(h));
        in.read((char*)&mh, sizeof(mh));
        in.read((char*)&lv, sizeof(lv));
        in.read((char*)&atk, sizeof(atk));
        in.read((char*)&ex, sizeof(ex));
        in.read((char*)&diff, sizeof(diff));
        in.read((char*)&flags, sizeof(flags));
        in.read((char*)bag, sizeof(bag));
        in.read(newName, NAME_LENGTH);
        if (in.fail()) {
            throw "The save file is damaged or too short";
        }
        in.close();

        newName[NAME_LENGTH - 1] = '\0';                // make sure the text ends
        checkLoadedData(h, mh, lv, atk, ex, diff, bag); // may throw
        useLoadedData(h, mh, lv, atk, ex, diff, flags, bag);
        strcpy(playerName, newName);

        lastError[0] = '\0';
        return true;
    }
    catch (const char* message) {
        strcpy(lastError, message);
        return false;
    }
}

const char* getLastFileError() {
    return lastError;
}
