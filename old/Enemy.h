#ifndef ENEMY_H
#define ENEMY_H
#include <string>

//confirm from amina
const int MAP_ROWS = 15;
const int MAP_COLS = 20;

//enemy type identifiers for amina's map. to be stored as per tile.
const int ENEMY_NONE          = -1;
const int ENEMY_MAD_SCIENTIST = 0;
const int ENEMY_HUMANOID_BAT  = 1;
const int ENEMY_HYDRA         = 2;       //final boss
const int ENEMY_TYPE_COUNT    = 3;

//human hydra specifics
const int HYDRA_HEADS  = 3;
const int HEAD_ATTACK  = 0;
const int HEAD_HEAL    = 1;
const int HEAD_DEFEND  = 2;
const int HEAD_NONE    = -1;

//max bats the humanoid bat can summon
const int MAX_BATS = 3;

//to be called rightaway when a battle starts
void createEnemy(int type); //to build the boss essentially. hp and atk scaled from player's current max hp, atk and difficulty.
//"reads" current enemy
int getEnemyType();
std::string getEnemyName();
int getEnemyHP();          //for hydra this would mean the sum of all its three heads
int getEnemyMaxHP();       //total of all heads once again fir the hydra boss
int getEnemyAttackPower();
int getEnemyExpReward();

//deals dmg to enemy and returns the dmg actually dealt.
//head only matters for the hydra (a defending head takes half damage)
int enemyTakeDamage(int amount, int head = 0);

//madscientist
int getScientistPower();                 //0 to 3 power up (stackable)
void addScientistPower();
int scientistHurtSelf(int amount);       //can never kill himself. returns the hp lost

//humanoidbat
int getBatCount();                       //summoned bats
bool summonBat();                         //returns false if already at MAX_BATS

//humanhydra
int getHeadHP(int head);
int getHeadMaxHP(int head);
bool isHeadAlive(int head);
int getLivingHeads();
int pickLivingHead(int preferred);       //preferred if alive. else first alive. else -1
int getMostWoundedHead();                //living head with lowest hp% or -1 if none are hurt
int healHead(int head, int amount);      //to return hp restored
void setHeadDefending(int head, bool on);
bool isHeadDefending(int head);
void setHeadAction(int head, int action);   //only so the screen can show it
int getHeadAction(int head);

//dead/defeated boss tracking so they never retrigger
bool isEnemyDefeated(int row, int col);
void markEnemyDefeated(int row, int col);
bool isBossTypeDefeated(int type);
void markBossTypeDefeated(int type);
void resetDefeatedEnemies();              //call when starting a new game

#endif
