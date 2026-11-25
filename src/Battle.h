//DUA NADEEM 26L-0852 member 3
//Battle.h - WHAT THIS FILE DOES: the list of combat functions + sound ids other files can use. code is in Battle.cpp
#ifndef BATTLE_H
#define BATTLE_H
#include <string>
#include <vector>

//player actions
const int ACTION_ATTACK = 0;
const int ACTION_DEFEND = 1;
const int ACTION_ITEM = 2;
const int ACTION_RUN = 3;     //only exists in the Mad Scientist fight and it never works (a decoy)

//battle possibilities
const int BATTLE_ONGOING = 0;
const int BATTLE_WON = 1;   // boss defeated, rewards already given
const int BATTLE_LOST = 2;    //when hp 0, game over

//sound ids. Battle.cpp announces them with addSound(), BattleSound.cpp plays the matching file
const int SND_NONE = 0;
const int SND_PLAYER_HIT = 1;   //sword lands a hit
const int SND_PLAYER_HURT = 2;   //player winces/grunts
const int SND_BLOCK = 3;   //damage taken while defending
const int SND_HEAL = 4;
const int SND_BUFF = 5;
const int SND_SCIENTIST_LAUGH = 6;
const int SND_BAT_SCREECH = 7;
const int SND_HYDRA_WAIL = 8;
const int SND_VICTORY = 9;
const int SND_DEFEAT = 10;
//the sound a boss makes when it gets HURT by the player (played right after the sword hit sound)
const int SND_SCIENTIST_HIT = 11;   //scientist gets hurt
const int SND_BAT_HIT = 12;         //bat gets hurt
const int SND_HYDRA_HIT = 13;       //hydra gets hurt
//two that are not about the boss: viktor brace himself (defend) and a footstep for walking in explore mode
const int SND_DEFEND = 14;          //viktor uses defend
const int SND_STEP = 15;            //a step on the map (played by Game.cpp, not by the battle)
const int SND_COUNT = 16;

// Begins a fight. Returns false (and does nothing) if that boss was already defeated or the type is invalid.
bool beginBattle(int bossType, int row, int col);

//mad scientist interaction (dialogue) before starting the battle with him
bool isInIntro();
int getIntroLineCount();
std::string getIntroLine(int index);
int getIntroIndex();
void advanceIntro();

//hydra head targets
int  getTargetHead();
void cycleTargetHead(int direction);    //to skip dead hydra heads

// itemId is only used for ACTION_ITEM. Returns the result afterwards.

// An invalid choice (no such item, Run in a non-Scientist fight) costs no turn.
int  playerAction(int action, int itemId = 0);

//small pause between the players move and the bosses answer (the battle screen turns this on).
//with it on, playerAction only does the players half. the boss answers when runPendingEnemyTurn() is called, a moment later.
//with it off (default) playerAction does the whole round at once
void setEnemyTurnDelay(bool on);
bool isEnemyTurnPending();            //true = the player already moved and the boss is about to answer
void runPendingEnemyTurn();           //the bosses half of the round

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
int roundCounter();          //goes up every time something happened that the screen should react to: your move, and (with the turn delay on) the boss's answer as its own step. invalid choices don't count
int lastDamageDealt();     //damage dealt by the player in the last step
int lastDamageTaken();    //HP the player really lost in the last step (your move, or the boss's answer when the delay is on)
int lastHeadHit();           //hydra head the player hit in last round or -1
bool isBattleActive();
int getBattleResult();
bool isPlayerDefending();
bool canRunInThisBattle();       //mad scientist
int playerBuffTurns();
int playerCurseTurns();
int playerCurseDamage();         //HP one curse tick really costs (difficulty included)
int hydraTurnsUntilWail();      //2 then 1 then reset essentially to 2 then back to 1 basically for the wail mechanic
int battleLogCount();             //lines about what just happened
std::string getBattleLogLine(int index);
std::vector<bool> hpWordFlags(const std::string& line);   //one flag per word of a log line, true = that word is about hp (screen draws it red)
int queueSound();               //next sound to be played including SND_NONE so no sound too basically

void resetBattleSystem();      //to be called when a new game starts

#endif
