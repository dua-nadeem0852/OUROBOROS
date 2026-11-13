

#include "PlayerInterface.h"
#include <fstream>      // for reading and writing files
#include <cstring>    
using namespace std;

// -(numbers that never change)

const int ID = 0;           // column 0 means which item 
const int COUNT = 1;        // column 1 meantt how many of that item
const int MAX_STACK = 99;   // we cannot carry more than 99 of one item
const int NAME_LENGTH = 20; // the player's name can have at most 19 letters


// 2D array for levels where rows represent levels and columns represent the % attack it cwill have and enemy strength accoriding to level
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


// Player infor and using static so that only this file can use these variables directly
// Other files will  use the functions from PlayerInterface.h. header file

static int hp = 100;
static int maxHp = 100;
static int level = 1;
static int atkLevel = 1;
static int maxAtkLevel = 3;
static int expPoints = 0;
static int difficulty = DIFF_NORMAL;
static unsigned int progressFlags = 0;      // many yes/no switches stored in one number
static char playerName[NAME_LENGTH] = "Hunter";   

// inventory has 8 things so we have 8 rows and 2 columns for id of things player may collect and and count of item like how much it has
// we are initialising it with -1 and 0 count as -1 represent that its empty.....
static int slot[INVENTORY_SLOTS][2] = {
    {-1, 0}, {-1, 0}, {-1, 0}, {-1, 0}, {-1, 0}, {-1, 0}, {-1, 0}, {-1, 0}
};

static char lastError[100] = "";    // remembers the last file problem

// Small helper functions only for this file use


//  to find an item to use first check whether its present or not and then Looks through all slots and returns the slot number that holds this item.
// Returning -1 if the item wasnt   in the bag.

static int findSlot(int itemId) {
    int i=0;
    while( i < INVENTORY_SLOTS) {
        if (slot[i][ID] == itemId && slot[i][COUNT] > 0) {
            return i;
        }

        i++;
    }
    return -1;
}

// to find an empty slot in bag to put and if its full then return -1 means that its full now
static int findEmptySlot() {
    for (int i = 0; i < INVENTORY_SLOTS; i++) {
        if (slot[i][COUNT] == 0) {
            return i;
        }
    }
    return -1;
}

// receivng a 2d array to a function and its the format to separately write rows num
static int totalItemsInBag(const int bag[][2], int rows) {
    int totali = 0;
    for (int i = 0; i < rows; i++) {
        totali = totali + bag[i][COUNT];
    }
    return totali;
}

// used recursion asking the function itself about next level exp, as for moving towards next level we need to multiply each exp by 100

static int expNeeded(int levl) {
    if (levl <= 1) {
        return 100;    //it will return 100 for easy level
    }
    return expNeeded(levl - 1) + 100;
}


// Readi player data noww

int getPlayerHP()
 { return hp; }
int getPlayerMaxHP() { return maxHp; }
int getplayerAtk() { return 10 + (atkLevel - 1) * 5; }   //as attack strength and power will depend on level
int getplayerAtkLevel() { return atkLevel; }
int getPlayerMaxAttackLevel() { return maxAtkLevel; }
int getDifficulty() { return difficulty; }
int getPlayerExp() { return expPoints; }
int getPlayerLevel() { return level; }
int getExpNeededForNextLevel() { return expNeeded(level); }
bool isPlayerDead() { return hp <= 0; }    //if helath becomes 0.........
int getEnemyStrengthPercent() { return difficultyTable[difficulty][1]; }   //user will enter the difficulty level and accordin to above mentioned grid enemy strength will be found and given to it from 1st column
int getTotalItems() { return totalItemsInBag(slot, INVENTORY_SLOTS); }

// Changing the player's stats according to the level of game

// Difficulty change damage like on hard: 10 * 150 / 100 = 15.
void takeDamage(int amount) {
    if (amount <= 0) {
        return;
    }
    int relDamage = amount * difficultyTable[difficulty][0] / 100;  //damage the player gets according to its level and enemy strength
    if (relDamage < 1) {
        relDamage = 1;                 // a hit always hurts at least 1
    }
    hp = hp - relDamage;
    if (hp < 0) {
        hp = 0;                         // hp cant go below 0
    }
}

//  two functions with the same name but different inputs to preven overloding

void healPlayer(int amount) {
    if (amount <= 0) {
        return;
    }
    hp = hp + amount;
    if (hp > maxHp) {
        hp = maxHp;                     // hp cant go above the maximum
    }
}

void healPlayer() {  
    hp = maxHp;                         // we use two functions to preevent overload as if there is no amount enter then player heals fully like when you move or start a new game
}

void addExperience(int amount) {
    if (amount <= 0) {
        return;
    }
    expPoints = expPoints + amount;

    // useing a loop instead of single condition check bcuz a big reward can give 2 levels at once
    while (expPoints >= expNeeded(level)) {
        expPoints = expPoints - expNeeded(level);   // the extra exp is kept and the level will be automatically incremented
        level++;
        maxHp = maxHp + 20;                          //als max hp will increase 
        hp = maxHp;                                 // level up heals the player fully
    }
}

void increasePlayerMaxHP(int amount) {
    if (amount <= 0) {
        return;                       // if oone find health upgrade containerr
    }
    maxHp = maxHp + amount;
    hp = hp + amount;
}

void increaseAttackLevel() {
    if (atkLevel < maxAtkLevel) {
        atkLevel++;
    }
}

void setDifficulty(int difflev) {  //for diff levels of gamee
    if (difflev >= DIFF_EASY && difflev <= DIFF_HARD) {   // check to ignore the wrong user inputss
        difficulty = difflev;
    }
}

// Inventory

int getSlotItemId(int s) {
    if (s >= 0 && s < INVENTORY_SLOTS && slot[s][COUNT] > 0) {    //to check which item is present in it nd for correct input checks are applied
        return slot[s][ID];
    }
    return -1;
}

int getSlotItemCount(int s) {
    if (s >= 0 && s < INVENTORY_SLOTS) {     //to give how many items are there in this lsot
        return slot[s][COUNT];
    }
    return 0;
}

bool hasItem(int itemId) {
    return findSlot(itemId) != -1;       //check if item exist or not
}

int getItemCount(int itemId) {
    int s = findSlot(itemId);
    if (s == -1) {
        return 0;
    }
    return slot[s][COUNT];
}

// only potions can be used for  Keys and relics use hasItem()....
bool useItem(int itemId) {
    int s = findSlot(itemId);
    if (s == -1) {
        return false;                   // it shows that we aint having this item
    }

    if (itemId == ITEM_HEALING_POTION) {
        if (hp == maxHp) {
            return false;               // dont waste a potion at full HP
        }
        healPlayer(40);   // so that 2or 3 potions are needed to get full hp..
    }
    else if (itemId == ITEM_POISON_POTION) {   //a trap item
        takeDamage(15);
    }
    else if (itemId == ITEM_BUFF_POTION) {
        if (atkLevel >= maxAtkLevel) {
            return false;               // attack is already at maximum
        }
        increaseAttackLevel();
    }
    else {
        return false;                   // keys and relics cannot be used...
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
    if (s == -1) {                      //first it checks whether its in the bag or not, if not then it  find empty slots and started to increase count 
        s = findEmptySlot();
        if (s == -1) {
            return false;               // bag is occupied fully
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

// OVERLOADING again, this one adds just one item if u dont addd quantity
bool addItem(int itemId) {
    return addItem(itemId, 1);
}


// C-strings 

string getItemName(int itemId) {
    if (itemId < 0 || itemId >= ITEM_TYPE_COUNT) {
        return "Empty";
    }
    return itemNames[itemId];           // the char array will becomes a string automatically
}

const char* getItemDescription(int itemId) {
    if (itemId < 0 || itemId >= ITEM_TYPE_COUNT) {
        return "";
    }
    return itemDescriptions[itemId];      //it will retun description written in item description....
}

void setPlayerName(const char* newName) {
    int length = strlen(newName);     
    if (length < 1 || length >= NAME_LENGTH) {
        strcpy(playerName, "Hunter");   // wrong length, so using the default name
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


// (progress switches)  to track all achievements...
// "progressFlags" is one number,
//   |=      turns a switch ON
//   &= ~    turns a switch OFF
//   &       checks if a switch is ON
//   >>      moves all bits one place to the right
// ------------------------------------------------------------
void setProgressFlag(unsigned int flag) {
    progressFlags = progressFlags | flag;   //turning switch on 
}

void clearProgressFlag(unsigned int flag) {
    progressFlags = progressFlags & ~flag;
}

bool hasProgressFlag(unsigned int flag) {

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




// Checks if loaded numbers make sense. Throws a message if they do not.
// (bag is a 2D array again, so we must write [2])
static void checkLoadedData(int h, int mh, int lv, int atk, int ex, int diff, const int bag[][2]) {
    if (mh < 1 || h < 0 || h > mh) {
        throw "The save file might have wrong hp values";
    }
    if (lv < 1 || ex < 0) {
        throw "The save file might havewrong level or exp values";
    }
    if (atk < 1 || atk > maxAtkLevel) {
        throw "The save file has a wrong attack level";
    }
    if (diff < DIFF_EASY || diff > DIFF_HARD) {
        throw "The save file may have wrong difficulty";
    }
    for (int i = 0; i < INVENTORY_SLOTS; i++) {
        if (bag[i][ID] < -1 || bag[i][ID] >= ITEM_TYPE_COUNT) {
            throw "The save file has a wrong item";
        }
        if (bag[i][COUNT] < 0 || bag[i][COUNT] > MAX_STACK) {
            throw "The save file might have wrong item count";
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

// saving themm
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

// ----- Load from the binary file (must read in the SAME order we had writtten it
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

        newName[NAME_LENGTH - 1] = '\0';                // making sure the text ends
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
