#include "Battle.h"
#include "Enemy.h"
#include "PlayerInterface.h"
#include <cstdlib>

//might chnage these numbers in the future!

//buff potion (used in battle)
static const int BUFF_PERCENT = 150;      //plus 50% extra dmg
static const int BUFF_TURNS = 3;        //said extra atk/dmg for these many rounds

//BOSS MECHANICS outside of the common atks (basically special mechanics)
//mad scientist: i wanted to show him a little erratic so each turn rolls 0-99
//0-49 normal attack.    50-74 wild swing (0x-2x damage).    75-99 hurts himself (power-up for himself)
static const int SCI_WILD_ROLL = 50;
static const int SCI_SELF_HARM_ROLL= 75;
static const int SCI_POWER_PERCENT= 30;   //plus 30% extra atk for each powerup
static const int SCI_MAX_POWER= 3;
static const int SCI_SELF_HARM_PERCENT = 10;   //percentage of his max hp that he loses everytime he hurts himself for a power up/buff
static const int SCI_SAFE_HP_PERCENT = 30;   //he will not hurt himself below this hp%

//humanoid bat
static const int BAT_SCREECH_CHANCE = 30;      //basically the % chance per turn to summon a bat/bats
static const int BAT_SWARM_PERCENT = 25;      //each bat would add this % of the humanoid bat attack

//human hydra (final boss)
static const int HEAD_ATTACK_PERCENT = 70;     //one hydra head's attack = boss attack*this
static const int HEAD_HEAL_PERCENT = 20;      //a single heal would restore this % of a head's max hp
static const int WAIL_EVERY = 2;           //rounds between the wail mechanic
static const int WAIL_HEAVY_PERCENT= 200;    //heavy damage wail = boss attack*this
static const int WAIL_DOT_PERCENT = 30;      //curse = boss attack x this (per turn)
static const int WAIL_DOT_TURNS = 3;

/*-------------------------------------------------------------------------------------------------------------------------------------------------------*/

//battle states
static bool battleActive = false;
static int battleResult= BATTLE_ONGOING;
static bool playerDefending = false;
static bool rewardGiven = false;
static int battleRow = -1;
static int battleCol = -1;
static int introIndex = 0;
static int targetHead = 0;
static int buffTurns= 0;
static int curseTurns = 0;
static int curseDamage= 0;
static int hydraRound = 0;      //rounds completed against the hydra
static int pendingItem = -1;    //waiting for inventory space
static int battleCounter = 0;
static int roundCounter = 0;
static int lastDamageToEnemy = 0;
static int lastDamageToPlayer = 0;
static int lastHitHead = -1;

/*-------------------------------------------------------------------------------------------------------------------------------------------------------*/

//message log: lines about the round that just happened
static const int MAX_LOG = 8;
static std::string logLines[MAX_LOG];
static int logCount = 0;

static void clearLog() { logCount = 0; }
static void addLog(const std::string& line)
{
    if (logCount < MAX_LOG) logLines[logCount++] = line;
}

/*-------------------------------------------------------------------------------------------------------------------------------------------------------*/
//queue for sound effects
static const int MAX_SOUNDS = 16;
static int soundQueue[MAX_SOUNDS];
static int soundCount = 0;

static void addSound(int id)
{
    if (soundCount < MAX_SOUNDS) soundQueue[soundCount++] = id;
}

int queueSound()
{
    if (soundCount == 0) return SND_NONE;
    int id = soundQueue[0];
    for (int i = 1; i < soundCount; i++) soundQueue[i - 1] = soundQueue[i];
    soundCount--;
    return id;
}

/*-------------------------------------------------------------------------------------------------------------------------------------------------------*/
//extras and misc

static int rnd(int n)
{
    return rand() % n;
}

static int rollDamage(int base)          //never below 1. + or -1 to atk
{
    int dmg = base + rnd(3) - 1;
    return dmg < 1 ? 1 : dmg;
}

static std::string num(int n)
{
    return std::to_string(n);
}

static bool playerAlive()
{
    return getPlayerHP() > 0;     //>< and other operators of the like already produce a bool so i didnt use an if structure for a cleaner code
}


//player takes a hit, choosing "defend" would half it basicallt.
static int hitPlayer(int dmg)    //returns dmg actually taken.
{
    if (dmg < 1) dmg = 1;
    if (playerDefending)
    {
        dmg = dmg / 2;
        if (dmg < 1) dmg = 1;
    }
    lastDamageToPlayer += dmg;
    takeDamage(dmg);
    addSound(playerDefending ? SND_BLOCK : SND_PLAYER_HURT);
    return dmg;
}

static std::string guardNote()
{
    return playerDefending ? " (guarded)" : "";
}

/*-------------------------------------------------------------------------------------------------------------------------------------------------------*/
//starting a battle + intro dialogue
//placeholder dialogue subjected to change maybe.

static const int SCIENTIST_INTRO_LINES = 3;
static const std::string SCIENTIST_INTRO[SCIENTIST_INTRO_LINES] = { "Ah, a visitor! Ha ha ha HA HA!",
    "Come closer, come closer... my experiments are always so eager to meet new subjects.",
    "Oh? You don't remember me? Wonderful! It's so much more fun that way!"
};

bool beginBattle(int bossType, int row, int col)
{
    if (bossType < 0 || bossType >= ENEMY_TYPE_COUNT) return false;
    if (isEnemyDefeated(row, col)) return false;

    createEnemy(bossType);
    battleCounter++;
    lastDamageToEnemy = lastDamageToPlayer = 0;
    lastHitHead = -1;
    battleActive= true;
    battleResult = BATTLE_ONGOING;
    playerDefending = false;
    rewardGiven = false;
    battleRow = row;
    battleCol = col;
    introIndex = 0;
    targetHead = 0;
    buffTurns = 0;
    curseTurns = 0;
    curseDamage= 0;
    hydraRound = 0;
    soundCount = 0;
    clearLog();

    if (bossType == ENEMY_MAD_SCIENTIST)
    {
        addSound(SND_SCIENTIST_LAUGH);
        addLog("The Mad Scientist grins at you from behind his workbench.");
    }
    else if (bossType == ENEMY_HUMANOID_BAT)
    {
        addSound(SND_BAT_SCREECH);
        addLog("The Humanoid Bat screeches and drops from the rafters!");
    }
    else
    {
        addSound(SND_HYDRA_WAIL);
        addLog("The Human Hydra rears up. Three heads, three wills.");
    }
    return true;
}


int getIntroLineCount()
{
    return (isBattleActive() && getEnemyType() == ENEMY_MAD_SCIENTIST) ? SCIENTIST_INTRO_LINES : 0;
}

std::string getIntroLine(int index)
{
    if (getEnemyType() == ENEMY_MAD_SCIENTIST && index >= 0 && index < SCIENTIST_INTRO_LINES)
        return SCIENTIST_INTRO[index];
    return "";
}

int getIntroIndex()
{
    return introIndex;
}

bool isInIntro()
{
    return battleActive && introIndex < getIntroLineCount();
}

void advanceIntro()
{
    if (isInIntro()) introIndex++;
}

/*-------------------------------------------------------------------------------------------------------------------------------------------------------*/
//player actions

void setTargetHead(int head) { targetHead = head; }
int  getTargetHead()         { return pickLivingHead(targetHead); }

void cycleTargetHead(int direction)
{
    if (getEnemyType() != ENEMY_HYDRA) return;
    int h = getTargetHead();
    for (int step = 0; step < HYDRA_HEADS; step++)
    {
        h = (h + direction + HYDRA_HEADS) % HYDRA_HEADS;
        if (isHeadAlive(h)) { targetHead = h; return; }
    }
}

void playerAtk()
{
    int head = getTargetHead();
    bool wasGuarding = (getEnemyType() == ENEMY_HYDRA) && isHeadDefending(head);

    int dmg = rollDamage(getplayerAtk());
    if (buffTurns > 0) dmg = dmg * BUFF_PERCENT / 100;

    int dealt = enemyTakeDamage(dmg, head);
    lastDamageToEnemy += dealt;
    lastHitHead = (getEnemyType() == ENEMY_HYDRA) ? head : -1;
    addSound(SND_PLAYER_HIT);

    if (getEnemyType() == ENEMY_HYDRA)
    {
        std::string who = "head " + num(head + 1);
        std::string line = "You strike " + who + " for " + num(dealt) + ".";
        if (wasGuarding) line += " It was guarding: half damage.";
        addLog(line);
        if (!isHeadAlive(head)) addLog("Head " + num(head + 1) + " falls silent!");
    }
    else
    {
        addLog("You hit " + getEnemyName() + " for " + num(dealt) + ".");
    }
}

void defend()
{
    playerDefending = true;
    addLog("You brace yourself. The next hits do half damage.");
}

bool isBattleItem(int itemId)
{
    return itemId == ITEM_HEALING_POTION || itemId == ITEM_POISON_POTION || itemId == ITEM_BUFF_POTION;
}

bool useBattleItem(int itemId)
{
    if (!isBattleItem(itemId))
    {
        addLog("That can't be used in battle.");
        return false;
    }
    if (!hasItem(itemId))
    {
        addLog("You don't have any " + getItemName(itemId) + "!");
        return false;
    }
    if (!useItem(itemId))
    {
        addLog("It had no effect.");
        return false;
    }

    if (itemId == ITEM_BUFF_POTION)
    {
        buffTurns = BUFF_TURNS;
        addSound(SND_BUFF);
        addLog("You drink the " + getItemName(itemId) + ". Power surges through you!");
    }
    else if (itemId == ITEM_HEALING_POTION)
    {
        addSound(SND_HEAL);
        addLog("You drink the " + getItemName(itemId) + " and feel better.");
    }
    else
    {
        addSound(SND_PLAYER_HURT);
        addLog("You drink the " + getItemName(itemId) + "...");
    }
    return true;
}

bool canRunInThisBattle() { return getEnemyType() == ENEMY_MAD_SCIENTIST; }

//the fake run button never works.
bool attemptRun()
{
    addSound(SND_SCIENTIST_LAUGH);
    addLog("You turn to run... the door slams shut!");
    addLog("\"Hahaha! Nowhere to go, Viktor!\"");
    return false;
}

/*-------------------------------------------------------------------------------------------------------------------------------------------------------*/
//bosses turns
static void scientistTurn()
{
    int power = getScientistPower();
    int atk = getEnemyAttackPower() * (100 + SCI_POWER_PERCENT * power) / 100;
    int roll = rnd(100);
    bool canHurtSelf = (power < SCI_MAX_POWER) &&
                       (getEnemyHP() * 100 > getEnemyMaxHP() * SCI_SAFE_HP_PERCENT);

    if (roll >= SCI_SELF_HARM_ROLL && canHurtSelf)
    {
        int loss = getEnemyMaxHP() * SCI_SELF_HARM_PERCENT / 100;
        if (loss < 1) loss = 1;
        int lost = scientistHurtSelf(loss);
        addScientistPower();
        addSound(SND_SCIENTIST_LAUGH);
        addLog("The Mad Scientist jabs himself with a syringe (-" + num(lost) + " HP) and howls with laughter!");
        addLog("His attacks grow stronger! (Power " + num(getScientistPower()) + "/" + num(SCI_MAX_POWER) + ")");
    }
    else if (roll >= SCI_WILD_ROLL && roll < SCI_SELF_HARM_ROLL)
    {
        int dmg = atk * rnd(201) / 100;      //anywhere from 0x to 2x
        if (dmg < 1)
        {
            addLog("The Mad Scientist swings wildly and misses!");
        }
        else
        {
            int taken = hitPlayer(dmg);
            addLog("He flails wildly and hits you for " + num(taken) + "!" + guardNote());
        }
    }
    else
    {
        int taken = hitPlayer(rollDamage(atk));
        addLog("The Mad Scientist zaps you for " + num(taken) + "." + guardNote());
    }
}

static void batTurn()
{
    if (rnd(100) < BAT_SCREECH_CHANCE && getBatCount() < MAX_BATS)
    {
        summonBat();
        addSound(SND_BAT_SCREECH);
        addLog("The Humanoid Bat screeches! Another bat joins the fight. (Bats: " + num(getBatCount()) + ")");
        return;
    }

    int swarmEach = getEnemyAttackPower() * BAT_SWARM_PERCENT / 100;
    if (swarmEach < 1) swarmEach = 1;
    int swarm = getBatCount() * swarmEach;

    int taken = hitPlayer(rollDamage(getEnemyAttackPower()) + swarm);
    std::string line = "The Humanoid Bat slashes you for " + num(taken) + ".";
    if (swarm > 0) line += " Its bats join in!";
    addLog(line + guardNote());
}

//a living head chooses to either attack or heal or defend. two can perform the same action, but all three cannot
static void chooseHeadActions(int act[HYDRA_HEADS])
{
    bool anyWounded = (getMostWoundedHead() >= 0);

    for (int i = 0; i < HYDRA_HEADS; i++)
    {
        act[i] = HEAD_NONE;
        if (!isHeadAlive(i)) continue;

        int roll = rnd(100);
        if (roll < 40)      act[i] = HEAD_ATTACK;
        else if (roll < 70) act[i] = HEAD_HEAL;
        else                act[i] = HEAD_DEFEND;

        if (act[i] == HEAD_HEAL && !anyWounded)          //nothing to heal
            act[i] = (rnd(2) == 0) ? HEAD_ATTACK : HEAD_DEFEND;
    }

    if (getLivingHeads() == HYDRA_HEADS && act[0] == act[1] && act[1] == act[2])
    {
        int k = rnd(HYDRA_HEADS);
        int options[2];
        int n = 0;
        for (int a = 0; a < 3; a++)
        {
            if (a == act[k]) continue;
            if (a == HEAD_HEAL && !anyWounded) continue;
            options[n++] = a;
        }
        act[k] = options[rnd(n)];
    }
}

static void tickCurse()
{
    if (curseTurns <= 0 || !playerAlive()) return;
    lastDamageToPlayer += curseDamage;
    takeDamage(curseDamage);
    curseTurns--;
    addSound(SND_PLAYER_HURT);
    addLog("The curse gnaws at you for " + num(curseDamage) + " HP. (" + num(curseTurns) + " turns left)");
}

static void hydraWail()
{
    int heads = getLivingHeads();
    addSound(SND_HYDRA_WAIL);

    if (rnd(2) == 0)
    {
        int dmg = getEnemyAttackPower() * WAIL_HEAVY_PERCENT * heads / 300;
        int taken = hitPlayer(dmg < 1 ? 1 : dmg);
        addLog("The heads wail together. A crushing scream hits you for " + num(taken) + "!" + guardNote());
    }
    else
    {
        int dot = getEnemyAttackPower() * WAIL_DOT_PERCENT * heads / 300;
        curseDamage = dot < 1 ? 1 : dot;
        curseTurns = WAIL_DOT_TURNS;
        addLog("The heads wail! A curse sinks in: -" + num(curseDamage) + " HP per turn for " + num(WAIL_DOT_TURNS) + " turns.");
    }
}

static void hydraTurn()
{
    for (int i = 0; i < HYDRA_HEADS; i++) setHeadDefending(i, false);

    int act[HYDRA_HEADS];
    chooseHeadActions(act);

    int headAtk = getEnemyAttackPower() * HEAD_ATTACK_PERCENT / 100;
    if (headAtk < 1) headAtk = 1;

    for (int i = 0; i < HYDRA_HEADS; i++)
    {
        setHeadAction(i, act[i]);
        if (act[i] == HEAD_NONE || !playerAlive()) continue;

        std::string who = "Head " + num(i + 1);
        if (act[i] == HEAD_ATTACK)
        {
            int taken = hitPlayer(rollDamage(headAtk));
            addLog(who + " bites you for " + num(taken) + "." + guardNote());
        }
        else if (act[i] == HEAD_HEAL)
        {
            int target = getMostWoundedHead();
            if (target >= 0)
            {
                int amount = getHeadMaxHP(target) * HEAD_HEAL_PERCENT / 100;
                if (amount < 1) amount = 1;
                int healed = healHead(target, amount);
                addLog(who + " heals head " + num(target + 1) + " (+" + num(healed) + ").");
            }
        }
        else
        {
            setHeadDefending(i, true);
            addLog(who + " raises its guard.");
        }
    }

    if (!playerAlive()) return;
    tickCurse();

    hydraRound++;
    if (hydraRound % WAIL_EVERY == 0 && playerAlive()) hydraWail();
}

static void enemyTurn()
{
    int type = getEnemyType();
    if (type == ENEMY_MAD_SCIENTIST)     scientistTurn();
    else if (type == ENEMY_HUMANOID_BAT) batTurn();
    else if (type == ENEMY_HYDRA)        hydraTurn();
}


/*-------------------------------------------------------------------------------------------------------------------------------------------------------*/
//reults rewards and stuff for rounds
int checkBattleResult()
{
    if (!battleActive) return battleResult;
    if (getPlayerHP() <= 0)       battleResult = BATTLE_LOST;
    else if (getEnemyHP() <= 0)   battleResult = BATTLE_WON;
    else                          battleResult = BATTLE_ONGOING;
    return battleResult;
}

static void giveItemReward(int itemId)
{
    if (addItem(itemId))
    {
        addLog("You obtained a " + getItemName(itemId) + ".");
    }
    else
    {
        pendingItem = itemId;
        addLog("Your pack is full! Make room to take the " + getItemName(itemId) + ".");
    }
}

void battleReward()
{
    if (rewardGiven) return;      //rewards wont be given twice
    rewardGiven = true;
    int type = getEnemyType();
    int exp = getEnemyExpReward();
    addSound(SND_VICTORY);
    addLog(getEnemyName() + " is defeated! +" + num(exp) + " EXP.");
    addExperience(exp);

    //rewards and exp etc conditions
    if (type == ENEMY_MAD_SCIENTIST)
    {
        increasePlayerMaxHP(exp / 2);
        increaseAttackLevel();
        addLog("Max HP +" + num(exp / 2) + ". Your attack grows stronger!");
        giveItemReward(ITEM_REAL_KEY);
    }
    else if (type == ENEMY_HUMANOID_BAT)
    {
        increasePlayerMaxHP(exp / 2);
        increaseAttackLevel();
        addLog("Max HP +" + num(exp / 2) + ". Your attack grows stronger!");
        giveItemReward(ITEM_DECOY_KEY);
    }
    else if (type == ENEMY_HYDRA)
    {
        addLog("The chest is within reach...");
    }
}

int playerAction(int action, int itemId)
{
    if (!battleActive || battleResult != BATTLE_ONGOING || isInIntro()) return battleResult;

    clearLog();
    lastDamageToEnemy = 0;
    lastDamageToPlayer = 0;
    lastHitHead = -1;
    bool turnUsed = true;     //false = invalid choice so no enemy response

    switch (action)
    {
    case ACTION_ATTACK: playerAtk(); break;        //put them in one lines, because you can do that apparently
    case ACTION_DEFEND: defend(); break;
    case ACTION_ITEM:   turnUsed = useBattleItem(itemId); break;
    case ACTION_RUN:
        if (canRunInThisBattle()) attemptRun();   //decoy option so the turn is wasted
        else turnUsed = false;
        break;
    default: turnUsed = false; break;
    }

    if (!turnUsed) return battleResult;
    roundCounter++;

    if (checkBattleResult() == BATTLE_ONGOING)
    {
        enemyTurn();
        playerDefending = false;
        if (buffTurns > 0) buffTurns--;
        checkBattleResult();
    }

    if (battleResult == BATTLE_WON)
        battleReward();
    else if (battleResult == BATTLE_LOST)
    {
        addSound(SND_DEFEAT);
        addLog("You collapse...");
    }
    return battleResult;
}

int endBattle()
{
    int result = battleResult;
    if (result == BATTLE_WON)
    {
        markEnemyDefeated(battleRow, battleCol);   //wont trigger it again 
        markBossTypeDefeated(getEnemyType());
    }
    battleActive    = false;
    playerDefending = false;
    return result;
}

bool hasPendingReward() { return pendingItem >= 0; }

bool retryPendingReward() //self explanatory
{
    if (pendingItem < 0) return false;
    if (!addItem(pendingItem)) return false;
    pendingItem = -1;
    return true;
}

//read only info for screen
int  battleCounter() { return battleCounter; }
int  roundCounter() { return roundCounter; }
int  lastDamageDealt() { return lastDamageToEnemy; }
int  lastDamageTaken() { return lastDamageToPlayer; }
int  lastHeadHit() { return lastHitHead; }
bool isBattleActive() { return battleActive; }
int  getBattleResult() { return battleResult; }
bool isPlayerDefending() { return playerDefending; }
int  playerBuffTurns() { return buffTurns; }
int  playerCurseTurns() { return curseTurns; }
int  playerCurseDamage() { return curseDamage; }
int  hydraTurnsUntilWail() { return WAIL_EVERY - (hydraRound % WAIL_EVERY); }
int  battleLogCount() { return logCount; }
bool isHydraDefeated() { return isBossTypeDefeated(ENEMY_HYDRA); }

std::string getBattleLogLine(int index)
{
    return (index >= 0 && index < logCount) ? logLines[index] : "";
}

void resetBattleSystem()
{
    battleActive = false;
    battleResult = BATTLE_ONGOING;
    playerDefending = false;
    rewardGiven = false;
    buffTurns = curseTurns = curseDamage = hydraRound = 0;
    pendingItem = -1;
    soundCount = 0;
    clearLog();
    resetDefeatedEnemies();
}