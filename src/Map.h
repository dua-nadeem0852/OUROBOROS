// Map.h - the map and walking around on it (Amina's part)
// WHAT THIS FILE DOES: lists the map functions other files can call, the tile letters and
// the RESULT_ numbers that say what happened. the code itself is in Map.cpp.
// the player stands on one tile at a time. other files can ask this file
// what is under the player (getTileType) and what happened (the RESULT_ numbers)
#ifndef MAP_H
#define MAP_H

#include <SFML/Graphics.hpp>
#include <string>

// one square of the map. the whole map is a 2D array of these (see Map.cpp)
// (this is the struct from the PF notes: one variable per square instead of 4 separate arrays)
struct Tile
{
    char type;            // what is on it: . floor, W wall, E enemy, B bush, R relic, N npc, ? clue, T trap, H hole
    bool visited;         // the player has walked on it
    bool enemyDefeated;   // the boss that stands on this square has been beaten
    bool revealed;        // the mist is gone (it has been seen)
    bool triggered;       // a trap/hole that already went off. hidden ones look like normal floor until then
};

const int MAP_SIZE  = 10;     // map is 10 x 10 tiles
const int TILE_SIZE = 64;     // every tile is 64 x 64 pixels on the screen
const int MAP_X     = 24;     // where the top left corner of the map is drawn
const int MAP_Y     = 40;

// tile letters used inside the map
//   .  empty ground      B  bush (has an item inside)    T  trap
//   H  hole              K  key (not used any more)       R  relic
//   C  treasure chest (not a tile letter any more: the chest is drawn at CHEST_ROW/CHEST_COL once a key is found)
//   E  enemy (the Scientist and the Bat wait on these. the Hydra is NOT on the map, it comes out of the chest)
//   W  wall              N  npc (talk with E)           ?  clue (read with E)

// what happened when the player stepped on a tile or pressed E
const int RESULT_NOTHING = 0;
const int RESULT_ENEMY = 1;     // a boss is here, game should start the battle
const int RESULT_TRAP = 2;     // player got hurt by a trap
const int RESULT_HOLE  = 3;     // fell in a hole, hurt and sent back to the start
const int RESULT_ITEM = 4;     // picked something up
const int RESULT_RELIC = 5;     // picked up a relic, game should show its story
const int RESULT_FULL = 6;     // wanted to pick something up but the bag is full
const int RESULT_LOCKED = 7;     // chest cannot be opened yet
const int RESULT_CHEST = 8;     // chest opened! ending time
const int RESULT_TALK = 9;     // talked to an npc or read a clue (the text is in getMapMessage
const int RESULT_CHEST_BOSS = 10; // chest dialogue, then Hydra encounter
const int CHEST_ROW = 5;          // where the hidden chest is (it only shows up once the Bat's key is in the bag)
const int CHEST_COL = 8;
bool isChestVisible();            // true once the player holds a key (or the Hydra is dead). before that the chest does not exist

void initializeMap();                          // call when a new game starts
bool checkBoundaries(int row, int col);        // true if the tile is inside the map
bool movePlayer(int rowChange, int colChange); // moves 1 tile, false if it would leave the map
void revealTile(int row, int col);             // shows a tile (and the ones around it)

char getTileType(int row, int col);            // the letter on that tile
const Tile& getTile(int row, int col);         // the whole Tile (a wall-like blank tile if row/col is outside the map)
bool isTileTriggered(int row, int col);        // true for a trap/hole that already went off
bool isRevealed(int row, int col);
int  getPlayerRow();
int  getPlayerCol();
int  getEnemyTypeAt(int row, int col);         // ENEMY_... from Enemy.h, or ENEMY_NONE

int  stepOnTile();                             // call right after the player moved
int  interactWithTile();                       // call when E is pressed

std::string getMapMessage();                   // short line to show the player ("You found a key.")
int  getRelicsFound();                         // how many relics were picked up so far
int  getLastRelicIndex();                      // which relic (0, 1, 2...) was found last

bool saveMapState(const std::string& filename);   // see SaveSystem.cpp, saves tiles, position, beaten bosses
bool loadMapState(const std::string& filename);   // false if the file is missing or looks broken
bool everythingReachable();                       // true if walls don't cut anything off (used by TestMap)

void drawMap(sf::RenderWindow& window, const sf::Font& font);

#endif
