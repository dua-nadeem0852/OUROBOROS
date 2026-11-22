//the first person battle screen
#ifndef BATTLE_SCREEN_H
#define BATTLE_SCREEN_H
#include <SFML/Graphics.hpp>

/*The screen is always 720 units tall. Its WIDTH follows the shape of the window (between 1.4:1 and 2.4:1)
so it fills a laptop screen either window or full screen without stretching
only a window shaped outside that range gets thin black bars*/
const int BATTLE_DESIGN_H = 720;
const int BATTLE_WINDOW_W = 1280;             //a good starting window size (16:9)
const int BATTLE_WINDOW_H = 720;

//draw the whole battle screen. call every frame while the game state is BATTLE. it sets its own view, so it never stretches whatever size the window is

void drawBattleScreen(sf::RenderWindow& window, const sf::Font& font);

//call from event loop on sf::Event::KeyPressed while in BATTLE state.
/* dialog / after the fight: enter/space
menu:  up/down = move the cursor, enter/ space = choose, left/right = pick which hydra head to target
items:  arrows = move, enter = use, esc = back
returns true when battle is finished and the game should leave the battle screen (go to game over if getBattleResult() == BATTLE_LOST).*/

bool handleBattleKey(sf::Keyboard::Key key);
#endif