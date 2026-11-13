/*from stubplayer.cpp so using fake player just to test thing. NOTE TO SELF: delete after full game is made!
controls: up and down to choose, enter to like perform action, left and right for hydra, esc backs out of items, F11 for full screen / window.
run from this folder so it can find the assets\ folder.*/
#include <SFML/Graphics.hpp>
#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include "Battle.h"
#include "BattleScreen.h"
#include "Enemy.h"
#include "PlayerInterface.h"

//frm PlayerStub.cpp: (max HP, attack level, healing, buff, poison)
void stubSetPlayer(int maxHP, int atkLevel, int healing, int buff, int poison);

//put dake player in the state thyed be in when reaching those bossess
static void startDemoBattle(int boss)
{
    if (boss == ENEMY_MAD_SCIENTIST) stubSetPlayer(100, 1, 2, 1, 1);
    if (boss == ENEMY_HUMANOID_BAT) stubSetPlayer(125, 2, 2, 1, 1);
    if (boss == ENEMY_HYDRA) stubSetPlayer(175, 3, 2, 1, 1);
    resetDefeatedEnemies();
    beginBattle(boss, boss, 0);
}

//opens the window to fit laptop screen, no stretching
static void openWindow(sf::RenderWindow& window, bool fullscreen)
{
    const char* title = "OUROBOROS - battle demo   (F11 = full screen)";
    sf::VideoMode desktop = sf::VideoMode::getDesktopMode();

    if (fullscreen)
    {
        window.create(desktop, title, sf::Style::Fullscreen);
    }
    else
    {
        float fit = std::min(desktop.width * 0.92f / BATTLE_WINDOW_W, desktop.height * 0.80f / BATTLE_WINDOW_H);
        if (fit > 1.5f) fit = 1.5f;
        window.create(sf::VideoMode((unsigned)(BATTLE_WINDOW_W * fit), (unsigned)(BATTLE_WINDOW_H * fit)), title, sf::Style::Default);
    }
    window.setFramerateLimit(60);
}

int main()
{
    srand((unsigned)time(0));
    resetBattleSystem();

    bool fullscreen = false;
    sf::RenderWindow window;
    openWindow(window, fullscreen);

    //font stuff
    sf::Font font;
    if (!font.loadFromFile("assets/fonts/game.ttf") && !font.loadFromFile("C:/Windows/Fonts/georgia.ttf") && !font.loadFromFile("arial.ttf"))
    {
        std::cout << "Could not load a font. Put any .ttf file at assets/fonts/game.ttf\n";
        return 1;
    }

    int boss = ENEMY_MAD_SCIENTIST;
    startDemoBattle(boss);
    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();

            if (event.type == sf::Event::KeyPressed)
            {
                if (event.key.code == sf::Keyboard::F11)
                {
                    fullscreen = !fullscreen;
                    openWindow(window, fullscreen);
                    break;                                   //restart event loop window created again
                }
                if (handleBattleKey(event.key.code))
                {
                    //moves to the next boss if battle won, if battle is lost it starts the battle again
                    if (getBattleResult() == BATTLE_WON)
                        boss = (boss + 1) % ENEMY_TYPE_COUNT;
                    startDemoBattle(boss);
                }
            }
        }
        drawBattleScreen(window, font);
        window.display();
    }
    return 0;
}
