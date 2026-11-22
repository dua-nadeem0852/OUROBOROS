// TestMap.cpp - checks the map rules (walking, traps, relics, chest) without opening a window
// WHAT THIS FILE DOES: a test program. it also checks the walls, npcs, clues, the difficulty traps,
// that no wall cuts off an item, saving/loading and the high score. run it with scripts/run_map_test.bat
// the two UiDraw functions the map calls are faked below so UiDraw.cpp is not needed
#include <iostream>
#include <fstream>
#include "Map.h"
#include "Enemy.h"
#include "Battle.h"
#include "PlayerInterface.h"
#include "SaveSystem.h"
#include <cstdio>
#include <iterator>
#include <string>
sf::Texture* loadTexture(const std::string&, bool) { return 0; }
void drawCenteredText(sf::RenderWindow&, const sf::Font&, const std::string&, unsigned, float, float, sf::Color) {}
int fails = 0;
void check(bool ok, const char* what) { std::cout << (ok ? "[PASS] " : "[FAIL] ") << what << "\n"; if (!ok) fails++; }
// walk with a list like "UUL" to a place and return the last step result
int walk(const char* path) { int res = RESULT_NOTHING; for (const char* p = path; *p; p++) {
    int dr = 0, dc = 0; if (*p=='U') dr=-1; if (*p=='D') dr=1; if (*p=='L') dc=-1; if (*p=='R') dc=1;
    if (movePlayer(dr, dc)) res = stepOnTile(); } return res; }
int main() {
    resetPlayerToDefault(); resetBattleSystem(); initializeMap();
    check(getPlayerRow()==4 && getPlayerCol()==4, "player starts at 4,4");
    check(isRevealed(3,3) && !isRevealed(0,0), "start area revealed, far corner hidden");
    for (int i = 0; i < 6; i++) movePlayer(-1,0);
    check(getPlayerRow()==0, "cannot walk off the top edge");
    check(!movePlayer(-1,0), "movePlayer says false at the edge");
    initializeMap();
    check(interactWithTile()==RESULT_ITEM && hasItem(ITEM_HEALING_POTION), "start bush gives a healing potion");
    check(getTileType(4,4)=='.', "searched bush becomes empty");
    initializeMap();                         // there is a trap at 4,3
    int hp = getPlayerHP();
    check(!isTileTriggered(4,3), "an untouched trap is still hidden");
    check(walk("L")==RESULT_TRAP && getPlayerHP() < hp && getTileType(4,3)=='T' && isTileTriggered(4,3), "trap hurts and is now revealed (triggered)");
    hp = getPlayerHP();
    walk("R"); walk("L");
    check(getPlayerHP() == hp, "a trap that already went off does not hurt again");
    initializeMap();                         // traps at 4,8 and a hole at 4,9
    walk("RRRR");
    check(getPlayerHP() < 100, "second trap at 4,8 hurts too");
    check(!isTileTriggered(4,9), "an untouched hole is still hidden");
    check(walk("R")==RESULT_HOLE && getPlayerRow()==4 && getPlayerCol()==4 && isTileTriggered(4,9), "hole hurts, sends player back to start and is revealed");
    initializeMap();                         // relic at 2,4 and the scientist at 2,2
    walk("UU");
    check(getTileType(2,4)=='R' && interactWithTile()==RESULT_RELIC && getRelicsFound()==1 && getLastRelicIndex()==0, "relic found, counted, story index 0");
    initializeMap(); walk("UURRR");           // relic at 2,7 is the bat brooch (story 3)
    check(interactWithTile()==RESULT_RELIC && getLastRelicIndex()==3, "relic at 2,7 always gives the bat brooch story");
    initializeMap(); walk("DRRR");            // relic at 5,7 is the child's drawing (story 2)
    check(interactWithTile()==RESULT_RELIC && getLastRelicIndex()==2, "relic at 5,7 always gives the drawing story, whatever the pickup order");
    initializeMap(); walk("UU");
    check(walk("LL")==RESULT_ENEMY && getEnemyTypeAt(2,2)==ENEMY_MAD_SCIENTIST, "scientist tile starts a battle");
    check(beginBattle(ENEMY_MAD_SCIENTIST,2,2), "combat accepts the battle");
    markEnemyDefeated(2,2); markBossTypeDefeated(ENEMY_MAD_SCIENTIST);
    check(walk("R")==RESULT_NOTHING && walk("L")==RESULT_NOTHING, "defeated scientist tile no longer triggers");
    check(getEnemyTypeAt(1,8)==ENEMY_HUMANOID_BAT && getEnemyTypeAt(CHEST_ROW,CHEST_COL)==ENEMY_HYDRA, "bat is on the map, hydra is at the chest");
    check(getTileType(9,6)=='.' && getEnemyTypeAt(9,6)==ENEMY_NONE, "the old hydra tile at 9,6 is just floor now");
    resetBattleSystem(); initializeMap(); resetPlayerToDefault();
    check(!isChestVisible(), "chest is not visible at the start of a game");
    check(getTile(2,2).type=='E' && !getTile(2,2).enemyDefeated, "Tile struct: scientist tile is an enemy tile that is not beaten");
    markEnemyDefeated(2,2); markBossTypeDefeated(ENEMY_MAD_SCIENTIST);
    walk("UULL");
    check(getTile(2,2).enemyDefeated, "Tile struct: enemyDefeated is filled in after the boss is beaten");
    initializeMap(); resetBattleSystem(); resetPlayerToDefault();
    // chest progression: it is hidden until the Bat's Decoy Key is received.
    initializeMap(); resetBattleSystem(); resetPlayerToDefault();
    int guard = 0; while ((getPlayerRow()!=5 || getPlayerCol()!=8) && guard++ < 60) {
        if (getPlayerRow() < 5) movePlayer(1,0); else if (getPlayerRow() > 5) movePlayer(-1,0);
        else if (getPlayerCol() < 8) movePlayer(0,1); else movePlayer(0,-1); }
    check(getTileType(5,8)=='.' && interactWithTile()==RESULT_NOTHING, "chest is hidden and inactive before the Decoy Key");
    check(stepOnTile()==RESULT_NOTHING, "standing on the chest square never starts the hydra fight by itself");
    addItem(ITEM_DECOY_KEY);
    check(isChestVisible(), "chest shows up after the Decoy Key");
    check(interactWithTile()==RESULT_CHEST_BOSS, "Decoy Key reveals chest and starts its dialogue event");
    markBossTypeDefeated(ENEMY_HYDRA);
    check(interactWithTile()==RESULT_LOCKED, "defeating Hydra without the Real Key leaves chest locked");
    addItem(ITEM_REAL_KEY);
    check(interactWithTile()==RESULT_CHEST && hasProgressFlag(PROGRESS_BEAT_GAME), "Real Key opens it and sets the flag");
    // ---- walls, npcs, clues ----
    initializeMap();
    check(getTileType(1,2)=='W' && getTileType(4,2)=='N' && getTileType(3,5)=='?', "walls, npc and clue are on the map");
    check(everythingReachable(), "walls do not cut anything off (normal)");
    walk("UUU");                             // 4,4 -> 1,4
    walk("L");                               // 1,3 is a wall
    check(getPlayerCol()==4 && getMapMessage()=="A wall blocks the way.", "a wall blocks the player");
    initializeMap(); walk("LL");             // 4,2 = npc
    check(interactWithTile()==RESULT_TALK && getMapMessage()!="", "talking to the npc gives text");
    initializeMap(); walk("U"); walk("R");   // 3,5 = clue
    check(interactWithTile()==RESULT_TALK && getMapMessage().find("lie")!=std::string::npos, "clue can be read");

    // ---- the old hydra tile no longer starts a fight ----
    initializeMap(); resetBattleSystem(); resetPlayerToDefault();
    int fightStarted = 0;
    for (int r = 4; r < 9; r++) { movePlayer(1,0); if (stepOnTile()==RESULT_ENEMY) fightStarted++; }
    movePlayer(0,1); if (stepOnTile()==RESULT_ENEMY) fightStarted++;
    movePlayer(0,1); if (stepOnTile()==RESULT_ENEMY) fightStarted++;
    check(fightStarted==0, "walking over 9,6 (where the hydra used to be) does not start a fight");

    // ---- difficulty ----
    setDifficulty(DIFF_HARD); initializeMap();
    check(getTileType(2,5)=='T' && getTileType(5,6)=='T' && getTileType(8,2)=='T', "hard has extra traps");
    check(everythingReachable(), "walls do not cut anything off (hard)");
    setDifficulty(DIFF_EASY); initializeMap();
    check(getTileType(6,0)=='.' && getTileType(8,3)=='.', "easy has fewer traps");
    setDifficulty(DIFF_NORMAL);

    // ---- save and load ----
    resetPlayerToDefault(); resetBattleSystem(); initializeMap();
    walk("UU"); interactWithTile();          // pick up the relic at 2,4
    markEnemyDefeated(2,2); markBossTypeDefeated(ENEMY_MAD_SCIENTIST);
    takeDamage(20); int hpSaved = getPlayerHP();
    check(saveGame("test_save.txt") && saveExists("test_save.txt"), "game saves");
    resetPlayerToDefault(); resetBattleSystem(); initializeMap();
    check(loadGame("test_save.txt"), "game loads");
    check(getPlayerRow()==2 && getPlayerCol()==4 && getRelicsFound()==1 && getTileType(2,4)=='.', "map position, relic and tiles came back");
    check(getPlayerHP()==hpSaved && hasItem(ITEM_RELIC), "hp and bag came back");
    check(isBossTypeDefeated(ENEMY_MAD_SCIENTIST) && isEnemyDefeated(2,2), "beaten boss came back");
    // a broken save must be refused and must not change anything
    { std::ofstream bad("test_save.txt.map"); bad << "2 4 1\nnot a map\n"; }
    initializeMap(); int before = getPlayerRow();
    check(!loadGame("test_save.txt") && getPlayerRow()==before, "broken map save is refused");
    check(!loadGame("does_not_exist.txt"), "missing save is refused");
    std::remove("test_save.txt"); std::remove("test_save.txt.map");

    // an old save file that still has a key tile (K) and the old chest tile (C) must still load, as floor
    resetPlayerToDefault(); resetBattleSystem(); initializeMap();
    saveGame("test_save.txt");
    { std::ifstream in("test_save.txt.map"); std::string all((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>()); in.close();
      size_t a = all.find('\n') + 1;                       // first map row starts after the first line
      all[a + 0] = 'K'; all[a + 11 + 5] = 'C';              // row 0 col 0 and row 1 col 5
      std::ofstream out("test_save.txt.map"); out << all; }
    initializeMap();
    check(loadGame("test_save.txt") && getTileType(0,0)=='.' && getTileType(1,5)=='.', "old save with K and C tiles still loads (they become floor)");
    std::remove("test_save.txt"); std::remove("test_save.txt.map");

    // ---- high score ----
    std::remove("test_scores.txt");
    check(getHighestScore("test_scores.txt")==0, "no file = high score 0");
    saveHighScore(500, "test_scores.txt"); saveHighScore(300, "test_scores.txt");
    check(getHighestScore("test_scores.txt")==500, "lower score does not replace the record");
    saveHighScore(900, "test_scores.txt");
    check(getHighestScore("test_scores.txt")==900, "higher score does");
    std::remove("test_scores.txt");

    // triggered traps/holes must survive a save + load
    resetPlayerToDefault(); resetBattleSystem(); initializeMap();
    walk("L");                                // trap at 4,3
    check(saveGame("test_save.txt"), "save with a triggered trap");
    initializeMap(); resetPlayerToDefault();
    check(loadGame("test_save.txt") && isTileTriggered(4,3) && !isTileTriggered(4,8), "triggered trap is remembered after loading");
    std::remove("test_save.txt"); std::remove("test_save.txt.map");

    // ---- inventory sorting (bubble sort on the Item structs) ----
    resetPlayerToDefault();                   // starts with 2 healing potions only
    addItem(ITEM_RELIC);                      // Viktor starts with no relics, so add one for the test
    addItem(ITEM_POISON_POTION); addItem(ITEM_POISON_POTION); addItem(ITEM_BUFF_POTION); addItem(ITEM_DECOY_KEY);
    sortInventory();
    bool sorted = true; int lastId = -2; std::string lastName = ""; int filled = 0;
    for (int i = 0; i < INVENTORY_SLOTS; i++) {
        int id = getSlotItemId(i);
        if (id == -1) { lastId = -1; continue; }
        if (lastId == -1) sorted = false;               // a filled slot after an empty one
        std::string name = getItemName(id);
        if (name < lastName) sorted = false;
        lastName = name; lastId = id; filled++;
    }
    check(sorted && filled == 5, "bag is in A-Z order and empty slots are at the end");
    check(getItemName(getSlotItemId(0))=="Buff Potion" && getItemName(getSlotItemId(4))=="Relic Bag", "first is Buff Potion, last is Relic Bag");
    check(getItemCount(ITEM_POISON_POTION)==2 && getItemCount(ITEM_HEALING_POTION)==2, "quantities travelled with their items");
    sortInventory();
    check(getItemName(getSlotItemId(0))=="Buff Potion", "sorting twice changes nothing");

    // the bag must survive the binary save too (it used to dump the raw Item struct)
    check(saveGameBinary("test_save.bin") && getItemCount(ITEM_POISON_POTION)==2, "binary save works");
    resetPlayerToDefault();
    check(loadGameBinary("test_save.bin") && getItemCount(ITEM_POISON_POTION)==2 && getItemName(getSlotItemId(0))=="Buff Potion", "binary load gives the same bag back");
    std::remove("test_save.bin");

    std::cout << (fails ? "FAILED\n" : "ALL OK\n");
    return fails;
}
