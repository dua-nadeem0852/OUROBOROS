#ifndef BATTLE_H
#define BATTLE_H
#include <string>

//player actions
const int ACTION_ATTACK = 0;
const int ACTION_DEFEND = 1;
const int ACTION_ITEM = 2;
const int ACTION_RUN = 3;     //more of a fake run in the mad scientist boss 

//battle possibilities
const int BATTLE_ONGOING = 0;
const int BATTLE_WON = 1;   // boss defeated, rewards already given
const int BATTLE_LOST = 2;    //when hp 0, game over

//sound stuff (may change it to a sep file, audio needs to be added)
const int SND_NONE = 0;
const int SND_PLAYER_HIT = 1;   //sword lands a hit
const int SND_PLAYER_HURT = 2;   //player winces/grunts
const int SND_BLOCK = 3;   //dmg when blocking a huff maybe
const int SND_HEAL = 4;
const int SND_BUFF = 5;
const int SND_SCIENTIST_LAUGH = 6;
const int SND_BAT_SCREECH = 7;
const int SND_HYDRA_WAIL = 8;
const int SND_VICTORY = 9;
const int SND_DEFEAT = 10;
const int SND_COUNT = 11;


// Begins a fight. Returns false (and does nothing) if that boss was already defeated or the type is invalid.
bool beginBattle(int bossType, int row, int col);

//mad scientist interaction (dialogue) before starting the battle with him
bool isInIntro();
int getIntroLineCount();
std::string getIntroLine(int index);
int getIntroIndex();
void advanceIntro();

//hydra head targets
void setTargetHead(int head);
int  getTargetHead();
void cycleTargetHead(int direction);    //to skip dead hydra heads

// itemId is only used for ACTION_ITEM. Returns the result afterwards.


// An invalid choice (no such item, Run in a non-Scientist fight) costs no turn.
int  playerAction(int action, int itemId = 0);

//playerAction calls:
void playerAtk();
void defend();
bool useBattleItem(int itemId);       //returns true if an item was used
bool attemptRun();                //will return false because its a fake run option

int  checkBattleResult();
void battleReward();          //hp, level exp and atk etc
int  endBattle();                 //pretty self explanantory, but also marks the boss dead if it is defeated by the player

//rewards that may not fit in player's inventory (just in case)
bool hasPendingReward();
bool retryPendingReward();        //if a slot frees up, it is added

//battle items
bool isBattleItem(int itemId);    // potions only basically

//info for the SFML battle screen (read only)
int battleCounter();         //increment every time a new battle starts
int roundCounter();          //increment every round that was actually played not the invalid ones though, they dont count
int lastDamageDealt();     //damage dealt by the player in the last round
int lastDamageTaken();    //total damage the player took in the last round
int lastHeadHit();           //hydra head the player hit in last round or -1
bool isBattleActive();
int getBattleResult();
bool isPlayerDefending();
bool canRunInThisBattle();       //mad scientist
int playerBuffTurns();
int playerCurseTurns();
int playerCurseDamage();
int hydraTurnsUntilWail();      //2 then 1 then reset essentially to 2 then back to 1 basically for the wail mechanic
int battleLogCount();             //lines about what just happened
std::string getBattleLogLine(int index);
int queueSound();               //next sound to be played including SND_NONE so no sound too basically
bool isHydraDefeated();          //needs to be true for the chest

void resetBattleSystem();      //to be called when a new game starts

#endif
