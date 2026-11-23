//Game.cpp for main loop and game states
//WHAT THIS FILE DOES: opens the window (resizable, fullscreen with F11, scales to any screen),
// runs the main loop, switches between the game states, handles the keys, keeps the score and
//connects the save system. the actual rules live in the other files (Map, Battle, Player etc)
//the loop does 3 things every frame: read the keys, draw whatever state we are in, show it
//the actual rules live in the other files as mentioned before, this file only connects them
#include "Game.h"
#include "Map.h"
#include "Screens.h"
#include "Story.h"
#include "Battle.h"
#include "BattleScreen.h"
#include "BattleSound.h"
#include "Enemy.h"
#include "PlayerInterface.h"
#include "UiDraw.h"
#include "SaveSystem.h"
#include <SFML/Graphics.hpp>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

//things the game has to remember. these variables will stay in the memory until the game closes
static int state = STATE_MENU;

static const int MENU_ITEMS = 6;           //0 start, 1 continue, 2 difficulty, 3 screen, 4 credits, 5 exit
static int menuIndex = 0;
static bool fullscreen = false;
static std::string pauseNotice = "";       //"Game saved." shown in the pause menu
static int levelBeforeBattle = 1;         //to say "level up!" after a fight
static int difficultyChoice = DIFF_NORMAL;
static bool hardUnlocked = false;          //hard is locked until the game is beaten once on normal

static bool paused = false;                //pause menu is on top of exploring
static int bagIndex = 0;                  //selected slot in the bag
static std::string bagNotice = "";

static std::vector<std::string> sceneLines;   //text shown in a story box
static int sceneIndex = 0;
static std::string sceneTitle = "";
static int sceneNextState = STATE_EXPLORE;    //where to go when the text is done

static std::string battleNote = "";        //"You defeated ..." shown after a fight
static float creditsScroll = 0.f;
static bool startHydraAfterScene = false;

//the small save file for the hard mode unlock
static void loadUnlocked()
{
    std::ifstream in("unlocked.txt");
    int value = 0;
    if (in >> value) hardUnlocked = (value == 1);
}

static void saveUnlocked()
{
    std::ofstream out("unlocked.txt");
    out << (hardUnlocked ? 1 : 0);
}

//finishing the game gives a big bonus and the harder the difficulty the bigger the total
static int computeScore(bool wonGame)
{
    int score = getRelicsFound() * 100;
    if (isBossTypeDefeated(ENEMY_MAD_SCIENTIST)) score += 300;
    if (isBossTypeDefeated(ENEMY_HUMANOID_BAT))  score += 300;
    if (isBossTypeDefeated(ENEMY_HYDRA)) score += 500;
    score += getPlayerHP() * 2;
    if (wonGame) score += 1000;

    int percent = 150;                                   //normal
    if (getDifficulty() == DIFF_EASY) percent = 100;
    if (getDifficulty() == DIFF_HARD) percent = 200;
    return score * percent / 100;
}

//changing state
//shows some text. when the player has read it all we go to nextState
static void startScene(const std::string& title, const std::vector<std::string>& lines, int nextState)
{
    sceneTitle = title;
    sceneLines = lines;
    sceneIndex = 0;
    sceneNextState = nextState;
    state = STATE_DIALOGUE;
}

static void goToMenu()
{
    stopBattleMusic();
    paused = false;
    pauseNotice = "";
    startHydraAfterScene = false;
    state = STATE_MENU;
}

static void goGameOver()
{
    saveHighScore(computeScore(false));     //dying still counts for the high score list
    stopBattleMusic();
    paused = false;
    state = STATE_GAME_OVER;
}

static void startNewGame()
{
    resetPlayerToDefault();            //also puts difficulty back to normal...
    setPlayerName("Viktor");
    setDifficulty(difficultyChoice);   //so we set the chosen one after
    resetBattleSystem();               //forgets the defeated bosses too
    initializeMap();
    stopBattleMusic();

    battleNote = "";
    bagIndex = 0;
    paused = false;
    startHydraAfterScene = false;      //forget a half finished chest scene from the old game

    std::vector<std::string> intro;
    for (int i = 0; i < getIntroLineCountStory(); i++) {intro.push_back(getIntroLineStory(i));}
    startScene("", intro, STATE_EXPLORE);
}

//loads save.txt (player + map) and goes straight back to exploring
static void continueGame()
{
    if (!saveExists()) return;                //nothing saved, the menu line is greyed out
    resetBattleSystem();
    if (!loadGame())
    {
        //the save was broken. loadGame leaves the game untouched in that case. so just stay in the menu
        std::cout << "Could not load the save: " << getLastFileError() << "\n";
        return;
    }
    stopBattleMusic();
    battleNote = "Game loaded.";
    bagIndex = 0;
    paused = false;
    startHydraAfterScene = false;
    pauseNotice = "";
    state = STATE_EXPLORE;
}

static void startEnding()
{
    //beating the game on normal opens up hard
    if (getDifficulty() == DIFF_NORMAL && !hardUnlocked)
    {
        hardUnlocked = true;
        saveUnlocked();
    }

    std::vector<std::string> lines;
    for (int i = 0; i < getEndingLineCount(); i++)
        lines.push_back(getEndingLine(i));

    //the more relics you found the more of the story you get
    if (getRelicsFound() == RELIC_COUNT)
        lines.insert(lines.begin() + 3, "You found every relic. Nothing is left to forget.");
    else
        lines.insert(lines.begin() + 3, "Some relics were never found, and part of the story stays dark.");

    int score = computeScore(true);
    bool record = score > getHighestScore();
    saveHighScore(score);
    lines.push_back("Final score: " + std::to_string(score) + (record ? "   (new high score!)" : ""));

    creditsScroll = 0.f;
    startScene("", lines, STATE_CREDITS);
}

static void startBossBattle()
{
    int row = getPlayerRow();
    int col = getPlayerCol();
    int type = getEnemyTypeAt(row, col);

    //beginBattle says no if that boss is already dead
    levelBeforeBattle = getPlayerLevel();
    if (beginBattle(type, row, col))
    {
        battleNote = "";
        state = STATE_BATTLE;
    }
}

//the battle screen already called endBattle() for the game, so  only look at how it went
static void afterBattle()
{
    if (getBattleResult() == BATTLE_LOST || isPlayerDead())
    {
        goGameOver();
        return;
    }

    battleNote = "You defeated " + getEnemyName() + ".";
    if (getPlayerLevel() > levelBeforeBattle)
        battleNote += " Level up! You are now level " + std::to_string(getPlayerLevel()) + ".";
    if (hasPendingReward())
        battleNote += " Your bag is full. Use something from the bag (I) to make room for the reward.";
    state = STATE_EXPLORE;
}

//what to do after a step or an E press on the map
static void handleMapResult(int result)
{
    if (isPlayerDead())              //a trap or a hole can deal damage essentially/take away health
    {
        goGameOver();
        return;
    }

    if (result == RESULT_ENEMY)
    {
        startBossBattle();
    }
    else if (result == RESULT_RELIC)
    {
        int which = getLastRelicIndex();
        std::vector<std::string> text;
        text.push_back(getRelicStory(which));
        startScene(getRelicTitle(which), text, STATE_EXPLORE);
    }
    else if (result == RESULT_CHEST_BOSS)
    {
        startHydraAfterScene = true;
        startScene("The Chest", { "The lock is rusted shut. Something stirs beneath the earth.",
                                  "A terrible presence rises from below..." }, STATE_EXPLORE);
    }
    else if (result == RESULT_CHEST)
    {
        startEnding();
    }
}

//keys,one function for each state
//(re)opens the window.
//windowed = a resizable window that fits the desktop, fullscreen = the whole screen
static void createWindow(sf::RenderWindow& window)
{
    sf::VideoMode desktop = sf::VideoMode::getDesktopMode();
    if (fullscreen)
    {
        window.create(desktop, "OUROBOROS", sf::Style::Fullscreen);
    }
    else
    {
        //i set it to 16:9 because most like 90% of the desktop so it also fits small laptop screens
        float w = (float)SCREEN_W;
        float h = (float)SCREEN_H;
        float scale = 1.f;
        if (w > desktop.width * 0.9f)   scale = desktop.width * 0.9f / w;
        if (h * scale > desktop.height * 0.9f) scale = desktop.height * 0.9f / h;
        window.create(sf::VideoMode((unsigned)(w * scale), (unsigned)(h * scale)), "OUROBOROS", sf::Style::Default);
    }
    window.setFramerateLimit(60);
    window.setKeyRepeatEnabled(false);       //holding a key shouldn't skip through text

    //icon in the window corner and the taskbar. (the icon of the .exe file itself comes from src/ouroboros.rc)
    sf::Image icon;
    if (icon.loadFromFile("assets/icon.png"))
        window.setIcon(icon.getSize().x, icon.getSize().y, icon.getPixelsPtr());
}

//everything except the battle is drawn for 1280 x 720. this scales it to the window and adds
//black bars if the window has another shape, so it looks right on any screen
static void setLetterboxView(sf::RenderWindow& window)
{
    sf::Vector2u size = window.getSize();
    if (size.x == 0 || size.y == 0) return;
    float windowAspect = (float)size.x / (float)size.y;
    float wanted = (float)SCREEN_W / (float)SCREEN_H;

    float x = 0.f, y = 0.f, w = 1.f, h = 1.f;
    if (windowAspect > wanted) { w = wanted / windowAspect; x = (1.f - w) / 2.f; }   //too wide: bars left/right
    else if (windowAspect < wanted) { h = windowAspect / wanted; y = (1.f - h) / 2.f; }   //too tall: bars top/bottom

    sf::View view(sf::FloatRect(0.f, 0.f, (float)SCREEN_W, (float)SCREEN_H));
    view.setViewport(sf::FloatRect(x, y, w, h));
    window.setView(view);
}

static void menuKey(sf::Keyboard::Key key, sf::RenderWindow& window)
{
    //menu sounds: a tick when the cursor or a setting moves, a "select" sound when something is chosen
    if (key == sf::Keyboard::Up || key == sf::Keyboard::Down || key == sf::Keyboard::Left || key == sf::Keyboard::Right)
        playUiSound(UI_SOUND_MOVE);
    if (key == sf::Keyboard::Enter || key == sf::Keyboard::Space)
        playUiSound((menuIndex == 1 && !saveExists()) ? UI_SOUND_BACK : UI_SOUND_SELECT);   //Continue with no save = "not allowed"
    if (key == sf::Keyboard::Up) menuIndex = (menuIndex + MENU_ITEMS - 1) % MENU_ITEMS;
    if (key == sf::Keyboard::Down) menuIndex = (menuIndex + 1) % MENU_ITEMS;

    //left and right change the difficulty (hard only if it is unlocked) or the screen mode
    int hardest = hardUnlocked ? DIFF_HARD : DIFF_NORMAL;
    if (key == sf::Keyboard::Left  && menuIndex == 2 && difficultyChoice > DIFF_EASY) difficultyChoice--;
    if (key == sf::Keyboard::Right && menuIndex == 2 && difficultyChoice < hardest)   difficultyChoice++;
    if ((key == sf::Keyboard::Left || key == sf::Keyboard::Right) && menuIndex == 3)
    {
        fullscreen = !fullscreen;
        createWindow(window);
    }

    if (key == sf::Keyboard::Enter || key == sf::Keyboard::Space)
    {
        if (menuIndex == 0) startNewGame();
        else if (menuIndex == 1) continueGame();
        else if (menuIndex == 2) difficultyChoice = (difficultyChoice >= hardest) ? DIFF_EASY : difficultyChoice + 1;
        else if (menuIndex == 3) { fullscreen = !fullscreen; createWindow(window); }
        else if (menuIndex == 4) { creditsScroll = 0.f; state = STATE_CREDITS; }
        else window.close();
    }
}

static void exploreKey(sf::Keyboard::Key key)
{
    //while paused only two keys do anything
    if (paused)
    {
        if (key == sf::Keyboard::Escape) { paused = false; pauseNotice = ""; playUiSound(UI_SOUND_BACK); }
        if (key == sf::Keyboard::S)
        {
            bool saved = saveGame();
            pauseNotice = saved ? "Game saved." : "Could not save the game!";
            playUiSound(saved ? UI_SOUND_SELECT : UI_SOUND_BACK);
        }
        if (key == sf::Keyboard::M) { playUiSound(UI_SOUND_SELECT); goToMenu(); }
        return;
    }

    int rowChange = 0;
    int colChange = 0;
    if (key == sf::Keyboard::W || key == sf::Keyboard::Up) rowChange = -1;
    if (key == sf::Keyboard::S || key == sf::Keyboard::Down) rowChange = 1;
    if (key == sf::Keyboard::A || key == sf::Keyboard::Left) colChange = -1;
    if (key == sf::Keyboard::D || key == sf::Keyboard::Right) colChange = 1;

    if (rowChange != 0 || colChange != 0)
    {
        battleNote = "";
        if (movePlayer(rowChange, colChange)) handleMapResult(stepOnTile());
    }
    else if (key == sf::Keyboard::E)
    {
        battleNote = "";
        int result = interactWithTile();
        //a "select" sound when E did something, the "not allowed" sound when it did not
        bool refused = (result == RESULT_NOTHING || result == RESULT_LOCKED || result == RESULT_FULL);
        playUiSound(refused ? UI_SOUND_BACK : UI_SOUND_SELECT);
        handleMapResult(result);
    }
    else if (key == sf::Keyboard::I)
    {
        bagNotice = "";
        playUiSound(UI_SOUND_SELECT);
        state = STATE_INVENTORY;
    }
    else if (key == sf::Keyboard::Escape)
    {
        paused = true;
        playUiSound(UI_SOUND_SELECT);
    }
}

static void bagKey(sf::Keyboard::Key key)
{
    int oldIndex = bagIndex;
    //the bag is 4 slots wide and 2 rows tall
    if (key == sf::Keyboard::Left  && bagIndex % 4 > 0) bagIndex--;
    if (key == sf::Keyboard::Right && bagIndex % 4 < 3) bagIndex++;
    if (key == sf::Keyboard::Up    && bagIndex >= 4)    bagIndex -= 4;
    if (key == sf::Keyboard::Down  && bagIndex + 4 < INVENTORY_SLOTS) bagIndex += 4;

    if (bagIndex != oldIndex) playUiSound(UI_SOUND_MOVE);      //cursor moved to another slot

    if (key == sf::Keyboard::Escape || key == sf::Keyboard::I)
    {
        retryPendingReward();      //maybe there is room for a waiting reward now
        playUiSound(UI_SOUND_BACK);
        state = STATE_EXPLORE;
        return;
    }

    // S sorts the bag A-Z (bubble sort in Player.cpp)
    if (key == sf::Keyboard::S)
    {
        sortInventory();
        bagNotice = "Bag sorted A-Z.";
        playUiSound(UI_SOUND_SELECT);
        return;
    }

    if (key == sf::Keyboard::Enter || key == sf::Keyboard::Space)
    {
        int id = getSlotItemId(bagIndex);
        bool used = (id == ITEM_HEALING_POTION && getPlayerHP() < getPlayerMaxHP());
        playUiSound(used ? UI_SOUND_SELECT : UI_SOUND_BACK);     //only drinking a potion counts as "done"
        if (id < 0)
        {
            bagNotice = "That slot is empty.";
        }
        else if (id == ITEM_HEALING_POTION)
        {
            if (getPlayerHP() >= getPlayerMaxHP()) bagNotice = "You are already at full health.";
            else { useItem(id); bagNotice = "You drink the potion."; }
        }
        else if (id == ITEM_BUFF_POTION || id == ITEM_POISON_POTION)
        {
            bagNotice = "Better to save this one for a fight.";
        }
        else
        {
            bagNotice = "You can't use that here.";
        }
    }
}

static void sceneKey(sf::Keyboard::Key key)
{
    if (key != sf::Keyboard::Enter && key != sf::Keyboard::Space && key != sf::Keyboard::E) return;

    playUiSound(UI_SOUND_SELECT);
    sceneIndex++;
    if (sceneIndex >= (int)sceneLines.size())
    {
        state = sceneNextState;
        if (startHydraAfterScene)
        {
            startHydraAfterScene = false;
            levelBeforeBattle = getPlayerLevel();
            if (beginBattle(ENEMY_HYDRA, getPlayerRow(), getPlayerCol()))
            {
                battleNote = "";
                state = STATE_BATTLE;
            }
        }
    }
}

//MAIN LOOP-

int runGame()
{
    srand((unsigned)time(0));
    loadUnlocked();

    sf::RenderWindow window;
    createWindow(window);

    sf::Font font;
    if (!font.loadFromFile("assets/fonts/game.ttf") && !font.loadFromFile("C:/Windows/Fonts/georgia.ttf") && !font.loadFromFile("arial.ttf"))
    {
        std::cout << "Could not load a font. Put any .ttf file at assets/fonts/game.ttf\n";
        return 1;
    }

    initializeMap();      //so the map exists even before the first game is started
    resetBattleSystem();

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();

            if (event.type != sf::Event::KeyPressed) continue;
            sf::Keyboard::Key key = event.key.code;

            //F11 works everywhere. (the window is recreated, SFML needs that to switch modes)
            if (key == sf::Keyboard::F11) { fullscreen = !fullscreen; createWindow(window); continue; }

            if (state == STATE_MENU)           menuKey(key, window);
            else if (state == STATE_EXPLORE)   exploreKey(key);
            else if (state == STATE_INVENTORY) bagKey(key);
            else if (state == STATE_DIALOGUE)  sceneKey(key);
            else if (state == STATE_BATTLE)    { if (handleBattleKey(key)) afterBattle(); }
            else if (state == STATE_GAME_OVER) { if (key == sf::Keyboard::Enter || key == sf::Keyboard::Space) goToMenu(); }
            else if (state == STATE_CREDITS)   { if (key == sf::Keyboard::Escape || key == sf::Keyboard::Enter) goToMenu(); }
        }

        //music: menu + credits have the menu song, walking around (and the bag, story boxes...) have the
        //exploration song. the battle starts and stops its own music (BattleScreen.cpp), so we leave it alone then
        if (state == STATE_MENU || state == STATE_CREDITS) playMusicTrack("menu");
        else if (state == STATE_GAME_OVER)                 playMusicTrack("");
        else if (state != STATE_BATTLE)                    playMusicTrack("explore");

        //draw
        window.clear(sf::Color(10, 8, 14));
        setLetterboxView(window);     //the battle screen changes the view, so we set it again every frame

        //the line shown in the side panel: what the map says, or what the last battle said
        std::string hudMessage = getMapMessage();
        if (hudMessage == "") hudMessage = battleNote;

        if (state == STATE_MENU)
        {
            drawMainMenu(window, font, menuIndex, difficultyChoice, hardUnlocked, saveExists(), fullscreen, getHighestScore());
        }
        else if (state == STATE_BATTLE)
        {
            drawBattleScreen(window, font);
        }
        else if (state == STATE_CREDITS)
        {
            creditsScroll += 1.2f;
            if (drawCredits(window, font, creditsScroll)) goToMenu();
        }
        else
        {
            //exploring, bag, story box, pause and game over are all drawn on top of the map
            drawMap(window, font);
            drawHud(window, font, hudMessage);

            if (state == STATE_INVENTORY) drawInventory(window, font, bagIndex, bagNotice);
            if (state == STATE_DIALOGUE)  drawTextScene(window, font, sceneTitle, sceneLines[sceneIndex]);
            if (state == STATE_GAME_OVER) drawGameOver(window, font);
            if (state == STATE_EXPLORE && paused) drawPauseMenu(window, font, pauseNotice);
        }
        window.display();
    }
    shutdownSound();        //free the sounds and music before closing (stops the OpenAL cleanup warning)
    return 0;
}
