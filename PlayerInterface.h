
// it contains a list of all the functions members 1 and 3 will be able to call from here.
// It only DECLARES functions It's really easy to summon units,

#ifndef PLAYER_INTERFACE_H     // "including  guard into it to  prevents double inclusion of this file.....
#define PLAYER_INTERFACE_H

#include <string>

//  Item identifiers 
// instead of using the numbers we use their names and it will automatically give it a number

const int ITEM_HEALING_POTION = 0;
const int ITEM_POISON_POTION  = 1;
const int ITEM_BUFF_POTION    = 2;
const int ITEM_RELIC          = 3;
const int ITEM_REAL_KEY       = 4;
const int ITEM_DECOY_KEY      = 5;
const int ITEM_TYPE_COUNT     = 6;

// for difficult levels ......
const int DIFF_EASY   = 0;
const int DIFF_NORMAL = 1;
const int DIFF_HARD   = 2;

//Inventory size ...
const int INVENTORY_SLOTS = 8;

//  using flag check  to avoid using alot of memory and making checking much faster
// These are ONESwitches (ONE bit in one number) each flag.
// "1u << 3" means: start with 1, slide it 3 places left  ->  binary 1000
const unsigned int PROGRESS_HAS_REAL_KEY   = 1u << 0;
const unsigned int PROGRESS_HAS_RELIC   = 1u << 1;
const unsigned int PROGRESS_BAT_DEFEATED  = 1u << 2;
const unsigned int PROGRESS_HYDRA_DEFEATED   = 1u << 3;
const unsigned int PROGRESS_MAD_SCIENTIST   = 1u << 4;


// Reading  stats of player....

int getPlayerHP();
int getPlayerMaxHP();
int getplayerAtk();              
int getplayerAtkLevel();
int getPlayerMaxAttackLevel();
int getDifficulty();

// now updating the stats of player....

void takeDamage(int amount);
void healPlayer(int amount);     //will add the amount into hp when you drink any potion...
void healPlayer();                //  When hp needs to be fullly restored....
void addExperience(int amount);
void increasePlayerMaxHP(int amount);
void increaseAttackLevel();


// Inventory (8 slots)

int  getSlotItemId(int slot);
int  getSlotItemCount(int slot);
bool hasItem(int itemId);
int  getItemCount(int itemId);
bool useItem(int itemId);
bool addItem(int itemId);  
bool addItem(int itemId,int quantity);           // if more items needed to  be added once....
  
std::string getItemName(int itemId);



int  getPlayerExp();
int  getPlayerLevel();
int  getExpNeededForNextLevel();   // for the exp bar
void setDifficulty(int diff);
bool isPlayerDead();
int  getEnemyStrengthPercent();          // This will be used in combating by member3.  80/100/130 depending on dfficulty levels
int  getTotalItems();                      // total items carried (uses recursion)

const char* getItemDescription(int itemId);


// using c strings for player naem
void setPlayerName(const char* newName);
const char* getPlayerName();



// flagss

void setProgressFlag(unsigned int flag);
void clearProgressFlag(unsigned int flag);
bool hasProgressFlag(unsigned int flag);



int  countProgressFlags();

//Looks up items based on a text (C-string compare). Returns -1 if there is no return.
int findItemIdByName(const char* name);

// starting the game from start ....
void resetPlayerToDefault();

//  Saving / loading 

bool saveGameBackup(const std::string& filename ="save_backup.txt");
bool loadGameBackup(const std::string& filename ="save_backup.txt");
bool saveGameBinary(const std::string& filename ="save_backup.bin");
bool loadGameBinary(const std::string& filename ="save_backup.bin");
const char* getLastFileError();

#endif