/*first-person battle screen
only draws, animates and reads keys. rules in Battle.cpp / Enemy.cpp.
ART:
assets/sprites/scientist.png, bat.png, hydra.png   boss pictures (transparent PNG, any size)
assets/sprites/viktor_battle.png
assets/backgrounds/battle_0.png, battle_1.png, battle_2.png   one per boss (0=Scientist, 1=Bat, 2=Hydra) or one shared assets/backgrounds/battle.png
for SOUNDS + MUSIC: see BattleSound.h */
#include "BattleScreen.h"
#include "BattleSound.h"
#include "UiDraw.h"
#include "Battle.h"
#include "Enemy.h"
#include "PlayerInterface.h"
#include <cmath>
#include <string>
#include <vector>

//basic settings
static const float SCREEN_H = (float)BATTLE_DESIGN_H;
static const float MIN_ASPECT = 1.4f;                 //narrowest window 
static const float MAX_ASPECT = 2.4f;                 //widest window
static float screenW = 1280.f;                        //current width. set every frame from the window

static const float GROUND_Y = 550.f;                  //where the boss feet touch the floor
static const float PANEL_Y = 560.f;                  //top of the bottom row of panels
static const float PANEL_H = 145.f;

static const float BOSS_HEIGHT[ENEMY_TYPE_COUNT] = { 350.f, 385.f, 385.f };   //in boss order (scentist,bat,hydra)
static const char* BOSS_FILE[ENEMY_TYPE_COUNT] = { "assets/sprites/scientist.png","assets/sprites/bat.png", "assets/sprites/hydra.png" };
static const bool SMOOTH_BOSS_SPRITES = true;            //for a pixelated look, make it false
static const char* VIKTOR_FILE = "assets/sprites/viktor_battle.png";

//were each Hydra head is inside hydra.png (fractions of its width / height)
static const float HEAD_FX[HYDRA_HEADS] = { 0.17f, 0.61f, 0.85f };
static const float HEAD_FY[HYDRA_HEADS] = { 0.19f, 0.08f, 0.26f };

//screen state
static int lastBattleSeen = -1, menuIndex = 0, gridIndex = 0, lastRoundSeen = 0;
static bool inItemGrid = false;
static bool showLog = true;                           //if true it shows the text box shows what just happened
static std::string notice = "";                       //message that replaces the text box

static float enemyHitTime = -10.f, playerHurtTime = -10.f, wonTime = -1.f;          //hit effects 

static const int MAX_NUMS = 8;                        //floating dmg numbers
static std::string numText[MAX_NUMS];
static float numX[MAX_NUMS], numY[MAX_NUMS], numStart[MAX_NUMS];
static bool numIsPlayer[MAX_NUMS], numActive[MAX_NUMS];

static float animTime()
{
    static sf::Clock clock;
    return clock.getElapsedTime().asSeconds();
}

static void syncUiToBattle()      //once a new battle starts, reset the screen
{
    if (battleCounter() == lastBattleSeen) return;
    lastBattleSeen = battleCounter();
    menuIndex = gridIndex = 0;
    inItemGrid = false;
    showLog = true;
    notice = "";
    lastRoundSeen = roundCounter();
    enemyHitTime = playerHurtTime = -10.f;
    wonTime = -1.f;
    for (int i = 0; i < MAX_NUMS; i++) numActive[i] = false;
    clearBattleSounds();
    startBattleMusic(getEnemyType());
}

//ensuring the window doesnt stretch
static sf::View makeView(const sf::RenderWindow& window)
{
    sf::Vector2u size = window.getSize();
    float windowAspect = (float)size.x / (float)size.y;
    float aspect = windowAspect;
    if (aspect < MIN_ASPECT) aspect = MIN_ASPECT;
    if (aspect > MAX_ASPECT) aspect = MAX_ASPECT;
    screenW = SCREEN_H * aspect;                      //wide window so wide screen

    float x = 0.f, y = 0.f, w = 1.f, h = 1.f;              //black bars if the window is outside the range
    if (windowAspect > aspect) { w = aspect / windowAspect; x = (1.f - w) / 2.f; }
    else if (windowAspect < aspect) { h = windowAspect / aspect; y = (1.f - h) / 2.f; }

    sf::View view(sf::FloatRect(0.f, 0.f, screenW, SCREEN_H));
    view.setViewport(sf::FloatRect(x, y, w, h));
    return view;
}

//animation
//small idle movements so nothing stands perfectly still.     kind: an ENEMY_* type or -1 for viktor.
static void idleMotion(int kind, float t, float& dx, float& dy, float& stretchY, float& rotation)
{
    dx = dy = rotation = 0.f;
    stretchY = 1.f;
    if (kind == ENEMY_MAD_SCIENTIST)                  //twitchy erratic
    {
        dx = 2.5f * std::sin(t * 13.f) * std::sin(t * 3.1f);
        dy = 3.f * std::sin(t * 2.2f) + 1.5f * std::sin(t * 17.f);
        rotation = 1.5f * std::sin(t * 4.7f) * std::sin(t * 1.3f);
        stretchY = 1.f + 0.01f * std::sin(t * 3.f);
    }
    else if (kind == ENEMY_HUMANOID_BAT)              //hovers, drifts
    {
        dy = -6.f + 6.f * std::sin(t * 2.4f);
        dx = 5.f * std::sin(t * 1.2f);
        rotation = 1.5f * std::sin(t * 1.7f);
    }
    else if (kind == ENEMY_HYDRA)                     //meant to be slow heavy breathing
    {
        dx = 1.5f * std::sin(t * 0.7f);
        dy = 2.f * std::sin(t * 1.3f);
        stretchY = 1.f + 0.012f * std::sin(t * 1.3f);
    }
    else dy = 1.5f * std::sin(t * 1.8f);              //sort of gentle breathing
}

//a quick shake that goes away.
//since = seconds since the event.
static float shake(float since, float duration, float strength, float speed)
{
    if (since < 0.f || since >= duration) return 0.f;
    return std::sin(since * speed) * strength * (1.f - since / duration);
}

//positions and hit effects
static float bossWidth(int type)
{
    sf::Texture* tex = loadTexture(BOSS_FILE[type], SMOOTH_BOSS_SPRITES);
    if (!tex) return BOSS_HEIGHT[type] * 0.7f;
    sf::Vector2u s = tex->getSize();
    return s.x * BOSS_HEIGHT[type] / (float)s.y;
}

static void headPoint(int head, float& x, float& y)   //hydra head resting pos
{
    float h = BOSS_HEIGHT[ENEMY_HYDRA], w = bossWidth(ENEMY_HYDRA);
    x = screenW / 2.f - w / 2.f + HEAD_FX[head] * w;
    y = GROUND_Y - h + HEAD_FY[head] * h;
}

static void spawnNumber(int value, float x, float y, bool onPlayer, float t)
{
    for (int i = 0; i < MAX_NUMS; i++)
    {
        if (numActive[i]) continue;
        numActive[i] = true;
        numText[i] = std::to_string(value);
        numX[i] = x; numY[i] = y; numStart[i] = t;
        numIsPlayer[i] = onPlayer;
        return;
    }
}

static void updateEffects(float t)
{
    int round = roundCounter();
    if (round != lastRoundSeen)                       //a new round was played
    {
        lastRoundSeen = round;
        int toEnemy = lastDamageDealt(), toPlayer = lastDamageTaken();
        if (toEnemy > 0)
        {
            enemyHitTime = t;
            float x = screenW / 2.f, y = GROUND_Y - BOSS_HEIGHT[getEnemyType()] * 0.7f;
            if (getEnemyType() == ENEMY_HYDRA && lastHeadHit() >= 0) headPoint(lastHeadHit(), x, y);
            spawnNumber(toEnemy, x, y, false, t);
        }
        if (toPlayer > 0)
        {
            playerHurtTime = t;
            spawnNumber(toPlayer, 76.f, PANEL_Y + 40.f, true, t);
        }
    }
    if (getBattleResult() == BATTLE_WON && wonTime < 0.f) wonTime = t;
}

static void drawNumbers(sf::RenderWindow& window, const sf::Font& font, float t)
{
    for (int i = 0; i < MAX_NUMS; i++)
    {
        if (!numActive[i]) continue;
        float age = t - numStart[i];
        if (age > 1.1f) { numActive[i] = false; continue; }

        sf::Uint8 alpha = (sf::Uint8)(255.f * (1.f - age / 1.1f));
        sf::Text text(numText[i], font, 34);
        text.setFillColor(numIsPlayer[i] ? sf::Color(255, 110, 110, alpha) : sf::Color(255, 235, 160, alpha));
        text.setOutlineColor(sf::Color(0, 0, 0, alpha));
        text.setOutlineThickness(2.f);
        text.setPosition(numX[i] - text.getLocalBounds().width / 2.f, numY[i] - 45.f * age);
        window.draw(text);
    }
}

//scene: background, boss, bats, hydra heads
static void drawBackground(sf::RenderWindow& window, int type)
{
    sf::Texture* tex = loadTexture("assets/backgrounds/battle_" + std::to_string(type) + ".png");
    if (!tex) tex = loadTexture("assets/backgrounds/battle.png");

    if (tex)       //fill the screen and keep the picture's shape. centre it but edges are cropped
    {
        sf::Vector2u s = tex->getSize();
        float scale = screenW / s.x;
        if (SCREEN_H / s.y > scale) scale = SCREEN_H / s.y;
        sf::Sprite bg(*tex);
        bg.setScale(scale, scale);
        bg.setPosition((screenW - s.x * scale) / 2.f, (SCREEN_H - s.y * scale) / 2.f);
        window.draw(bg);
        return;
    }

    sf::VertexArray sky(sf::Quads, 4);               //dark gradient + floor (alt)
    sky[0] = sf::Vertex(sf::Vector2f(0.f, 0.f),             sf::Color(34, 28, 44));
    sky[1] = sf::Vertex(sf::Vector2f(screenW, 0.f),         sf::Color(34, 28, 44));
    sky[2] = sf::Vertex(sf::Vector2f(screenW, GROUND_Y),    sf::Color(16, 13, 20));
    sky[3] = sf::Vertex(sf::Vector2f(0.f, GROUND_Y),        sf::Color(16, 13, 20));
    window.draw(sky);
    sf::RectangleShape floor(sf::Vector2f(screenW, SCREEN_H - GROUND_Y));
    floor.setPosition(0.f, GROUND_Y);
    floor.setFillColor(sf::Color(22, 18, 24));
    window.draw(floor);
}

static void drawSummonedBats(sf::RenderWindow& window, float t, sf::Uint8 alpha)
{
    const float offX[MAX_BATS] = { -330.f, 330.f, -220.f };
    const float offY[MAX_BATS] = { -170.f, -205.f, -325.f };
    sf::Texture* tex = loadTexture(BOSS_FILE[ENEMY_HUMANOID_BAT], SMOOTH_BOSS_SPRITES);

    for (int i = 0; i < getBatCount() && i < MAX_BATS; i++)
    {
        float x = screenW / 2.f + offX[i] + 6.f * std::sin(t * 1.9f + i * 1.3f);     //each bat flutters
        float y = GROUND_Y + offY[i] + 7.f * std::sin(t * 3.3f + i * 2.1f);
        if (tex)
        {
            float l, tp, w, h;
            drawSprite(window, tex, x, y, 115.f, sf::Color(205, 175, 195, alpha), 1.f, 0.f, l, tp, w, h);
        }
        else
        {
            sf::CircleShape blob(18.f);
            blob.setPosition(x - 18.f, y - 36.f);
            blob.setFillColor(sf::Color(90, 60, 100, alpha));
            window.draw(blob);
        }
    }
}

static void drawHydraHeads(sf::RenderWindow& window, const sf::Font& font, float left, float top, float w, float h)
{
    int target = getTargetHead();
    for (int i = 0; i < HYDRA_HEADS; i++)
    {
        float hx = left + HEAD_FX[i] * w, hy = top + HEAD_FY[i] * h;
        bool alive = isHeadAlive(i);

        sf::CircleShape mark(alive ? 34.f : 30.f);          //defeated head: dark patch. guarding head: blue ring
        mark.setOrigin(mark.getRadius(), mark.getRadius());
        mark.setPosition(hx, hy);

        if (!alive) { mark.setFillColor(sf::Color(8, 6, 10, 200)); window.draw(mark); }
        else if (isHeadDefending(i)) { mark.setFillColor(sf::Color::Transparent); mark.setOutlineColor(sf::Color(110, 180, 255)); mark.setOutlineThickness(3.f); window.draw(mark); }

        float barX = hx - 58.f, barY = hy - 48.f;
        drawBar(window, barX, barY, 116.f, 9.f, getHeadHP(i), getHeadMaxHP(i), alive ? sf::Color(190, 45, 45) : sf::Color(70, 70, 70));
        drawText(window, font, std::to_string(i + 1), 15, barX - 18.f, barY - 6.f, alive ? TEXT_COLOR : DIM_COLOR);
        if (i == target && alive && getBattleResult() == BATTLE_ONGOING && !inItemGrid)
        drawTriangle(window, hx - 7.f, barY - 24.f, 14.f, true, GOLD);      //the head player  will hit
    }
}

static void drawBoss(sf::RenderWindow& window, const sf::Font& font, float t)
{
    int type = getEnemyType();
    if (type < 0 || type >= ENEMY_TYPE_COUNT) return;
    float height = BOSS_HEIGHT[type], cx = screenW / 2.f;

    float dx, dy, stretch, rot;
    idleMotion(type, t, dx, dy, stretch, rot);
    float sinceHit = t - enemyHitTime;
    dx += shake(sinceHit, 0.3f, 9.f, 70.f);

    int g = 255, b = 255;                             //redder with power ups,red flash when hit
    if (type == ENEMY_MAD_SCIENTIST) { g -= 45 * getScientistPower(); b -= 70 * getScientistPower(); }
    if (sinceHit >= 0.f && sinceHit < 0.3f) { int k = (int)(120.f * (1.f - sinceHit / 0.3f)); g -= k; b -= k; }
    if (g < 0) g = 0;
    if (b < 0) b = 0;
    int alpha = 255;                                  //fade away after victory
    if (wonTime >= 0.f) { float k = 1.f - (t - wonTime) / 1.3f; alpha = (k <= 0.f) ? 0 : (int)(255.f * k); }
    sf::Color tint(255, (sf::Uint8)g, (sf::Uint8)b, (sf::Uint8)alpha);

    float left = cx - 100.f, top = GROUND_Y - height, w = 200.f, h = height;
    sf::Texture* tex = loadTexture(BOSS_FILE[type], SMOOTH_BOSS_SPRITES);
    if (tex) drawSprite(window, tex, cx + dx, GROUND_Y + dy, height, tint, stretch, rot, left, top, w, h);
    else
    {
        sf::RectangleShape box(sf::Vector2f(w, h));   //made a placeholder for the picture if it doenst exist
        box.setPosition(left + dx, top + dy);
        box.setFillColor(sf::Color(90, 60, 70, (sf::Uint8)alpha));
        window.draw(box);
    }

    if (type == ENEMY_HUMANOID_BAT) drawSummonedBats(window, t, (sf::Uint8)alpha);
    if (type == ENEMY_HYDRA && alpha > 0) drawHydraHeads(window, font, left, top, w, h);
}

//panels
static void drawEnemyPanel(sf::RenderWindow& window, const sf::Font& font)       //topleft
{
    drawPanel(window, 20.f, 16.f, 250.f, 76.f);
    drawCenteredText(window, font, getEnemyName(), 22, 145.f, 20.f);
    drawText(window, font, "HP", 17, 34.f, 50.f, GOLD);
    drawText(window, font, std::to_string(getEnemyHP()) + " / " + std::to_string(getEnemyMaxHP()), 17, 68.f, 50.f);
    drawBar(window, 34.f, 76.f, 222.f, 8.f, getEnemyHP(), getEnemyMaxHP(), sf::Color(190, 45, 45));

    int type = getEnemyType();                        //hint under the panel thats boss specific
    if (type == ENEMY_MAD_SCIENTIST && getScientistPower() > 0)
    drawText(window, font, "Power-ups: " + std::to_string(getScientistPower()), 15, 24.f, 100.f, sf::Color(255, 170, 90));
    if (type == ENEMY_HUMANOID_BAT && getBatCount() > 0)
    drawText(window, font, "Summoned bats: " + std::to_string(getBatCount()), 15, 24.f, 100.f, sf::Color(205, 170, 255));
    if (type == ENEMY_HYDRA)
    drawText(window, font, "The heads wail in " + std::to_string(hydraTurnsUntilWail()) + " turn(s)", 15, 24.f, 100.f, sf::Color(235, 120, 120));
}

static void drawPartyPanel(sf::RenderWindow& window, const sf::Font& font, float t)     //bottomleft
{
    drawPanel(window, 20.f, PANEL_Y, 280.f, PANEL_H);

    sf::Texture* tex = loadTexture(VIKTOR_FILE);
    if (tex)
    {
        float dx, dy, stretch, rot, l, tp, w, h;
        idleMotion(-1, t, dx, dy, stretch, rot);
        dx += shake(t - playerHurtTime, 0.3f, 6.f, 70.f);                 //flinches when hurt
        drawSprite(window, tex, 76.f + dx, PANEL_Y + PANEL_H - 8.f + dy, (float)tex->getSize().y, sf::Color::White, stretch, rot, l, tp, w, h);
    }

    drawText(window, font, "Viktor", 20, 135.f, PANEL_Y + 8.f, GOLD);
    drawText(window, font, "HP", 16, 135.f, PANEL_Y + 34.f, GOLD);
    drawText(window, font, std::to_string(getPlayerHP()) + " / " + std::to_string(getPlayerMaxHP()), 16, 162.f, PANEL_Y + 34.f);
    drawBar(window, 135.f, PANEL_Y + 56.f, 140.f, 9.f, getPlayerHP(), getPlayerMaxHP(), sf::Color(70, 190, 90));
    drawText(window, font, "ATK Lv " + std::to_string(getplayerAtkLevel()) + "/" + std::to_string(getPlayerMaxAttackLevel()), 14, 135.f, PANEL_Y + 68.f, DIM_COLOR);

    float y = PANEL_Y + 90.f;                         //status effects
    if (isPlayerDefending()) { drawText(window, font, "Guarding", 14, 135.f, y, sf::Color(130, 190, 255)); y += 17.f; }
    if (playerBuffTurns() > 0) { drawText(window, font, "Buffed (" + std::to_string(playerBuffTurns()) + ")", 14, 135.f, y, GOLD); y += 17.f; }
    if (playerCurseTurns() > 0)
    drawText(window, font, "Cursed: -" + std::to_string(playerCurseDamage()) + "/turn (" + std::to_string(playerCurseTurns()) + ")", 14, 135.f, y, sf::Color(220, 110, 220));
}

static const char* MENU_NAMES[4] = { "Attack", "Item", "Defend", "Run" };
static int menuCount() { return canRunInThisBattle() ? 4 : 3; }      //run only being an option in the scientist battle

static std::string commandHelp(int index)
{
    if (index == 0) return (getEnemyType() == ENEMY_HYDRA) ? "Strike the marked head. (Left / Right to choose a head)" : "Strike the enemy with your weapon.";
    if (index == 1) return "Use an item from your pack.";
    if (index == 2) return "Brace yourself. Take half damage from the next attacks.";
    return "Try to escape from the fight...";
}

static void drawItemGrid(sf::RenderWindow& window, const sf::Font& font, float startX)
{
    const float slotW = 100.f, slotH = 58.f, gap = 6.f;
    for (int i = 0; i < INVENTORY_SLOTS; i++)
    {
        float x = startX + (i % 4) * (slotW + gap), y = PANEL_Y + 12.f + (i / 4) * (slotH + gap);
        sf::RectangleShape box(sf::Vector2f(slotW, slotH));
        box.setPosition(x, y);
        box.setFillColor(sf::Color(26, 22, 30));
        box.setOutlineColor(i == gridIndex ? GOLD : sf::Color(80, 68, 62));
        box.setOutlineThickness(i == gridIndex ? 3.f : 1.f);
        window.draw(box);

        int id = getSlotItemId(i);
        if (id < 0) continue;
        std::vector<std::string> name = wrapText(font, getItemName(id), 14, slotW - 10.f);
        for (size_t l = 0; l < name.size() && l < 3; l++)
        drawText(window, font, name[l], 14, x + 5.f, y + 12.f + l * 16.f, isBattleItem(id) ? TEXT_COLOR : DIM_COLOR);
        if (getSlotItemCount(i) > 1)       //superscript for how many copies of the items
        drawText(window, font, std::to_string(getSlotItemCount(i)), 12, x + slotW - 17.f, y + 2.f, GOLD);
    }
}

//text box says: a notice or what just happened or help for the highlighted choice
static std::vector<std::string> textBoxLines()
{
    std::vector<std::string> lines;
    bool ongoing = (getBattleResult() == BATTLE_ONGOING);
    if (!notice.empty()) lines.push_back(notice);
    else if (!ongoing || (showLog && battleLogCount() > 0))
    for (int i = 0; i < battleLogCount(); i++) lines.push_back(getBattleLogLine(i));
    else if (inItemGrid)
    {
        int id = getSlotItemId(gridIndex);
        if (id < 0) lines.push_back("An empty slot.");
        else if (isBattleItem(id)) lines.push_back("Use the " + getItemName(id) + ".");
        else lines.push_back("The " + getItemName(id) + " can't be used in battle.");
    }
    else lines.push_back(commandHelp(menuIndex));
    return lines;
}

static void drawBottomPanels(sf::RenderWindow& window, const sf::Font& font)
{
    bool ongoing = (getBattleResult() == BATTLE_ONGOING);

    if (inItemGrid && ongoing)          //the item grid takes the middle and right panels
    {
        drawPanel(window, 312.f, PANEL_Y, screenW - 332.f, PANEL_H);
        drawItemGrid(window, font, 326.f);
        drawWrapped(window, font, textBoxLines(), 768.f, PANEL_Y + 14.f, screenW - 802.f, 5);
        return;
    }

    drawPanel(window, 312.f, PANEL_Y, 180.f, PANEL_H);                    //command menu
    if (ongoing)
        for (int i = 0; i < menuCount(); i++)
        {
            float y = PANEL_Y + 14.f + i * 30.f;
            if (i == menuIndex) drawTriangle(window, 330.f, y + 8.f, 12.f, false, GOLD);
            drawText(window, font, MENU_NAMES[i], 24, 354.f, y, i == menuIndex ? GOLD : TEXT_COLOR);
        }
    else
    {
        bool won = (getBattleResult() == BATTLE_WON);
        drawCenteredText(window, font, won ? "VICTORY" : "DEFEATED", 26, 402.f, PANEL_Y + 38.f, won ? GOLD : sf::Color(200, 80, 80));
        drawCenteredText(window, font, "Press Enter", 16, 402.f, PANEL_Y + 90.f, DIM_COLOR);
    }

    drawPanel(window, 504.f, PANEL_Y, screenW - 524.f, PANEL_H);          //text box
    drawWrapped(window, font, textBoxLines(), 520.f, PANEL_Y + 14.f, screenW - 556.f, 5);
}

static void drawIntro(sf::RenderWindow& window, const sf::Font& font)    //boss 1 dailogue (scientist)
{
    drawPanel(window, 20.f, PANEL_Y, screenW - 40.f, PANEL_H);
    drawText(window, font, getEnemyName(), 22, 40.f, PANEL_Y + 12.f, GOLD);
    std::vector<std::string> line(1, getIntroLine(getIntroIndex()));
    drawWrapped(window, font, line, 40.f, PANEL_Y + 46.f, screenW - 80.f, 3, 20);
    drawTriangle(window, screenW - 52.f, PANEL_Y + 118.f, 12.f, true, GOLD);
}

//functions
void drawBattleScreen(sf::RenderWindow& window, const sf::Font& font)
{
    syncUiToBattle();
    sf::View view = makeView(window);                 //also sets screen width for this frame
    float t = animTime();
    updateBattleSounds();
    updateEffects(t);

    window.clear(sf::Color(8, 6, 12));

    float sinceHurt = t - playerHurtTime;             //screen shake when player is hurt
    sf::View sceneView = view;
    sceneView.move(shake(sinceHurt, 0.3f, 7.f, 70.f), shake(sinceHurt, 0.3f, 5.f, 83.f));
    window.setView(sceneView);
    drawBackground(window, getEnemyType());
    drawBoss(window, font, t);
    drawNumbers(window, font, t);

    window.setView(view);                  //panels dont shake ever
    if (sinceHurt >= 0.f && sinceHurt < 0.35f)        //red flash
    {
        sf::RectangleShape flash(sf::Vector2f(screenW, SCREEN_H));
        flash.setFillColor(sf::Color(200, 20, 20, (sf::Uint8)(90.f * (1.f - sinceHurt / 0.35f))));
        window.draw(flash);
    }

    if (isInIntro()) { drawIntro(window, font); return; }
    drawEnemyPanel(window, font);
    drawPartyPanel(window, font, t);
    drawBottomPanels(window, font);
}

bool handleBattleKey(sf::Keyboard::Key key)
{
    if (!isBattleActive()) return true;
    syncUiToBattle();
    bool confirm = (key == sf::Keyboard::Enter || key == sf::Keyboard::Space);

    if (isInIntro())   //dialogue before fight
    {
        if (confirm) { advanceIntro(); playUiSound(UI_SOUND_SELECT); }
        return false;
    }

    if (getBattleResult() != BATTLE_ONGOING)         //once fight is over, pressing enter will leave the battle
    {
        if (!confirm) return false;
        playUiSound(UI_SOUND_SELECT);
        stopBattleMusic();
        endBattle();
        return true;
    }

    //item grid
    if (inItemGrid)
    {
        int old = gridIndex;
        if (key == sf::Keyboard::Left && gridIndex % 4 > 0) gridIndex--;
        if (key == sf::Keyboard::Right && gridIndex % 4 < 3) gridIndex++;
        if (key == sf::Keyboard::Up && gridIndex >= 4) gridIndex -= 4;
        if (key == sf::Keyboard::Down && gridIndex + 4 < INVENTORY_SLOTS) gridIndex += 4;
        if (gridIndex != old) { notice = ""; showLog = false; playUiSound(UI_SOUND_MOVE); }

        if (key == sf::Keyboard::Escape) { inItemGrid = false; notice = ""; showLog = false; playUiSound(UI_SOUND_BACK); }
        else if (confirm)
        {
            int id = getSlotItemId(gridIndex);
            if (id < 0) { notice = "That slot is empty."; playUiSound(UI_SOUND_BACK); }
            else if (!isBattleItem(id)) { notice = "The " + getItemName(id) + " can't be used in battle."; playUiSound(UI_SOUND_BACK); }
            else
            {
                playUiSound(UI_SOUND_SELECT);
                playerAction(ACTION_ITEM, id);
                inItemGrid = false; notice = ""; showLog = true;
            }
        }
        return false;
    }

    int count = menuCount();                          //kind of a main menu
    if (menuIndex >= count) menuIndex = 0;
    if (key == sf::Keyboard::Up || key == sf::Keyboard::Down)
    {
        menuIndex = (menuIndex + (key == sf::Keyboard::Up ? count - 1 : 1)) % count;
        showLog = false; notice = "";
        playUiSound(UI_SOUND_MOVE);
    }
    else if ((key == sf::Keyboard::Left || key == sf::Keyboard::Right) && getEnemyType() == ENEMY_HYDRA)
    {
        cycleTargetHead(key == sf::Keyboard::Left ? -1 : 1);
        playUiSound(UI_SOUND_MOVE);
    }
    else if (confirm)
    {
        notice = "";
        playUiSound(UI_SOUND_SELECT);
        showLog = true;
        if (menuIndex == 0)playerAction(ACTION_ATTACK);
        else if (menuIndex == 1){ inItemGrid = true; gridIndex = 0; showLog = false; }
        else if (menuIndex == 2)playerAction(ACTION_DEFEND);
        else playerAction(ACTION_RUN);
    }
    return false;
}