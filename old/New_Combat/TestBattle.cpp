//g++ -std=c++11 TestBattle.cpp Battle.cpp Enemy.cpp PlayerStub.cpp -o test_battle
#include <iostream>
#include <cstdlib>
#include <string>
#include "Battle.h"
#include "Enemy.h"
#include "PlayerInterface.h"

//helpers that exist only in PlayerStub.cpp
void stubSetHP(int v); 
void stubSetDifficulty(int d);
void stubSetCapacity(int c);
int stubExp();
int stubCount(int id);
void stubSetCount(int id, int n);
void stubSetPlayer(int maxHP, int atkLevel, int healing, int buff, int poison);

static int passed = 0, failed = 0;
static void check(bool ok, const char* name)
{
    std::cout << (ok ? "[PASS] " : "[FAIL] ") << name << "\n";
    if (ok) passed++; else failed++;
}

static void skipIntro() { while (isInIntro()) advanceIntro(); }

static bool logContains(const std::string& word)
{
    for (int i = 0; i < battleLogCount(); i++)
        if (getBattleLogLine(i).find(word) != std::string::npos) return true;
    return false;
}

//to attack until the battle is decided. keeping the player alive.
static int fightToEnd()
{
    for (int i = 0; i < 800 && getBattleResult() == BATTLE_ONGOING; i++)
    {
        stubSetHP(getPlayerMaxHP());
        if (getEnemyType() == ENEMY_HYDRA)
        {
            int best = -1;
            for (int h = 0; h < HYDRA_HEADS; h++)
                if (isHeadAlive(h) && (best < 0 || getHeadHP(h) < getHeadHP(best))) best = h;
            setTargetHead(best);
        }
        playerAction(ACTION_ATTACK);
    }
    return getBattleResult();
}

int main(int argc, char** argv)
{
    int seed = (argc > 1) ? atoi(argv[1]) : 42;     //test with a different random seed
    srand(seed);
    resetBattleSystem();

    //difficulty scaling
    stubSetPlayer(100, 1, 2, 1, 1);
    stubSetDifficulty(DIFF_EASY);   createEnemy(ENEMY_MAD_SCIENTIST); int easyHP = getEnemyMaxHP(); int easyAtk = getEnemyAttackPower();
    stubSetDifficulty(DIFF_NORMAL); createEnemy(ENEMY_MAD_SCIENTIST); int normHP = getEnemyMaxHP(); int normAtk = getEnemyAttackPower();
    stubSetDifficulty(DIFF_HARD);   createEnemy(ENEMY_MAD_SCIENTIST); int hardHP = getEnemyMaxHP(); int hardAtk = getEnemyAttackPower();
    check(easyHP < 100 && easyAtk < getplayerAtk(), "Easy: boss weaker than the player");
    check(normHP > 100 && normHP < 130 && normAtk > easyAtk, "Normal: boss slightly above the player");
    check(hardHP > normHP + 20 && hardAtk > normAtk, "Hard: boss far above the player");
    stubSetDifficulty(DIFF_NORMAL);
    check(!beginBattle(99, 0, 0), "invalid boss type rejected");

    //mad scientist
    stubSetPlayer(100, 1, 2, 1, 1);
    resetBattleSystem();
    check(beginBattle(ENEMY_MAD_SCIENTIST, 3, 4), "start battle with the Mad Scientist");
    check(isInIntro() && getIntroLineCount() == 3, "Scientist has intro dialogue");
    int hpBefore = getEnemyHP();
    playerAction(ACTION_ATTACK);
    check(getEnemyHP() == hpBefore, "cannot act during the intro dialogue");
    skipIntro();
    check(!isInIntro(), "intro can be advanced to the end");

    //decoy run to waste turn
    playerAction(ACTION_RUN);
    check(getBattleResult() == BATTLE_ONGOING && logContains("Nowhere"), "decoy attemptRunfails and the Scientist mocks you");
    check(lastDamageDealt() == 0 && getEnemyHP() <= hpBefore, "decoy attemptRundoes not hurt the Scientist");   // (he may hurt HIMSELF on his turn)
    check(canRunInThisBattle(), "attemptRunbutton is offered vs the Scientist");

    //scientist wont die. his powerups are also fixed
    createEnemy(ENEMY_MAD_SCIENTIST);
    scientistHurtSelf(99999);
    check(getEnemyHP() == 1, "Scientist can never kill himself");
    beginBattle(ENEMY_MAD_SCIENTIST, 7, 7); skipIntro();
    int maxPower = 0; bool everHealed = false; int lastHP = getEnemyHP();
    for (int i = 0; i < 300 && getBattleResult() == BATTLE_ONGOING; i++)
    {
        stubSetHP(100);
        playerAction(ACTION_DEFEND);
        if (getScientistPower() > maxPower) maxPower = getScientistPower();
        if (getEnemyHP() > lastHP) everHealed = true;
        lastHP = getEnemyHP();
    }
    check(maxPower >= 1 && maxPower <= 3, "Scientist powers up by hurting himself (max 3)");
    check(!everHealed, "Scientist never heals");
    endBattle();

    //rewards and wins
    resetBattleSystem();
    stubSetPlayer(100, 1, 2, 1, 1);
    beginBattle(ENEMY_MAD_SCIENTIST, 3, 4); skipIntro();
    check(fightToEnd() == BATTLE_WON, "Mad Scientist can be defeated");
    check(getEnemyHP() == 0, "enemy HP is exactly 0 (not negative)");
    check(stubCount(ITEM_REAL_KEY) == 1 && stubCount(ITEM_DECOY_KEY) == 0, "Scientist gives the REAL key");
    check(stubExp() == 50, "Scientist gives 50 EXP");
    check(getPlayerMaxHP() == 125, "max HP 100 -> 125");
    check(getplayerAtkLevel() == 2, "attack level 1 -> 2");
    battleReward();
    check(stubCount(ITEM_REAL_KEY) == 1 && getPlayerMaxHP() == 125, "reward cannot be given twice");
    endBattle();
    check(isEnemyDefeated(3, 4) && isBossTypeDefeated(ENEMY_MAD_SCIENTIST), "boss marked defeated");
    check(!beginBattle(ENEMY_MAD_SCIENTIST, 3, 4), "defeated boss does not re-trigger");
    check(getItemName(ITEM_REAL_KEY) == "Key" && getItemName(ITEM_DECOY_KEY) == "Key", "both keys are shown as 'Key'");

   //itms and rules for defense
    resetBattleSystem();
    stubSetPlayer(100, 1, 1, 1, 0);
    beginBattle(ENEMY_HUMANOID_BAT, 5, 5);
    check(!canRunInThisBattle(), "no attemptRunbutton vs the Bat");
    stubSetHP(100);
    playerAction(ACTION_RUN);
    check(getPlayerHP() == 100 && getBattleResult() == BATTLE_ONGOING, "attemptRunvs the Bat does nothing and costs no turn");

    int r0 = roundCounter();
    playerAction(ACTION_RUN);   //not allowed against the Bat
    check(roundCounter() == r0, "an invalid action is not counted as a round");
    playerAction(ACTION_ATTACK);
    check(roundCounter() == r0 + 1 && lastDamageDealt() > 0, "an attack counts as a round and records its damage");

    stubSetHP(50);
    playerAction(ACTION_ITEM, ITEM_POISON_POTION);
    check(logContains("don't have"), "using an item you don't have is refused");
    check(stubCount(ITEM_HEALING_POTION) == 1, "refused item does not use up another item");
    playerAction(ACTION_ITEM, ITEM_HEALING_POTION);
    check(stubCount(ITEM_HEALING_POTION) == 0 && getPlayerHP() > 50 - 40, "healing potion is consumed and works");
    check(!logContains("don't have"), "healing potion used normally");
    playerAction(ACTION_ITEM, ITEM_RELIC);
    check(logContains("can't be used"), "non-battle items can't be used in battle");

    playerAction(ACTION_ITEM, ITEM_BUFF_POTION);
    check(playerBuffTurns() == 2, "buff potion lasts 3 rounds (2 left after this one)");
    check(isBattleItem(ITEM_BUFF_POTION) && !isBattleItem(ITEM_REAL_KEY) && !isBattleItem(ITEM_RELIC), "only potions are battle items");
    check(getSlotItemId(0) == ITEM_BUFF_POTION || getSlotItemId(0) == ITEM_HEALING_POTION || getSlotItemId(0) == -1, "inventory slots can be read");

    //defend: over many rounds.
    //no single round costs more than half of the bat's biggest possible hit
    int atk = getEnemyAttackPower();
    int swarmEach = atk * 25 / 100; if (swarmEach < 1) swarmEach = 1;
    int biggestHalfHit = ((atk + 1) + MAX_BATS * swarmEach) / 2;
    int worstLoss = 0; bool defendCleared = true;
    for (int i = 0; i < 40 && getBattleResult() == BATTLE_ONGOING; i++)
    {
        stubSetHP(100);
        playerAction(ACTION_DEFEND);
        int loss = 100 - getPlayerHP();
        if (loss > worstLoss) worstLoss = loss;
        if (isPlayerDefending()) defendCleared = false;
    }
    check(worstLoss > 0 && worstLoss <= biggestHalfHit, "defend halves the damage taken");
    check(defendCleared, "defend wears off after the enemy's turn");
    endBattle();

    //bat
    resetBattleSystem();
    stubSetPlayer(125, 2, 0, 0, 0);
    beginBattle(ENEMY_HUMANOID_BAT, 5, 5);
    check(!isInIntro(), "Bat has no dialogue");
    int maxBats = 0; bool batHealed = false; int prevHP = getEnemyHP();
    for (int i = 0; i < 200 && getBattleResult() == BATTLE_ONGOING; i++)
    {
        stubSetHP(125);
        playerAction(ACTION_DEFEND);
        if (getBatCount() > maxBats) maxBats = getBatCount();
        if (getEnemyHP() > prevHP) batHealed = true;
        prevHP = getEnemyHP();
    }
    check(maxBats >= 1 && maxBats <= MAX_BATS, "Bat summons bats by screeching (max 3)");
    check(!batHealed, "Bat never heals");
    endBattle();

    resetBattleSystem();
    stubSetPlayer(125, 2, 0, 0, 0);
    beginBattle(ENEMY_HUMANOID_BAT, 5, 5);
    check(fightToEnd() == BATTLE_WON, "Humanoid Bat can be defeated");
    check(stubCount(ITEM_DECOY_KEY) == 1 && stubCount(ITEM_REAL_KEY) == 0, "Bat gives the DECOY key");
    check(stubExp() == 100, "Bat gives 100 EXP");
    check(getPlayerMaxHP() == 175 && getplayerAtkLevel() == 3, "max HP 125 -> 175, attack level 3/3");
    endBattle();

    //when inventory full and pending rewads
    resetBattleSystem();
    stubSetPlayer(100, 1, 0, 0, 0);
    stubSetCount(ITEM_RELIC, 1);
    stubSetCapacity(1);                      //only one slot which is already used by the relic
    beginBattle(ENEMY_MAD_SCIENTIST, 1, 1); skipIntro();
    fightToEnd();
    check(hasPendingReward() && stubCount(ITEM_REAL_KEY) == 0, "full inventory: key is held as a pending reward");
    check(!retryPendingReward(), "retry fails while still full");
    stubSetCapacity(6);
    check(retryPendingReward() && stubCount(ITEM_REAL_KEY) == 1 && !hasPendingReward(), "retry works once there is room");
    endBattle();

    //hydra
    resetBattleSystem();
    stubSetPlayer(175, 3, 0, 0, 0);
    beginBattle(ENEMY_HYDRA, 12, 12);
    check(getEnemyName() == "Human Hydra" && !isInIntro(), "Hydra: name, no dialogue");
    check(getLivingHeads() == 3 && getHeadMaxHP(0) > 0, "Hydra has three heads with their own HP");
    check(getEnemyMaxHP() == getHeadMaxHP(0) + getHeadMaxHP(1) + getHeadMaxHP(2), "total HP is the sum of the heads");

    //guarded head takes half damage
    setHeadDefending(0, true);
    check(enemyTakeDamage(10, 0) == 5, "a guarding head takes half damage");
    setHeadDefending(0, false);
    check(enemyTakeDamage(10, 1) == 10, "an unguarded head takes full damage");

    //heads never all do the same thing. wail every 2 rounds mechanic.
    createEnemy(ENEMY_HYDRA); resetBattleSystem(); beginBattle(ENEMY_HYDRA, 12, 12);
    bool neverAllSame = true, wailTimingOK = true, sawCurse = false, sawHeavy = false;
    int actionSeen[3] = { 0, 0, 0 };
    for (int round = 1; round <= 40; round++)
    {
        stubSetHP(175);
        for (int h = 0; h < HYDRA_HEADS; h++) { healHead(h, 9999); enemyTakeDamage(10, h); }   //keep heads hurt so healing mechqnic can be used
        playerAction(ACTION_DEFEND); 
        int a0 = getHeadAction(0), a1 = getHeadAction(1), a2 = getHeadAction(2);
        if (a0 == a1 && a1 == a2) neverAllSame = false;
        actionSeen[a0]++; actionSeen[a1]++; actionSeen[a2]++;
        bool wailed = logContains("wail");
        if (wailed != (round % 2 == 0)) wailTimingOK = false;
        if (logContains("curse")) sawCurse = true;
        if (logContains("crushing")) sawHeavy = true;
        if (playerCurseTurns() > 3) wailTimingOK = false;
    }
    check(neverAllSame, "the three heads never all pick the same action");
    check(actionSeen[HEAD_ATTACK] > 0 && actionSeen[HEAD_HEAL] > 0 && actionSeen[HEAD_DEFEND] > 0, "heads use attack, heal and defend");
    check(wailTimingOK, "all heads wail after every 2 full rounds (and only then)");
    check(sawCurse && sawHeavy, "wail is sometimes a curse, sometimes heavy damage");
    endBattle();

    //curse chips away health over time
    resetBattleSystem();
    stubSetPlayer(175, 3, 0, 0, 0);
    beginBattle(ENEMY_HYDRA, 12, 12);
    bool chipped = false;
    for (int round = 1; round <= 60 && !chipped; round++)
    {
        stubSetHP(175);
        playerAction(ACTION_DEFEND);
        if (playerCurseTurns() > 0)
        {
            int before = getPlayerHP();
            stubSetHP(175);
            playerAction(ACTION_DEFEND);      //in next round the curse ticks
            chipped = logContains("curse gnaws");
            (void)before;
        }
    }
    check(chipped, "the curse deals damage over several turns");
    endBattle();

    //to beat the hydra all heads must fall. granted 200 EXP, chest unlocked.
    resetBattleSystem();
    stubSetPlayer(175, 3, 0, 0, 0);
    beginBattle(ENEMY_HYDRA, 12, 12);
    check(!isHydraDefeated(), "chest still locked before the fight");
    int result = fightToEnd();
    check(result == BATTLE_WON && getLivingHeads() == 0, "Hydra is only beaten when all 3 heads are down");
    check(stubExp() == 200, "Hydra gives 200 EXP");
    check(stubCount(ITEM_REAL_KEY) == 0 && stubCount(ITEM_DECOY_KEY) == 0, "Hydra gives no key");
    endBattle();
    check(isHydraDefeated(), "defeating the Hydra opens the chest");

    //losing and sounds
    resetBattleSystem();
    stubSetPlayer(100, 1, 0, 0, 0);
    stubSetHP(1);
    beginBattle(ENEMY_HUMANOID_BAT, 9, 9);
    while (queueSound() != SND_NONE) {}
    for (int i = 0; i < 20 && getBattleResult() == BATTLE_ONGOING; i++) playerAction(ACTION_ATTACK);
    check(getBattleResult() == BATTLE_LOST, "player can lose");
    check(getPlayerHP() == 0, "player HP never goes below 0");
    check(playerAction(ACTION_ATTACK) == BATTLE_LOST, "no actions after the battle ends");
    bool sawHit = false, sawHurt = false, sawDefeat = false;
    for (int s = queueSound(); s != SND_NONE; s = queueSound())
    {
        if (s == SND_PLAYER_HIT) sawHit = true;
        if (s == SND_PLAYER_HURT) sawHurt = true;
        if (s == SND_DEFEAT) sawDefeat = true;
    }
    check(sawHit && sawHurt && sawDefeat, "hit, hurt and defeat sounds are queued");
    check(queueSound() == SND_NONE, "sound queue empties");
    endBattle();
    check(!isEnemyDefeated(9, 9), "a lost battle does not mark the boss defeated");

    std::cout << "\n" << passed << " passed, " << failed << " failed\n";
    return failed == 0 ? 0 : 1;
}
