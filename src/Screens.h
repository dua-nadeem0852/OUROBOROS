//AMINA AHMAD SYED 26L-0760 member 1
//WHAT THIS FILE DOES: lists the drawing functions for the menu, hud, bag, pause, etc
//every screen that is not the map or the battle (menu, hud, bag, text scenes, game over, credits)
//these only DRAW stuff. Game.cpp decides which one to call and handles the keys
//everything is drawn for a 1280 x 720 window

#ifndef SCREENS_H
#define SCREENS_H
#include <SFML/Graphics.hpp>
#include <string>
#include <vector>

const int SCREEN_W = 1280;
const int SCREEN_H = 720;

//main menu. difficulty is DIFF_EASY / DIFF_NORMAL / DIFF_HARD (from PlayerInterface.h)
void drawMainMenu(sf::RenderWindow& window, const sf::Font& font, int selected, int difficulty, bool hardUnlocked, bool canContinue, bool fullscreen, int highScore);

//the panel on the right side of the map (hp, exp, keys, relics, message)
void drawHud(sf::RenderWindow& window, const sf::Font& font, const std::string& message);

//the bag (I key) selected = which of the 8 slots is highlighted
void drawInventory(sf::RenderWindow& window, const sf::Font& font, int selected, const std::string& notice);

//a story/dialogue box over a dark background. used for relics, the intro, the ending
void drawTextScene(sf::RenderWindow& window, const sf::Font& font, const std::string& title, const std::string& text);

void drawPauseMenu(sf::RenderWindow& window, const sf::Font& font, const std::string& notice);   //notice = "Game saved." etc
void drawGameOver(sf::RenderWindow& window, const sf::Font& font);

//scrolls the credits. scroll goes up every frame, returns true once everything has moved off the top
bool drawCredits(sf::RenderWindow& window, const sf::Font& font, float scroll);
#endif