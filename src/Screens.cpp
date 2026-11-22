//Screens.cpp ------ menu, hud, bag, story box, pause, game over and credits (drawing only)
//WHAT THIS FILE DOES: draws every screen that is not the map or the battle. it never changes
//the game, Game.cpp decides which screen to show and handles the keys
#include "Screens.h"
#include "UiDraw.h"
#include "Map.h"
#include "Story.h"
#include "PlayerInterface.h"
#include <algorithm>
#include <cmath>
#include <utility>

static std::string difficultyName(int d)
{
    if (d == DIFF_EASY) return "Easy";
    if (d == DIFF_HARD) return "Hard";
    return "Normal";
}

//a dark trasparent/seethrough layer so the thing behind it is still visible but like dimmer
static void dimScreen(sf::RenderWindow& window, int alpha)
{
    sf::RectangleShape layer(sf::Vector2f((float)SCREEN_W, (float)SCREEN_H));
    layer.setFillColor(sf::Color(0, 0, 0, alpha));
    window.draw(layer);
}

void drawMainMenu(sf::RenderWindow& window, const sf::Font& font, int selected, int difficulty, bool hardUnlocked, bool canContinue, bool fullscreen, int highScore)
{
    //SFML 2.6 does not decode MP4 files itself. The menu video is therefore
    //stored as numbered JPG frames in assets/backgrounds/bg_mainmenu/.
    //oad the frames once, then display them in order to create a looping video.
    static std::vector<sf::Texture> menuFrames;
    static bool triedLoadingMenuFrames = false;
    static sf::Clock menuVideoClock;
    const float MENU_VIDEO_FPS = 12.f;

    if (!triedLoadingMenuFrames)
    {
        triedLoadingMenuFrames = true;
        for (int i = 0; i < 1000; i++)
        {
            std::string number = std::to_string(i);
            while (number.length() < 3) number = "0" + number;
            std::string filename = "assets/backgrounds/bg_mainmenu/frame_" + number + ".jpg";

            sf::Texture frame;
            if (!frame.loadFromFile(filename)) break;  //stop at the first missing frame
            frame.setSmooth(false);                   //preserve the pixel-art look
            menuFrames.push_back(std::move(frame));
        }
    }

    if (!menuFrames.empty())
    {
        int frameIndex = (int)(menuVideoClock.getElapsedTime().asSeconds() * MENU_VIDEO_FPS)
                         % (int)menuFrames.size();
        sf::Texture& currentFrame = menuFrames[frameIndex];
        sf::Vector2u size = currentFrame.getSize();
        sf::Sprite videoFrame(currentFrame);

        //scale to cover the full 1280x720 menu without stretching the image.
        float scale = std::max((float)SCREEN_W / size.x, (float)SCREEN_H / size.y);
        videoFrame.setScale(scale, scale);
        float width = size.x * scale;
        float height = size.y * scale;
        videoFrame.setPosition((SCREEN_W - width) / 2.f, (SCREEN_H - height) / 2.f);
        window.draw(videoFrame);
    }
    else
    {
        //keep an alt background in case the frame folder is missing.
        sf::Texture* background = loadTexture("assets/backgrounds/battle_2.png", true);
        if (background)
        {
            sf::Vector2u size = background->getSize();
            float scale = std::max((float)SCREEN_W / size.x, (float)SCREEN_H / size.y);
            sf::Sprite image(*background);
            image.setScale(scale, scale);
            image.setPosition((SCREEN_W - size.x * scale) / 2.f,
                              (SCREEN_H - size.y * scale) / 2.f);
            window.draw(image);
        }
    }

    //dark panel keeps the menu readable without hiding the artwork everywhere.
    sf::RectangleShape shade(sf::Vector2f(500.f, (float)SCREEN_H));
    shade.setPosition(0.f, 0.f);
    shade.setFillColor(sf::Color(5, 5, 10, 205));
    window.draw(shade);

    drawText(window, font, "OUROBOROS", 65, 48.f, 62.f, GOLD);
    drawText(window, font, "the serpent eats its own tail", 18, 54.f, 151.f, DIM_COLOR);

    std::string lines[6];
    lines[0] = "Start New Game";
    lines[1] = "Continue";
    lines[2] = "Difficulty:  " + difficultyName(difficulty);
    lines[3] = std::string("Screen:  ") + (fullscreen ? "Fullscreen" : "Windowed");
    lines[4] = "Credits";
    lines[5] = "Exit";

    for (int i = 0; i < 6; i++)
    {
        sf::Color color = (i == selected) ? GOLD : TEXT_COLOR;
        if (i == 1 && !canContinue) color = DIM_COLOR;
        std::string text = (i == selected) ? ">  " + lines[i] : lines[i];
        drawText(window, font, text, 24, 62.f, 235.f + i * 54.f, color);
    }

    drawText(window, font, "High score: " + std::to_string(highScore), 18, 62.f, 590.f, GOLD);
    if (!hardUnlocked)
        drawText(window, font, "Hard unlocks after you beat the game on Normal", 14, 62.f, 622.f, DIM_COLOR);
    drawText(window, font, "Up / Down: move     Left / Right: change     Enter: choose     F11: fullscreen", 13, 24.f, 678.f, DIM_COLOR);
}

void drawHud(sf::RenderWindow& window, const sf::Font& font, const std::string& message)
{
    float x = 690.f;
    drawPanel(window, x, 40.f, 566.f, 640.f);

    drawText(window, font, getPlayerName(), 28, x + 24.f, 56.f, GOLD);
    drawText(window, font, difficultyName(getDifficulty()), 16, x + 460.f, 66.f, DIM_COLOR);

    drawText(window, font, "HP  " + std::to_string(getPlayerHP()) + " / " + std::to_string(getPlayerMaxHP()), 20, x + 24.f, 110.f);
    drawBar(window, x + 24.f, 142.f, 518.f, 16.f, getPlayerHP(), getPlayerMaxHP(), sf::Color(70, 190, 90));

    drawText(window, font, "Level " + std::to_string(getPlayerLevel()) + "     Attack level " + std::to_string(getplayerAtkLevel()) + " / " + std::to_string(getPlayerMaxAttackLevel()), 20, x + 24.f, 180.f);
    drawText(window, font, "EXP  " + std::to_string(getPlayerExp()) + " / " + std::to_string(getExpNeededForNextLevel()), 18, x + 24.f, 220.f, DIM_COLOR);
    drawBar(window, x + 24.f, 250.f, 518.f, 10.f, getPlayerExp(), getExpNeededForNextLevel(), sf::Color(90, 140, 230));

    //both keys are shown as just "Key" so the player can't tell the real one apart
    int keys = getItemCount(ITEM_REAL_KEY) + getItemCount(ITEM_DECOY_KEY);
    drawText(window, font, "Keys: " + std::to_string(keys), 20, x + 24.f, 300.f);
    drawText(window, font, "Relics: " + std::to_string(getRelicsFound()) + " / " + std::to_string(RELIC_COUNT), 20, x + 200.f, 300.f);
    drawText(window, font, "Bag: " + std::to_string(getTotalItems()) + " items", 20, x + 380.f, 300.f);

    //the message line (what just happened). wrapped for longer ones
    std::vector<std::string> lines;

    if (message != "") lines.push_back(message);
    drawWrapped(window, font, lines, x + 24.f, 360.f, 518.f, 4, 20);

    drawText(window, font, "WASD / arrows: move", 16, x + 24.f, 540.f, DIM_COLOR);
    drawText(window, font, "E: interact     I: bag     Esc: pause", 16, x + 24.f, 566.f, DIM_COLOR);
    drawText(window, font, "Red ! tiles are bosses. Careful.", 16, x + 24.f, 596.f, DIM_COLOR);
    drawText(window, font, "N = someone to talk to,  ? = something to read,  # = wall", 16, x + 24.f, 620.f, DIM_COLOR);
}

void drawInventory(sf::RenderWindow& window, const sf::Font& font, int selected, const std::string& notice)
{
    dimScreen(window, 200);
    drawPanel(window, 140.f, 70.f, 1000.f, 580.f);
    drawText(window, font, "Bag", 32, 170.f, 86.f, GOLD);
    drawText(window, font, "HP " + std::to_string(getPlayerHP()) + " / " + std::to_string(getPlayerMaxHP()), 20, 900.f, 96.f);

    //8 slots. 4 in a row
    for (int i = 0; i < INVENTORY_SLOTS; i++)
    {
        float sx = 170.f + (i % 4) * 235.f;
        float sy = 150.f + (i / 4) * 130.f;
        drawPanel(window, sx, sy, 215.f, 110.f);

        int id = getSlotItemId(i);
        if (id < 0)
        {
            drawText(window, font, "empty", 18, sx + 16.f, sy + 40.f, DIM_COLOR);
        }
        else
        {
            //picture on the right side of the slot (nothing is drawn if the file is missing)
            const float BOX = 56.f;                           //raise this to make the icons bigger
            drawItemIcon(window, id, sx + 215.f - 10.f - BOX / 2.f, sy + 55.f, BOX);
            drawText(window, font, getItemName(id), 20, sx + 16.f, sy + 24.f);
            drawText(window, font, "x" + std::to_string(getSlotItemCount(i)), 20, sx + 16.f, sy + 58.f, GOLD);
        }

        //gold frame around the selected slot
        if (i == selected)
        {
            sf::RectangleShape frame(sf::Vector2f(215.f, 110.f));
            frame.setPosition(sx, sy);
            frame.setFillColor(sf::Color::Transparent);
            frame.setOutlineColor(GOLD);
            frame.setOutlineThickness(3.f);
            window.draw(frame);
        }
    }

    //description of the item selected
    int id = getSlotItemId(selected);
    std::vector<std::string> description;
    if (id >= 0) description.push_back(getItemDescription(id));
    drawPanel(window, 170.f, 420.f, 940.f, 120.f);
    drawWrapped(window, font, description, 190.f, 440.f, 900.f, 3, 20);

    drawText(window, font, notice, 20, 170.f, 560.f, GOLD);
    drawText(window, font, "Arrows: move     Enter: use     S: sort A-Z     Esc or I: close", 16, 170.f, 610.f, DIM_COLOR);
}

void drawTextScene(sf::RenderWindow& window, const sf::Font& font, const std::string& title, const std::string& text)
{
    dimScreen(window, 150);
    drawPanel(window, 60.f, 440.f, 1160.f, 240.f);
    if (title != "") drawText(window, font, title, 26, 90.f, 458.f, GOLD);

    std::vector<std::string> lines;
    lines.push_back(text);
    drawWrapped(window, font, lines, 90.f, 506.f, 1100.f, 4, 22);
    drawText(window, font, "Press Enter", 16, 1090.f, 646.f, DIM_COLOR);
}

void drawPauseMenu(sf::RenderWindow& window, const sf::Font& font, const std::string& notice)
{
    dimScreen(window, 190);
    drawPanel(window, 440.f, 200.f, 400.f, 320.f);
    drawCenteredText(window, font, "Paused", 36, SCREEN_W / 2.f, 225.f, GOLD);
    drawCenteredText(window, font, "Esc - back to the game", 22, SCREEN_W / 2.f, 310.f);
    drawCenteredText(window, font, "S - save the game", 22, SCREEN_W / 2.f, 355.f);
    drawCenteredText(window, font, "M - quit to main menu", 22, SCREEN_W / 2.f, 400.f);
    drawCenteredText(window, font, notice, 18, SCREEN_W / 2.f, 460.f, GOLD);
}

void drawGameOver(sf::RenderWindow& window, const sf::Font& font)
{
    dimScreen(window, 230);
    drawCenteredText(window, font, "GAME OVER", 80, SCREEN_W / 2.f, 230.f, sf::Color(200, 70, 70));
    drawCenteredText(window, font, "Viktor falls, and the mist closes over him.", 22, SCREEN_W / 2.f, 350.f);
    drawCenteredText(window, font, "Press Enter to return to the main menu", 18, SCREEN_W / 2.f, 440.f, DIM_COLOR);
}

bool drawCredits(sf::RenderWindow& window, const sf::Font& font, float scroll)
{
    int count = getCreditsLineCount();
    for (int i = 0; i < count; i++)
    {
        float y = SCREEN_H - scroll + i * 44.f;
        if (y < -40.f || y > SCREEN_H) continue;       //off screen. don't bother
        unsigned size = (i == 0) ? 56 : 24;
        sf::Color color = (i == 0) ? GOLD : TEXT_COLOR;
        drawCenteredText(window, font, getCreditsLine(i), size, SCREEN_W / 2.f, y, color);
    }
    drawText(window, font, "Press Esc to skip", 14, 20.f, 690.f, DIM_COLOR);

    //done when the last line has gone past the top
    float lastY = SCREEN_H - scroll + (count - 1) * 44.f;
    return lastY < -40.f;
}
