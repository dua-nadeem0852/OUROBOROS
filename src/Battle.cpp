//DUA NADEEM 26L-0852 member 3
//Battle.cpp - WHAT THIS FILE DOES: the combat rules. turns, damage, the 3 bosses' attacks, rewards and the sound queue. no drawing here (see BattleScreen.cpp)
#include "Battle.h"
#include "Enemy.h"
#include "PlayerInterface.h"
#include <cstdlib>
#include <cctype>

//tuning numbers for the fights

//buff potion (used in battle)
static const int BUFF_PERCENT = 150;      //plus 50% extra dmg
static const int BUFF_TURNS = 3;        //how many of your ATTACKS get the buff. drinking the potion itself does not use one up, and defending / other items don't either

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
static int battleRow = -1; //the map tile the boss stands on, so endBattle knows which tile to mark as beaten
static int battleCol = -1;
static int introIndex = 0;
static int targetHead = 0;   //which hydra head we are aiming at (0-2), only matters in the hydra fight
static int buffTurns= 0; //rounds left on the buff potion
static int curseTurns = 0;
static int curseDamage= 0; //hp lost every turn from the hydra curse
static int hydraRound = 0;      //rounds completed against the hydra
static int pendingItem = -1;    //waiting for inventory space
static int battleCounterVal = 0; //goes up by 1 every new battle. the screen watches it to know when to reset itself
static int roundCounterVal = 0; //goes up every time a round is really played. the screen uses it to know when to show the damage numbers + shake
//what happened in the LAST round. only stored so the screen can read it (floating numbers etc)
static int lastDamageToEnemy = 0; 
static int lastDamageToPlayer = 0;
static int lastHitHead = -1;
static bool delayEnemyTurn = false;     //the screen turns this on: the boss answers a moment AFTER the players move
static bool enemyTurnPending = false;   //the player moved and the boss still has to answer

/*-------------------------------------------------------------------------------------------------------------------------------------------------------*/

//message log: lines about the round that just happened
static const int MAX_LOG = 8;
static std::string logLines[MAX_LOG];
static int logCount = 0;

static void clearLog() { logCount = 0; }
static void addLog(const std::string& line)
{
    if (logCount < MAX_LOG) logLines[logCount++] = line;   //extra lines are just ignored instead of writing past the end of the array
}

/*-------------------------------------------------------------------------------------------------------------------------------------------------------*/
//queue for sound effects
static const int MAX_SOUNDS = 16;
static int soundQueue[MAX_SOUNDS];
static int soundCount = 0;

static void addSound(int id)
{
    if (soundCount < MAX_SOUNDS) soundQueue[soundCount++] = id; //if queue full then drop the sound. better than crashing
}

int queueSound()
{
    if (soundCount == 0) return SND_NONE;
    int id = soundQueue[0];
    //take the oldest sound out and shift everything else one spot left (first in first out)
    for (int i = 1; i < soundCount; i++) soundQueue[i - 1] = soundQueue[i];
    soundCount--;
    return id;
}

/*-------------------------------------------------------------------------------------------------------------------------------------------------------*/
//extras and misc

static int rnd(int n)
{
    return rand() % n;   //0 to n-1
}

static int rollDamage(int base)          //never below 1. + or -1 to atk
{
    int dmg = base + rnd(3) - 1;     //rnd(3) is 0, 1 or 2 so minus 1 gives -1, 0 or +1
    return dmg < 1 ? 1 : dmg;
}

static std::string num(int n)
{
    return std::to_string(n); //only here so i dont have to type std::to_string everywhere
}

static bool playerAlive()
{
    return getPlayerHP() > 0;     //>< and other operators of the like already produce a bool so i didnt use an if structure for a cleaner code
}

//picks the sound the boss makes when IT gets hurt (depends on which boss we are fighting)
static int bossHitSound()
{
    int type = getEnemyType();
    if (type == ENEMY_MAD_SCIENTIST) return SND_SCIENTIST_HIT;
    if (type == ENEMY_HUMANOID_BAT) return SND_BAT_HIT;
    if (type == ENEMY_HYDRA) return SND_HYDRA_HIT;
    return SND_NONE;
}

//player takes a hit, choosing "defend" halves it.
static int hitPlayer(int dmg)    //returns the HP the player really lost (what the log and the damage numbers show)
{
    if (dmg < 1) dmg = 1;
    if (playerDefending)
    {
        dmg = dmg / 2;
        if (dmg < 1) dmg = 1;
    }
    int before = getPlayerHP();
    takeDamage(dmg);   //the difficulty % is applied inside takeDamage (Player.cpp), so dmg is not always what gets lost
    int lost = before - getPlayerHP();       //so the number shown is the HP really lost (same idea as hurtPlayer in Map.cpp)
    lastDamageToPlayer += lost;       //+= because the hydra can hit more than once in one round
    addSound(playerDefending ? SND_BLOCK : SND_PLAYER_HURT);
    return lost;
}

//little bit of text added to the log when defend halves the hit
static std::string guardNote()
{
    return playerDefending ? " (guarded)" : "";
}

/*-------------------------------------------------------------------------------------------------------------------------------------------------------*/
//starting a battle + intro dialogue
static const int SCIENTIST_INTRO_LINES = 3;
static const std::string SCIENTIST_INTRO[SCIENTIST_INTRO_LINES] = { "Ah, a visitor! Ha ha ha HA HA!",
    "Come closer, come closer... my experiments are always so eager to meet new subjects.",
    "Oh? You don't remember me? Wonderful! It's so much more fun that way!"
};

bool beginBattle(int bossType, int row, int col)
{
    if (bossType < 0 || bossType >= ENEMY_TYPE_COUNT) return false;
    if (isEnemyDefeated(row, col)) return false;  //a beaten boss cant be fought twice

    createEnemy(bossType);    //builds the boss stats from the players current stats
    battleCounterVal++;
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
    soundCount = 0;        //throw away any sounds left over from before
    enemyTurnPending = false;
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

//only the scientist talks before the fight. the others start right away
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

//while this is true playerAction does nothing, so you cant attack during his speech
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

int  getTargetHead()  { return pickLivingHead(targetHead); }

void cycleTargetHead(int direction)
{
    if (getEnemyType() != ENEMY_HYDRA) return;
    int h = getTargetHead();
    //max 3 tries so it cant loop forever and dead heads get skipped
    for (int step = 0; step < HYDRA_HEADS; step++)
    {
        h = (h + direction + HYDRA_HEADS) % HYDRA_HEADS;   //+HYDRA_HEADS so going left from head 0 wraps to head 2 instead of -1
        if (isHeadAlive(h)) { targetHead = h; return; }
    }
}

void playerAtk()
{
    int head = getTargetHead();
    bool wasGuarding = (getEnemyType() == ENEMY_HYDRA) && isHeadDefending(head);  //checked BEFORE the hit, only for the log text

    int dmg = rollDamage(getplayerAtk());
    if (buffTurns > 0)
    {
        dmg = dmg * BUFF_PERCENT / 100;    //150/100 = x1.5. multiply first then divide, otherwise integer division would ruin it
        buffTurns--;                       //one buffed attack used up (only attacks count, not the round you drink it)
    }

    int dealt = enemyTakeDamage(dmg, head);   //dealt can be less than dmg (boss almost dead or head guarding) so the log uses dealt
    lastDamageToEnemy += dealt;
    lastHitHead = (getEnemyType() == ENEMY_HYDRA) ? head : -1;
    addSound(SND_PLAYER_HIT);       //the sword sound...
    addSound(bossHitSound());    //the boss's own "ouch" sound

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
    playerDefending = true;  //turns off again after the enemys turn (see playerAction)
    addSound(SND_DEFEND);
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
    int hpBefore = getPlayerHP();      //so poison can report how much it hurt
    //Player.cpp decides if it works (healing at full hp says no; the buff always works, at max attack level it only skips the permanent +1). if not, no turn is used
    if (!useItem(itemId))
    {
        addLog("It had no effect.");
        return false;
    }

    if (itemId == ITEM_BUFF_POTION)
    {
        buffTurns = BUFF_TURNS;        //useItem already raised the attack level (if it was not at max yet), this is the extra x1.5 for your next 3 attacks
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
        int lost = hpBefore - getPlayerHP();
        lastDamageToPlayer += lost;           //so the damage number shows like any other hit
        addSound(SND_PLAYER_HURT);
        addLog("You drink the " + getItemName(itemId) + "... it was poison! You lose " + num(lost) + " HP.");
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
    //+30% per power-up he has. power 0,1,2,3 = x1.0, x1.3, x1.6, x1.9
    int atk = getEnemyAttackPower() * (100 + SCI_POWER_PERCENT * power) / 100;
    int roll = rnd(100);
    //he can only hurt himself if he has power left to gain AND is above 30% hp. (hp*100 > max*30 is "hp above 30%" without decimals)
    bool canHurtSelf = (power < SCI_MAX_POWER) && (getEnemyHP() * 100 > getEnemyMaxHP() * SCI_SAFE_HP_PERCENT);

    //if the roll is 75+ but he cant hurt himself, it falls through to the normal attack at the bottom
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
    //30% chance he calls another bat instead of attacking. summoning uses up his turn
    if (rnd(100) < BAT_SCREECH_CHANCE && getBatCount() < MAX_BATS)
    {
        summonBat();
        addSound(SND_BAT_SCREECH);
        addLog("The Humanoid Bat screeches! Another bat joins the fight. (Bats: " + num(getBatCount()) + ")");
        return;
    }

    int swarmEach = getEnemyAttackPower() * BAT_SWARM_PERCENT / 100;
    if (swarmEach < 1) swarmEach = 1;
    //every bat that is already here adds some extra damage on top
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
        //40% attack, 30% heal, 30% defend
        if (roll < 40) act[i] = HEAD_ATTACK;
        else if (roll < 70) act[i] = HEAD_HEAL;
        else act[i] = HEAD_DEFEND;

        if (act[i] == HEAD_HEAL && !anyWounded)          //nothing to heal
            act[i] = (rnd(2) == 0) ? HEAD_ATTACK : HEAD_DEFEND;
    }

    //the rule: all 3 living heads cant pick the same thing, so one random head (k) switches to something different
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

//the curse from the wail hurts every hydra turn until the turns run out
static void tickCurse()
{
    if (curseTurns <= 0 || !playerAlive()) return;
    int before = getPlayerHP();
    takeDamage(curseDamage);
    int lost = before - getPlayerHP();       //the HP really lost (difficulty included)
    lastDamageToPlayer += lost;
    curseTurns--;
    addSound(SND_PLAYER_HURT);
    addLog("The curse gnaws at you for " + num(lost) + " HP. (" + num(curseTurns) + " turns left)");
}

static void hydraWail()
{
    int heads = getLivingHeads();
    addSound(SND_HYDRA_WAIL);

    //coin flip: one big scream or the curse (damage over time)
    if (rnd(2) == 0)
    {
        //the 300 is 100 (percent) x 3 heads. all heads alive = full wail. every dead head makes it weaker
        int dmg = getEnemyAttackPower() * WAIL_HEAVY_PERCENT * heads / 300;
        int taken = hitPlayer(dmg < 1 ? 1 : dmg);
        addLog("The heads wail together. A crushing scream hits you for " + num(taken) + "!" + guardNote());
    }
    else
    {
        int dot = getEnemyAttackPower() * WAIL_DOT_PERCENT * heads / 300;
        curseDamage = dot < 1 ? 1 : dot;
        curseTurns = WAIL_DOT_TURNS;
        addLog("The heads wail! A curse sinks in: -" + num(damageAfterDifficulty(curseDamage)) + " HP per turn for " + num(WAIL_DOT_TURNS) + " turns.");
    }
}

static void hydraTurn()
{
    for (int i = 0; i < HYDRA_HEADS; i++) setHeadDefending(i, false); //a guard only lasts one hydra turn so wipe them all first

    int act[HYDRA_HEADS];
    chooseHeadActions(act);

    int headAtk = getEnemyAttackPower() * HEAD_ATTACK_PERCENT / 100;
    if (headAtk < 1) headAtk = 1;

    for (int i = 0; i < HYDRA_HEADS; i++)
    {
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
    if (hydraRound % WAIL_EVERY == 0 && playerAlive()) hydraWail();   //every 2nd round (2, 4, 6...) the heads wail
}

//just picks which boss turn to run
static void enemyTurn()
{
    int type = getEnemyType();
    if (type == ENEMY_MAD_SCIENTIST) scientistTurn();
    else if (type == ENEMY_HUMANOID_BAT) batTurn();
    else if (type == ENEMY_HYDRA) hydraTurn();
}

/*-------------------------------------------------------------------------------------------------------------------------------------------------------*/
//reults rewards and stuff for rounds
int checkBattleResult()
{
    if (!battleActive) return battleResult;
    //player is checked first
    if (getPlayerHP() <= 0) battleResult = BATTLE_LOST;
    else if (getEnemyHP() <= 0) battleResult = BATTLE_WON;
    else battleResult = BATTLE_ONGOING;
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
        pendingItem = itemId;  //bag full so keep it here and try again later 
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
        increasePlayerMaxHP(exp / 2);     //half of the exp is given as bonus max hp too
        increaseAttackLevel();
        addLog("Max HP +" + num(exp / 2) + ". Your attack grows stronger!");
    }
    else if (type == ENEMY_HUMANOID_BAT)
    {
        increasePlayerMaxHP(exp / 2);
        increaseAttackLevel();
        addLog("Max HP +" + num(exp / 2) + ". Your attack grows stronger!");
        giveItemReward(ITEM_DECOY_KEY); 
    }         //the decoy key is what makes the chest show up but it cant open it
        
    else if (type == ENEMY_HYDRA)
    {
        giveItemReward(ITEM_REAL_KEY);       //the proposal: the final boss gives the real key
        addLog("The chest is within reach...");
    }
}

//the second half of a round: the boss answers. done right away, or later if the screen asked for a pause (setEnemyTurnDelay)
static void bossAnswers()
{
    enemyTurn();
    playerDefending = false;   //defend only covers this one enemy turn
    checkBattleResult();
}

//the end of a round: rewards if the boss is down, the defeat sound if the player is
static void settleRound()
{
    if (battleResult == BATTLE_WON)
        battleReward();
    else if (battleResult == BATTLE_LOST)
    {
        addSound(SND_DEFEAT);
        addLog("You collapse...");
    }
}

int playerAction(int action, int itemId)
{
    //ignore the key if the fight isnt running, is already over, the scientist is still talking or the boss still has to answer
    if (!battleActive || battleResult != BATTLE_ONGOING || isInIntro() || enemyTurnPending) return battleResult;

    clearLog();
    lastDamageToEnemy = 0;
    lastDamageToPlayer = 0;
    lastHitHead = -1;
    bool turnUsed = true;     //false = invalid choice so no enemy response

    switch (action)
    {
    case ACTION_ATTACK: playerAtk(); break;        //put them in one lines, because you can do that apparently
    case ACTION_DEFEND: defend(); break;
    case ACTION_ITEM: turnUsed = useBattleItem(itemId); break;
    case ACTION_RUN:
        if (canRunInThisBattle()) attemptRun();   //decoy option so the turn is wasted
        else turnUsed = false;
        break;
    default: turnUsed = false; break;
    }

    if (!turnUsed) return battleResult;  //bad choice (no potion etc). no round passes so the boss doesnt get a free hit
    roundCounterVal++;

    //the boss only acts if he survived the players move
    if (checkBattleResult() == BATTLE_ONGOING)
    {
        if (delayEnemyTurn)       //the screen wants a small pause first, so the boss answers later (runPendingEnemyTurn)
        {
            enemyTurnPending = true;
            return battleResult;
        }
        bossAnswers();
    }

    settleRound();
    return battleResult;
}

void setEnemyTurnDelay(bool on) { delayEnemyTurn = on; }
bool isEnemyTurnPending() { return enemyTurnPending; }

void runPendingEnemyTurn()
{
    if (!enemyTurnPending) return;
    enemyTurnPending = false;
    if (!battleActive || battleResult != BATTLE_ONGOING) return;

    clearLog();                  //the players lines were read during the pause, so the boss gets a clean text box
    lastDamageToEnemy = 0;       //only what the boss does now (the screens numbers and shake would repeat the players hit otherwise)
    lastDamageToPlayer = 0;
    lastHitHead = -1;
    roundCounterVal++;           //counts as its own step for the screen, so it shows the effects of this half too
    bossAnswers();
    settleRound();
}

int endBattle()
{
    int result = battleResult;
    if (result == BATTLE_WON)
    {
        markEnemyDefeated(battleRow, battleCol);   //wont trigger it again 
        markBossTypeDefeated(getEnemyType());    //the map asks "which tile" and the rest of the game asks "which boss" so both get marked
    }
    battleActive    = false;
    playerDefending = false;
    enemyTurnPending = false;
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
int battleCounter() { return battleCounterVal; }
int roundCounter() { return roundCounterVal; }
int lastDamageDealt() { return lastDamageToEnemy; }
int lastDamageTaken() { return lastDamageToPlayer; }
int lastHeadHit() { return lastHitHead; }
bool isBattleActive() { return battleActive; }
int getBattleResult() { return battleResult; }
bool isPlayerDefending() { return playerDefending; }
int playerBuffTurns() { return buffTurns; }
int playerCurseTurns() { return curseTurns; }
int playerCurseDamage() { return damageAfterDifficulty(curseDamage); }   //what one curse tick really costs (difficulty included)
//goes 2, 1, 2, 1... the screen uses it for the wail warning
int hydraTurnsUntilWail() { return WAIL_EVERY - (hydraRound % WAIL_EVERY); }
int battleLogCount() { return logCount; }

std::string getBattleLogLine(int index)
{
    //a bad index gives "" instead of reading outside the array
    return (index >= 0 && index < logCount) ? logLines[index] : "";
}

//which words of a log line are about HP. the battle screen draws those in red.
//the line is split on spaces, one true/false for each word. red = the word "HP" and the numbers that are an amount of hp:
//"+15" / "-4" (with a sign), the number after "for" (damage) or a number right before "HP".
//not red: head numbers ("head 2"), "Power 1/3", "Bats: 2", "3 turns", "+50 EXP"
static std::string bareWord(const std::string& s)     //the word without the signs/punctuation around it: "(+6)." -> "6", "HP)" -> "HP"
{
    size_t a = 0, b = s.size();
    while (a < b && !isalnum((unsigned char)s[a])) a++;
    while (b > a && !isalnum((unsigned char)s[b - 1])) b--;
    return s.substr(a, b - a);
}

std::vector<bool> hpWordFlags(const std::string& line)
{
    std::vector<std::string> words;
    std::string cur;
    for (size_t i = 0; i <= line.size(); i++)
    {
        if (i < line.size() && line[i] != ' ') { cur += line[i]; continue; }
        if (!cur.empty()) words.push_back(cur);
        cur.clear();
    }

    std::vector<bool> red(words.size(), false);
    for (size_t i = 0; i < words.size(); i++)
    {
        std::string w = bareWord(words[i]);
        if (w == "HP") { red[i] = true; continue; }

        size_t digit = words[i].find_first_of("0123456789");
        if (digit == std::string::npos) continue;               //no number in this word

        std::string prev = (i > 0) ? bareWord(words[i - 1]) : "";
        std::string next = (i + 1 < words.size()) ? bareWord(words[i + 1]) : "";
        if (next == "EXP" || next == "turns" || next == "turn") continue;     //"+50 EXP", "for 3 turns", "(2 turns left)"

        bool hasSign = (digit > 0 && (words[i][digit - 1] == '+' || words[i][digit - 1] == '-'));
        if (hasSign || prev == "for" || next == "HP") red[i] = true;
    }
    return red;
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
    enemyTurnPending = false;
    clearLog();
    resetDefeatedEnemies();   //new game, meaning every boss can be fought again
}
