//small drawing helpers used by the battle screen
//it only draws text, panels, bars, sprites
#ifndef UI_DRAW_H
#define UI_DRAW_H

#include <SFML/Graphics.hpp>
#include <string>
#include <vector>

//colours
static const sf::Color TEXT_COLOR(225, 215, 195);          //parchment
static const sf::Color GOLD(235, 200, 110);
static const sf::Color DIM_COLOR(120, 112, 105);

//loads a picture once and returns 0 if the file is missing
//smooth = true keeps big art looking clean even when it is drawn smaller
sf::Texture* loadTexture(const std::string& path, bool smooth = false);

void drawText(sf::RenderWindow& window, const sf::Font& font, const std::string& text, unsigned size, float x, float y, sf::Color color = TEXT_COLOR);
void drawCenteredText(sf::RenderWindow& window, const sf::Font& font, const std::string& text, unsigned size, float centerX, float y, sf::Color color = TEXT_COLOR);

void drawPanel(sf::RenderWindow& window, float x, float y, float w, float h);     //double frame
void drawBar(sf::RenderWindow& window, float x, float y, float w, float h, int value, int maxValue, sf::Color fill);
void drawTriangle(sf::RenderWindow& window, float x, float y, float size, bool pointDown, sf::Color color);

//draws a sprite standing on the floor at (centerX, feetY), scaled to 'height'
//tint = colour overlay (eg: red when hit), stretchY = squash/stretch, rotation = tilt in degrees
//left, top, w, h are output values: the box the sprite covers on screen which is used to place health bars and damage numbers
void drawSprite(sf::RenderWindow& window, sf::Texture* tex, float centerX, float feetY, float height, sf::Color tint, float stretchY, float rotation,
float& left, float& top, float& w, float& h);

//text that wraps inside a box
//if it is too long the last lines are shown
void drawWrapped(sf::RenderWindow& window, const sf::Font& font, const std::vector<std::string>& paragraphs,
 float x, float y, float width, int maxLines, unsigned size = 17);
std::vector<std::string> wrapText(const sf::Font& font, const std::string& text, unsigned size, float maxWidth);
#endif