#include "Enemy.h"
#include "PlayerInterface.h"

/*TUNING OF STATS. A boss's stats come from the player's stats at battle start:
 hp = player max hp  x HP_WEIGHT  x difficulty HP%
attack = player attack  x ATK_WEIGHT x difficulty attack%
(for the hydra, HP_WEIGHT is per head.)*/

static const std::string BASE_NAME[ENEMY_TYPE_COUNT] = { "Mad Scientist", "Humanoid Bat", "Human Hydra"};
static const int BASE_EXP[ENEMY_TYPE_COUNT]   = {  50, 100, 200 };
static const int HP_WEIGHT[ENEMY_TYPE_COUNT]  = { 100,  105,  30 };   //%
static const int ATK_WEIGHT[ENEMY_TYPE_COUNT] = {  90,   70,  52 };   //%

static const int HP_PERCENT[3] = {   70,  105,  145 };     //     easy    normal   hard
static const int ATK_PERCENT[3] = {   65,  105,  135 };      //     easy    normal   hard

//boss that player is CURRENTLY fighting
static int curType = ENEMY_NONE;
static std::string curName   = "";
static int curHP = 0;
static int curMaxHP = 0;
static int curAttack = 0;
static int curExp = 0;

static int sciPower = 0;
static int batCount = 0;
static int headHP[HYDRA_HEADS];
static int headMaxHP[HYDRA_HEADS];
static bool headDefending[HYDRA_HEADS];
static int headAction[HYDRA_HEADS];

//defeated tracking
static bool defeated[MAP_ROWS][MAP_COLS] = { { false } };
static bool bossDefeated[ENEMY_TYPE_COUNT] = { false };

void createEnemy(int type)
{
    curType = ENEMY_NONE; curName = ""; curHP = curMaxHP = curAttack = curExp = 0;
    sciPower = 0; batCount = 0;
    for (int i = 0; i < HYDRA_HEADS; i++)
    {
        headHP[i] = headMaxHP[i] = 0;
        headDefending[i] = false;
        headAction[i] = HEAD_NONE;
    }
    if (type < 0 || type >= ENEMY_TYPE_COUNT) return;

    int diff = getDifficulty();
    if (diff < 0 || diff > 2) diff = DIFF_NORMAL;

    int hp  = getPlayerMaxHP() * HP_WEIGHT[type]  * HP_PERCENT[diff]  / 10000;
    int atk = getplayerAtk() * ATK_WEIGHT[type] * ATK_PERCENT[diff] / 10000;
    if (hp  < 1) hp  = 1;
    if (atk < 1) atk = 1;

    curType   = type;
    curName   = BASE_NAME[type];
    curAttack = atk;
    curExp    = BASE_EXP[type];     //exp not really scaled by difficulty

    if (type == ENEMY_HYDRA)
    {
        for (int i = 0; i < HYDRA_HEADS; i++) headHP[i] = headMaxHP[i] = hp;
        curMaxHP = hp * HYDRA_HEADS;
        curHP = curMaxHP;
    }
    else
    {
        curMaxHP = curHP = hp;
    }
}

int getEnemyType() { return curType; }
std::string getEnemyName() { return curName; }
int getEnemyMaxHP() { return curMaxHP; }
int getEnemyAttackPower() { return curAttack;}
int getEnemyExpReward() { return curExp; }

int getEnemyHP()
{
    if (curType == ENEMY_HYDRA)
    {
        int total = 0;
        for (int i = 0; i < HYDRA_HEADS; i++) total += headHP[i];
        return total;
    }
    return curHP;
}

int enemyTakeDamage(int amount, int head)
{
    if (amount < 0) amount = 0;

    if (curType == ENEMY_HYDRA)
    {
        head = pickLivingHead(head);
        if (head < 0) return 0;
        if (headDefending[head] && amount > 0)
        {
            amount = amount / 2;
            if (amount < 1) amount = 1;
        }
        if (amount > headHP[head]) amount = headHP[head];
        headHP[head] -= amount;
        return amount;
    }

    if (amount > curHP) amount = curHP;
    curHP -= amount;
    return amount;
}

//mad scientist
int  getScientistPower() { return sciPower; }
void addScientistPower() { sciPower++; }

int scientistHurtSelf(int amount)
{
    if (amount >= curHP) amount = curHP - 1;   //never kills himself
    if (amount < 0) amount = 0;
    curHP -= amount;
    return amount;
}

//humanoid bat
int getBatCount() { return batCount; }

bool summonBat()
{
    if (batCount >= MAX_BATS) return false;
    batCount++;
    return true;
}

//human hydra
int  getHeadHP(int h) { return (h >= 0 && h < HYDRA_HEADS) ? headHP[h] : 0; }
int  getHeadMaxHP(int h) { return (h >= 0 && h < HYDRA_HEADS) ? headMaxHP[h] : 0; }
bool isHeadAlive(int h) { return getHeadHP(h) > 0; }

int getLivingHeads()
{
    int n = 0;
    for (int i = 0; i < HYDRA_HEADS; i++) if (headHP[i] > 0) n++;
    return n;
}

int pickLivingHead(int preferred)
{
    if (curType != ENEMY_HYDRA) return (curHP > 0) ? 0 : -1;
    if (preferred >= 0 && preferred < HYDRA_HEADS && headHP[preferred] > 0) return preferred;
    for (int i = 0; i < HYDRA_HEADS; i++) if (headHP[i] > 0) return i;
    return -1;
}

int getMostWoundedHead()
{
    int best = -1;
    int bestPercent = 101;
    for (int i = 0; i < HYDRA_HEADS; i++)
    {
        if (headHP[i] <= 0 || headHP[i] >= headMaxHP[i]) continue;   //dead or unhurt
        int percent = headHP[i] * 100 / headMaxHP[i];
        if (percent < bestPercent)
        {
            best = i;
            bestPercent = percent;
        }
    }
    return best;
}

int healHead(int head, int amount)
{
    if (head < 0 || head >= HYDRA_HEADS || headHP[head] <= 0 || amount < 0) return 0;
    int room = headMaxHP[head] - headHP[head];
    if (amount > room) amount = room;
    headHP[head] += amount;
    return amount;
}

void setHeadDefending(int h, bool on) { if (h >= 0 && h < HYDRA_HEADS) headDefending[h] = on; }
bool isHeadDefending(int h) { return (h >= 0 && h < HYDRA_HEADS) ? headDefending[h] : false; }
void setHeadAction(int h, int a) { if (h >= 0 && h < HYDRA_HEADS) headAction[h] = a; }
int  getHeadAction(int h) { return (h >= 0 && h < HYDRA_HEADS) ? headAction[h] : HEAD_NONE; }

//for tracking defeated enemies
bool isEnemyDefeated(int row, int col)
{
    if (row < 0 || row >= MAP_ROWS || col < 0 || col >= MAP_COLS) return false;
    return defeated[row][col];
}

void markEnemyDefeated(int row, int col)
{
    if (row < 0 || row >= MAP_ROWS || col < 0 || col >= MAP_COLS) return;
    defeated[row][col] = true;
}

bool isBossTypeDefeated(int type)
{
    return (type >= 0 && type < ENEMY_TYPE_COUNT) ? bossDefeated[type] : false;
}

void markBossTypeDefeated(int type)
{
    if (type >= 0 && type < ENEMY_TYPE_COUNT) bossDefeated[type] = true;
}

void resetDefeatedEnemies()
{
    for (int r = 0; r < MAP_ROWS; r++)
        for (int c = 0; c < MAP_COLS; c++)
            defeated[r][c] = false;
    for (int t = 0; t < ENEMY_TYPE_COUNT; t++) bossDefeated[t] = false;
}
