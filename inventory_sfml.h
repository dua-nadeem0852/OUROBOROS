// PlayerUI.h  (Member 2 - SFML part)
// Everything Member 2 draws: HUD, message box, inventory screen.
// The calculations stay in player.cpp. This file only READS the numbers and draws them.
// Written for SFML 2.6.2.

#ifndef PLAYER_UI_H
#define PLAYER_UI_H

#include <SFML/Graphics.hpp>
#include <string>

// Call ONCE after creating the window. Loads the font and optional item pictures.
// Returns false if no font could be loaded.
bool initPlayerUI(const std::string& fontPath = "assets/fonts/game_font.ttf");

// ----- Helpers Member 1 and Member 3 can call (they also show a message) -----
void setMessage(const std::string& text);   // shows text in the message box for 4 seconds
bool pickUpItem(int itemId);                // addItem + message ("You picked up: ...")
bool collectRelic(int relicNumber);         // relicNumber 1, 2 or 3
void gainExperience(int amount);            // addExperience + "LEVEL UP" message

// ----- Drawing (call these from the main loop, depending on the game state) -----
void drawHUD(sf::RenderWindow& window);          // HP / EXP / level / attack (top-left)
void drawMessageBox(sf::RenderWindow& window);   // bottom message bar (exploration)

void openInventory();                            // call when switching to INVENTORY state
void drawInventory(sf::RenderWindow& window);
bool handleInventoryEvent(const sf::Event& event);   // returns true when the player closes it

#endif
