// Map.cpp - It contains the map, the player's position and whatever is on every tile (Amina's part)
// WHAT THIS FILE DOES: It is a 10x10 map, moves the player (walls block you), decides what
// happens on every tile (traps, holes, bushes, relics, keys, npcs, clues, bosses, the chest),
// makes the map a bit harder/easier depending on the difficulty, saves/loads the map state
// and draws the map. 
// the layout is the same one Amina made, only the extra E tiles were taken out
// (there are only 3 bosses so only 3 enemy tiles are needed, see the map below)

#include "Map.h"
#include "UiDraw.h"
#include "Enemy.h"
#include "Battle.h"
#include "PlayerInterface.h"
#include <fstream>

const int START_ROW= 4;     //player begins in the middle
const int START_COL = 4;
const int TRAP_DAMAGE = 10;    //takeDamage() makes it bigger on hard
const int HOLE_DAMAGE = 15;

// the map. each string is one row
static const char* START_MAP[MAP_SIZE] = {
    ".T.B..B...",
    "......T.E.",
    "H.E.R..R..",
    ".R....R...",
    "...TB...TH",
    ".B.....R..",
    "T...H.H...",
    "..T..B....",
    "...TR....R",
    "B.........."
};
// extra things from the proposal that sit on top of Amina's layout:
//   W = wall (cannot walk through)   N = npc (press E to talk)   ? = clue (press E to read)
// they are put on the map in initializeMap() so START_MAP above stays exactly what Amina made.

static const int WALL_COUNT = 8;
static const int WALL_ROW[WALL_COUNT] = { 1, 1, 3, 3, 6, 7, 7, 8 };
static const int WALL_COL[WALL_COUNT] = { 2, 3, 8, 9, 8, 7, 8, 6 };

static const int NPC_COUNT = 2;
static const int NPC_ROW[NPC_COUNT] = { 4, 6 };
static const int NPC_COL[NPC_COUNT] = { 2, 1 };
static const char* NPC_TEXT[NPC_COUNT] = {
    "A lost traveller whispers: \"Not every key is what it looks like. Ask the thing at the very end.\"",
    "A hooded figure says: \"Three guard this place. The last one carries the only key that matters.\""
};

static const int CLUE_COUNT = 3;
static const int CLUE_ROW[CLUE_COUNT] = { 3, 7, 8 };
static const int CLUE_COL[CLUE_COUNT] = { 5, 3, 8 };
static const char* CLUE_TEXT[CLUE_COUNT] = {
    "Scratched into the stone: \"The first key you are given is a lie.\"",
    "A faded note: \"Holes send you home. Watch where you step in the south.\"",
    "Carved words: \"A chest waits in the far east. It only shows itself to someone carrying a key.\""""
};

// difficulty changes the traps: hard gets extra ones, easy loses a couple
// (row, col pairs, all on empty ground)
static const int HARD_TRAP_COUNT = 3;
static const int HARD_TRAP_ROW[HARD_TRAP_COUNT] = { 2, 5, 8 };
static const int HARD_TRAP_COL[HARD_TRAP_COUNT] = { 5, 6, 2 };
static const int EASY_REMOVED_COUNT = 2;
static const int EASY_REMOVED_ROW[EASY_REMOVED_COUNT] = { 6, 8 };
static const int EASY_REMOVED_COL[EASY_REMOVED_COUNT] = { 0, 3 };

// every relic on the map and which story/picture it has (the numbers are the story numbers in Story.cpp,
// and the picture is assets/images/relic_<number>.png). the stories are placed near what they talk about:
//   0 locket (next to the start)  1 scientist's notebook page (beside the scientist)  2 child's drawing of the hydra (beside the chest)
//   3 bat brooch (beside the bat)  4 water-stained ledger  5 empty key mould  6 warm stone (far corner, the last one)
static const int RELIC_TILE_COUNT = 7;
static const int RELIC_TILE_ROW[RELIC_TILE_COUNT]   = { 2, 2, 3, 3, 5, 8, 8 };
static const int RELIC_TILE_COL[RELIC_TILE_COUNT]   = { 4, 7, 1, 6, 7, 4, 9 };
static const int RELIC_TILE_STORY[RELIC_TILE_COUNT] = { 0, 3, 1, 4, 2, 5, 6 };

// the three bosses and where they stand
// (row, col, which boss). scientist is the closest one to the start, hydra the furthest
static const int BOSS_COUNT = 3;
// the hydra's "position" is the chest, because that is where you fight it
static const int BOSS_ROW[BOSS_COUNT]  = { 2, 1, CHEST_ROW };
static const int BOSS_COL[BOSS_COUNT]  = { 2, 8, CHEST_COL };
static const int BOSS_TYPE[BOSS_COUNT] = { ENEMY_MAD_SCIENTIST, ENEMY_HUMANOID_BAT, ENEMY_HYDRA };

// what is hiding in each bush (same order the bushes show up when reading the map top to bottom)
static const int BUSH_COUNT = 6;
static const int BUSH_ROW[BUSH_COUNT]  = { 0, 0, 4, 5, 7, 9 };
static const int BUSH_COL[BUSH_COUNT]  = { 3, 6, 4, 1, 5, 0 };
static const int BUSH_ITEM[BUSH_COUNT] = { ITEM_HEALING_POTION, ITEM_HEALING_POTION, ITEM_HEALING_POTION,
                                           ITEM_BUFF_POTION, ITEM_HEALING_POTION, ITEM_BUFF_POTION };

// these change while the game is played
// (Tile struct instead of the separate tiles / revealed / visited arrays we had before)
static Tile gameMap[MAP_SIZE][MAP_SIZE];
static int  playerRow = START_ROW;
static int  playerCol = START_COL;
static int  relicsFound = 0;
static int  lastRelicStory = -1;     // which story the last relic picked up has
static std::string message = "";

void initializeMap()
{
    for (int r = 0; r < MAP_SIZE; r++)
    {
        for (int c = 0; c < MAP_SIZE; c++)
        {
            gameMap[r][c].type = START_MAP[r][c];
            gameMap[r][c].revealed = false;
            gameMap[r][c].visited = false;
            gameMap[r][c].enemyDefeated = false;
            gameMap[r][c].triggered = false;
        }
    }

    // walls, npcs and clues
    for (int i = 0; i < WALL_COUNT; i++) gameMap[WALL_ROW[i]][WALL_COL[i]].type = 'W';
    for (int i = 0; i < NPC_COUNT; i++)  gameMap[NPC_ROW[i]][NPC_COL[i]].type = 'N';
    for (int i = 0; i < CLUE_COUNT; i++) gameMap[CLUE_ROW[i]][CLUE_COL[i]].type = '?';

    // difficulty. (startNewGame sets the difficulty BEFORE it calls this)
    if (getDifficulty() == DIFF_HARD)
        for (int i = 0; i < HARD_TRAP_COUNT; i++) gameMap[HARD_TRAP_ROW[i]][HARD_TRAP_COL[i]].type = 'T';
    if (getDifficulty() == DIFF_EASY)
        for (int i = 0; i < EASY_REMOVED_COUNT; i++) gameMap[EASY_REMOVED_ROW[i]][EASY_REMOVED_COL[i]].type = '.';
    playerRow = START_ROW;
    playerCol = START_COL;
    relicsFound = 0;
    lastRelicStory = -1;
    message = "";
    revealTile(playerRow, playerCol);
    gameMap[playerRow][playerCol].visited = true;
}

bool checkBoundaries(int row, int col)
{
    return row >= 0 && row < MAP_SIZE && col >= 0 && col < MAP_SIZE;
}

// shows the tile and the 8 tiles around it, so you can see a little bit ahead
void revealTile(int row, int col)
{
    for (int r = row - 1; r <= row + 1; r++)
        for (int c = col - 1; c <= col + 1; c++)
            if (checkBoundaries(r, c))
                gameMap[r][c].revealed = true;
}

bool movePlayer(int rowChange, int colChange)
{
    int newRow = playerRow + rowChange;
    int newCol = playerCol + colChange;

    if (!checkBoundaries(newRow, newCol))
    {
        message = "You can't go any further that way.";
        return false;
    }

    // bug fix: there was nothing that could block the player before. walls do now
    if (gameMap[newRow][newCol].type == 'W')
    {
        message = "A wall blocks the way.";
        return false;
    }

    playerRow = newRow;
    playerCol = newCol;
    gameMap[playerRow][playerCol].visited = true;
    revealTile(playerRow, playerCol);
    message = "";
    return true;
}

char getTileType(int row, int col)
{
    if (!checkBoundaries(row, col)) return '#';
    return gameMap[row][col].type;
}

// outside the map we hand back a blank wall tile so callers never read out of the array
const Tile& getTile(int row, int col)
{
    static const Tile outside = { '#', false, false, true, false };
    if (!checkBoundaries(row, col)) return outside;
    return gameMap[row][col];
}

bool isTileTriggered(int row, int col) { return checkBoundaries(row, col) && gameMap[row][col].triggered; }

bool isRevealed(int row, int col) { return checkBoundaries(row, col) && gameMap[row][col].revealed; }
int  getPlayerRow() { return playerRow; }
int  getPlayerCol() { return playerCol; }
std::string getMapMessage() { return message; }
int  getRelicsFound() { return relicsFound; }
// which story belongs to the relic that was picked up last (so Game.cpp shows the right text)
int  getLastRelicIndex() { return lastRelicStory; }

// story number of the relic lying on this tile, or -1 if it is not one of the listed relic tiles
static int findRelicStory(int row, int col)
{
    for (int i = 0; i < RELIC_TILE_COUNT; i++)
        if (RELIC_TILE_ROW[i] == row && RELIC_TILE_COL[i] == col)
            return RELIC_TILE_STORY[i];
    return -1;
}

int getEnemyTypeAt(int row, int col)
{
    for (int i = 0; i < BOSS_COUNT; i++)
        if (BOSS_ROW[i] == row && BOSS_COL[i] == col)
            return BOSS_TYPE[i];
    return ENEMY_NONE;
}

// which bush is this? returns -1 if it isn't one of the bushes in the list
static int findBush(int row, int col)
{
    for (int i = 0; i < BUSH_COUNT; i++)
        if (BUSH_ROW[i] == row && BUSH_COL[i] == col)
            return i;
    return -1;
}

// hurts the player and returns how much hp was really lost
// (takeDamage changes the number depending on the difficulty)
static int hurtPlayer(int amount)
{
    int before = getPlayerHP();
    takeDamage(amount);
    return before - getPlayerHP();
}

// the chest does not exist until the Bat has given us a key (that is the decoy key).
// once the Hydra is dead it stays, even if the key is somehow gone
bool isChestVisible()
{
    return hasItem(ITEM_DECOY_KEY) || hasItem(ITEM_REAL_KEY) || isBossTypeDefeated(ENEMY_HYDRA);
}

// copies "is this boss dead" from Enemy.cpp into the Tile struct, so the map can just look at its own tiles
static void syncEnemyFlags()
{
    for (int i = 0; i < BOSS_COUNT; i++)
        gameMap[BOSS_ROW[i]][BOSS_COL[i]].enemyDefeated = isEnemyDefeated(BOSS_ROW[i], BOSS_COL[i]);
}

// stuff that happens by itself as soon as you walk onto a tile
int stepOnTile()
{
    syncEnemyFlags();
    char tile = gameMap[playerRow][playerCol].type;

    if (tile == 'E')
    {
        int type = getEnemyTypeAt(playerRow, playerCol);
        if (type == ENEMY_NONE || gameMap[playerRow][playerCol].enemyDefeated)
            return RESULT_NOTHING;          // already beaten, nothing left here
        message = "Something is waiting for you...";
        return RESULT_ENEMY;
    }

    // traps and holes are hidden: they look like plain floor until they go off.
    // after that the tile is marked as triggered and shows its picture for the rest of the game
    if (tile == 'T')
    {
        if (gameMap[playerRow][playerCol].triggered)
        {
            message = "The trap here has already gone off.";    // it goes off only once
            return RESULT_NOTHING;
        }
        gameMap[playerRow][playerCol].triggered = true;
        int lost = hurtPlayer(TRAP_DAMAGE);
        message = "A trap snaps shut! You lose " + std::to_string(lost) + " HP.";
        return RESULT_TRAP;
    }

    if (tile == 'H')
    {
        gameMap[playerRow][playerCol].triggered = true;          // now everyone can see the hole
        int lost = hurtPlayer(HOLE_DAMAGE);
        message = "You fall into a hole and lose " + std::to_string(lost) + " HP. You crawl back to the start.";
        playerRow = START_ROW;
        playerCol = START_COL;
        revealTile(playerRow, playerCol);
        return RESULT_HOLE;
    }

    if (playerRow == CHEST_ROW && playerCol == CHEST_COL && isChestVisible())
        message = "A distant chest. Press E to inspect it.";
    else if (tile == 'R') message = "A strange relic. Press E to pick it up.";
    else if (tile == 'B') message = "A thick bush. Press E to search it.";
    else if (tile == 'N') message = "Someone is standing here. Press E to talk.";
    else if (tile == '?') message = "There is writing here. Press E to read it.";
    return RESULT_NOTHING;
}

// what happens when E is pressed on the tile the player is standing on
int interactWithTile()
{
    char tile = gameMap[playerRow][playerCol].type;

    // The chest is hidden until the Bat's Decoy Key has been obtained (isChestVisible).
    // pressing E on it never starts the fight by itself, Game.cpp shows the story first
    if (playerRow == CHEST_ROW && playerCol == CHEST_COL && isChestVisible())
    {
        if (!hasItem(ITEM_REAL_KEY) && !isBossTypeDefeated(ENEMY_HYDRA))
        {
            message = "The lock is rusted shut. Something stirs beneath the earth.";
            return RESULT_CHEST_BOSS; // Game.cpp displays the story before starting Hydra.
        }
        if (!hasItem(ITEM_REAL_KEY))
        {
            message = "The chest remains locked. The real key is still missing.";
            return RESULT_LOCKED;
        }
        setProgressFlag(PROGRESS_BEAT_GAME);
        message = "The key turns. The chest opens.";
        return RESULT_CHEST;
    }

    if (tile == 'R')
    {
        if (!addItem(ITEM_RELIC)) { message = "Your pack is full."; return RESULT_FULL; }
        gameMap[playerRow][playerCol].type = '.';
        relicsFound++;
        lastRelicStory = findRelicStory(playerRow, playerCol);   // the story belongs to this relic, not to the pickup order
        message = "You pick up the relic.";
        return RESULT_RELIC;
    }

    if (tile == 'B')
    {
        int bush = findBush(playerRow, playerCol);
        if (bush == -1)
        {
            gameMap[playerRow][playerCol].type = '.';
            message = "You search the bush. Nothing.";
            return RESULT_NOTHING;
        }
        if (!addItem(BUSH_ITEM[bush])) { message = "Your pack is full."; return RESULT_FULL; }
        gameMap[playerRow][playerCol].type = '.';
        message = "You search the bush and find a " + getItemName(BUSH_ITEM[bush]) + ".";
        return RESULT_ITEM;
    }


    if (tile == 'N')
    {
        // the npc says something different depending on which one it is
        for (int i = 0; i < NPC_COUNT; i++)
            if (NPC_ROW[i] == playerRow && NPC_COL[i] == playerCol) message = NPC_TEXT[i];
        return RESULT_TALK;
    }

    if (tile == '?')
    {
        for (int i = 0; i < CLUE_COUNT; i++)
            if (CLUE_ROW[i] == playerRow && CLUE_COL[i] == playerCol) message = CLUE_TEXT[i];
        return RESULT_TALK;
    }

    message = "Nothing to interact with here.";
    return RESULT_NOTHING;
}

// ---------- drawing ----------

// a coloured square with a letter in the middle, used when a tile has no picture
static void drawLetterTile(sf::RenderWindow& window, const sf::Font& font, float x, float y,
                           const char* letter, sf::Color fill)
{
    sf::RectangleShape box(sf::Vector2f(TILE_SIZE - 6.f, TILE_SIZE - 6.f));
    box.setPosition(x + 3.f, y + 3.f);
    box.setFillColor(fill);
    window.draw(box);
    drawCenteredText(window, font, letter, 28, x + TILE_SIZE / 2.f, y + 14.f, sf::Color(240, 235, 220));
}

// how much of the tile these pictures fill (1.0 = whole tile). change the numbers to make them bigger or smaller
static const float RELIC_SCALE = 0.65f;
static const float CHEST_SCALE = 0.75f;

// turns "fill this part of the tile" into the pad value drawTilePicture wants (the picture stays centred and square)
static float padFor(float scale) { return TILE_SIZE * (1.f - scale) / 2.f; }

// draws a picture so it fills the tile (if the image file is missing it just returns false)
// pad = how many pixels to leave free around the picture. floors and walls use 0 so they touch each other
static bool drawTilePicture(sf::RenderWindow& window, const char* path, float x, float y, float pad = 2.f)
{
    sf::Texture* tex = loadTexture(path);
    if (!tex) return false;

    sf::Sprite sprite(*tex);
    sprite.setPosition(x + pad, y + pad);
    sprite.setScale((TILE_SIZE - 2.f * pad) / tex->getSize().x, (TILE_SIZE - 2.f * pad) / tex->getSize().y);
    window.draw(sprite);
    return true;
}

// draws the picture, or a coloured letter tile if the picture file is missing
static void drawTileOrLetter(sf::RenderWindow& window, const sf::Font& font, const char* path,
                             float x, float y, const char* letter, sf::Color color, float pad = 2.f)
{
    if (!drawTilePicture(window, path, x, y, pad))
        drawLetterTile(window, font, x, y, letter, color);
}

// the ground under one tile. uses assets/images/floor.png when it exists, otherwise plain dark squares
static void drawFloor(sf::RenderWindow& window, const Tile& tile, float x, float y)
{
    sf::RectangleShape floor(sf::Vector2f((float)TILE_SIZE, (float)TILE_SIZE));
    floor.setPosition(x, y);

    if (!tile.revealed)
    {
        floor.setFillColor(sf::Color(12, 10, 16));          // hidden = almost black
        floor.setOutlineColor(sf::Color(8, 6, 10));
        floor.setOutlineThickness(1.f);
        window.draw(floor);
        return;
    }

    if (drawTilePicture(window, "assets/images/floor.png", x, y, 0.f))
    {
        // places you have walked on are a little lighter, places you only saw are a little darker
        floor.setFillColor(tile.visited ? sf::Color(255, 255, 255, 22) : sf::Color(0, 0, 0, 70));
        window.draw(floor);
        return;
    }

    floor.setFillColor(tile.visited ? sf::Color(58, 52, 66) : sf::Color(38, 34, 46));
    floor.setOutlineColor(sf::Color(8, 6, 10));
    floor.setOutlineThickness(1.f);
    window.draw(floor);
}

// every relic has its own picture (assets/images/relic_0.png ... relic_6.png) that matches its story.
// if that file is missing it tries the shared assets/images/relic.png, and false means no picture at all
static bool drawRelic(sf::RenderWindow& window, int row, int col, float x, float y)
{
    int story = findRelicStory(row, col);
    if (story >= 0)
    {
        std::string own = "assets/images/relic_" + std::to_string(story) + ".png";
        if (drawTilePicture(window, own.c_str(), x, y, padFor(RELIC_SCALE))) return true;
    }
    return drawTilePicture(window, "assets/images/relic.png", x, y, padFor(RELIC_SCALE));
}

void drawMap(sf::RenderWindow& window, const sf::Font& font)
{
    syncEnemyFlags();

    for (int r = 0; r < MAP_SIZE; r++)
    {
        for (int c = 0; c < MAP_SIZE; c++)
        {
            float x = (float)(MAP_X + c * TILE_SIZE);
            float y = (float)(MAP_Y + r * TILE_SIZE);
            const Tile& tile = gameMap[r][c];

            drawFloor(window, tile, x, y);

            if (!tile.revealed)
            {
                drawTilePicture(window, "assets/images/mist.png", x, y, 0.f);   // mist on top if we have the picture
                continue;
            }

            // Traps and holes stay hidden (nothing is drawn, so they look like floor) until the
            // player has triggered them. then the picture stays on the map.
            char t = tile.type;
            if      (t == 'B')                   drawTileOrLetter(window, font, "assets/images/bush.png", x, y, "B", sf::Color(40, 90, 50));
            else if (t == 'T' && tile.triggered) drawTileOrLetter(window, font, "assets/images/trap.png", x, y, "T", sf::Color(120, 60, 40));
            else if (t == 'H' && tile.triggered) drawTileOrLetter(window, font, "assets/images/hole.png", x, y, "H", sf::Color(0, 0, 0));
            else if (t == 'W')                   drawTileOrLetter(window, font, "assets/images/wall.png", x, y, "#", sf::Color(90, 90, 100), 0.f);
            else if (t == 'N')                   drawTileOrLetter(window, font, "assets/images/npc.png", x, y, "N", sf::Color(60, 100, 140));
            else if (t == '?')                   drawTileOrLetter(window, font, "assets/images/clue.png", x, y, "?", sf::Color(120, 110, 60));
            else if (t == 'R' && !drawRelic(window, r, c, x, y))
                drawLetterTile(window, font, x, y, "R", sf::Color(90, 60, 130));
            else if (t == 'E' && getEnemyTypeAt(r, c) != ENEMY_NONE && !tile.enemyDefeated)
                drawLetterTile(window, font, x, y, "!", sf::Color(150, 40, 40));      // we don't say which boss it is
        }
    }

    // the chest only exists once a key has been found, and only if the player has seen that square
    if (isChestVisible() && gameMap[CHEST_ROW][CHEST_COL].revealed)
    {
        float cx = (float)(MAP_X + CHEST_COL * TILE_SIZE);
        float cy = (float)(MAP_Y + CHEST_ROW * TILE_SIZE);
        drawTileOrLetter(window, font, "assets/images/treasurechest.png", cx, cy, "C", sf::Color(140, 100, 30), padFor(CHEST_SCALE));
    }

    // the player on top of everything
    float px = (float)(MAP_X + playerCol * TILE_SIZE);
    float py = (float)(MAP_Y + playerRow * TILE_SIZE);
    if (!drawTilePicture(window, "assets/images/character.png", px, py))
    {
        sf::CircleShape dot(TILE_SIZE / 2.f - 10.f);
        dot.setPosition(px + 10.f, py + 10.f);
        dot.setFillColor(sf::Color(220, 200, 120));
        window.draw(dot);
    }
}

// ---------- saving and loading the map (used by SaveSystem.cpp) ----------

// writes everything about the map that can change while playing
// (the player's hp/bag are saved by Player.cpp, the beaten bosses are saved here too)
bool saveMapState(const std::string& filename)
{
    std::ofstream out(filename.c_str());
    if (!out.is_open()) return false;

    out << playerRow << " " << playerCol << " " << relicsFound << "\n";
    for (int r = 0; r < MAP_SIZE; r++)
    {
        for (int c = 0; c < MAP_SIZE; c++) out << gameMap[r][c].type;
        out << "\n";
    }
    for (int r = 0; r < MAP_SIZE; r++)
    {
        for (int c = 0; c < MAP_SIZE; c++) out << (gameMap[r][c].revealed ? '1' : '0');
        out << "\n";
    }
    for (int r = 0; r < MAP_SIZE; r++)
    {
        for (int c = 0; c < MAP_SIZE; c++) out << (gameMap[r][c].visited ? '1' : '0');
        out << "\n";
    }
    for (int i = 0; i < BOSS_COUNT; i++)
        out << (isBossTypeDefeated(BOSS_TYPE[i]) ? 1 : 0) << " ";
    out << "\n";
    // which traps/holes already went off (added later, old save files do not have this part)
    for (int r = 0; r < MAP_SIZE; r++)
    {
        for (int c = 0; c < MAP_SIZE; c++) out << (gameMap[r][c].triggered ? '1' : '0');
        out << "\n";
    }
    return out.good();
}

// reads it back. everything is read into temporary variables first and checked,
// so a broken/edited save file can never leave the map half loaded
bool loadMapState(const std::string& filename)
{
    std::ifstream in(filename.c_str());
    if (!in.is_open()) return false;

    int r0, c0, relics;
    if (!(in >> r0 >> c0 >> relics)) return false;
    if (!checkBoundaries(r0, c0) || relics < 0 || relics > 99) return false;

    char t[MAP_SIZE][MAP_SIZE];
    bool rev[MAP_SIZE][MAP_SIZE], vis[MAP_SIZE][MAP_SIZE], trig[MAP_SIZE][MAP_SIZE];
    for (int r = 0; r < MAP_SIZE; r++) for (int c = 0; c < MAP_SIZE; c++) trig[r][c] = false;
    std::string line;

    for (int r = 0; r < MAP_SIZE; r++)
    {
        if (!(in >> line) || (int)line.size() != MAP_SIZE) return false;
        for (int c = 0; c < MAP_SIZE; c++)
        {
            if (std::string(".BTHRCENW?KC").find(line[c]) == std::string::npos) return false;
            t[r][c] = line[c];
        }
    }
    for (int pass = 0; pass < 2; pass++)
    {
        for (int r = 0; r < MAP_SIZE; r++)
        {
            if (!(in >> line) || (int)line.size() != MAP_SIZE) return false;
            for (int c = 0; c < MAP_SIZE; c++)
            {
                if (line[c] != '0' && line[c] != '1') return false;
                if (pass == 0) rev[r][c] = (line[c] == '1');
                else           vis[r][c] = (line[c] == '1');
            }
        }
    }
    int beaten[BOSS_COUNT];
    for (int i = 0; i < BOSS_COUNT; i++)
        if (!(in >> beaten[i]) || (beaten[i] != 0 && beaten[i] != 1)) return false;
    // the triggered grid is optional (older saves stop after the bosses). if it is there it must be valid
    for (int r = 0; r < MAP_SIZE && in >> line; r++)
    {
        if ((int)line.size() != MAP_SIZE) return false;
        for (int c = 0; c < MAP_SIZE; c++)
        {
            if (line[c] != '0' && line[c] != '1') return false;
            trig[r][c] = (line[c] == '1');
        }
    }
    // older saves can still have a key tile (K) or a chest tile (C). neither is a map tile any more
    // (keys come from the bosses, the chest is drawn at CHEST_ROW/CHEST_COL), so they just become floor
    for (int r = 0; r < MAP_SIZE; r++)
        for (int c = 0; c < MAP_SIZE; c++)
            if (t[r][c] == 'K' || t[r][c] == 'C') t[r][c] = '.';
    // older saves also had the hydra as an enemy tile at 9,6. it is in the chest now, so that tile is just floor
    for (int r = 0; r < MAP_SIZE; r++)
        for (int c = 0; c < MAP_SIZE; c++)
            if (t[r][c] == 'E' && getEnemyTypeAt(r, c) == ENEMY_NONE) t[r][c] = '.';
    if (t[r0][c0] == 'W') return false;      // standing inside a wall would trap the player

    // all good, now copy it over
    for (int r = 0; r < MAP_SIZE; r++)
        for (int c = 0; c < MAP_SIZE; c++) { gameMap[r][c].type = t[r][c]; gameMap[r][c].revealed = rev[r][c]; gameMap[r][c].visited = vis[r][c]; gameMap[r][c].triggered = trig[r][c]; gameMap[r][c].enemyDefeated = false; }
    playerRow = r0;
    playerCol = c0;
    relicsFound = relics;
    message = "";
    resetDefeatedEnemies();
    for (int i = 0; i < BOSS_COUNT; i++)
        if (beaten[i] == 1) { markBossTypeDefeated(BOSS_TYPE[i]); markEnemyDefeated(BOSS_ROW[i], BOSS_COL[i]); }
    syncEnemyFlags();
    return true;
}

// true if every tile you need (relics, key, bushes, bosses, chest) can be reached from the start
// without going through walls or holes, so the walls can never lock the game
bool everythingReachable()
{
    bool seen[MAP_SIZE][MAP_SIZE];
    for (int r = 0; r < MAP_SIZE; r++) for (int c = 0; c < MAP_SIZE; c++) seen[r][c] = false;

    int queueR[MAP_SIZE * MAP_SIZE], queueC[MAP_SIZE * MAP_SIZE];
    int head = 0, tail = 0;
    queueR[tail] = START_ROW; queueC[tail++] = START_COL; seen[START_ROW][START_COL] = true;

    int dr[4] = { -1, 1, 0, 0 }, dc[4] = { 0, 0, -1, 1 };
    while (head < tail)
    {
        int r = queueR[head], c = queueC[head++];
        for (int d = 0; d < 4; d++)
        {
            int nr = r + dr[d], nc = c + dc[d];
            if (!checkBoundaries(nr, nc) || seen[nr][nc]) continue;
            if (gameMap[nr][nc].type == 'W' || gameMap[nr][nc].type == 'H') continue;
            seen[nr][nc] = true;
            queueR[tail] = nr; queueC[tail++] = nc;
        }
    }

    for (int r = 0; r < MAP_SIZE; r++)
        for (int c = 0; c < MAP_SIZE; c++)
            if (std::string("BRNE?").find(gameMap[r][c].type) != std::string::npos && !seen[r][c])
                return false;
    return seen[CHEST_ROW][CHEST_COL];
}
