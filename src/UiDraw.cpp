//DUA NADEEM 26L-0852 member 3
//UiDraw.cpp - WHAT THIS FILE DOES: draws text, panels, bars and sprites, and loads/caches the pictures
//small drawing helpers (see UiDraw.h)
#include "UiDraw.h"
#include "PlayerInterface.h"
#include <algorithm>
#include <map>

sf::Texture* loadTexture(const std::string& path, bool smooth)
{
    static std::map<std::string, sf::Texture*> cache;
    std::map<std::string, sf::Texture*>::iterator it = cache.find(path);
    if (it != cache.end()) return it->second;

    sf::Texture* tex = new sf::Texture();
    if (!tex->loadFromFile(path)) { delete tex; tex = 0; }
    else if (smooth) { tex->setSmooth(true); tex->generateMipmap(); }
    cache[path] = tex;
    return tex;
}

//one icon file per item id (0 = no icon). both keys use the same picture on purpose:
//the player must not be able to tell the real key from the decoy by looking at it
static const char* ITEM_ICON[ITEM_TYPE_COUNT] = {
    "assets/images/potion_healing.png", "assets/images/potion_poison.png", "assets/images/potion_buff.png",
    "assets/images/relic_bag.png", "assets/images/key.png", "assets/images/key.png"
};

void drawItemIcon(sf::RenderWindow& window, int itemId, float centerX, float centerY, float box)
{
    if (itemId < 0 || itemId >= ITEM_TYPE_COUNT || !ITEM_ICON[itemId]) return;
    sf::Texture* icon = loadTexture(ITEM_ICON[itemId]);
    if (!icon) return;

    //the icon files are cropped to the item itself, so the item fills the box
    float scale = std::min(box / icon->getSize().x, box / icon->getSize().y);
    sf::Sprite s(*icon);
    s.setScale(scale, scale);
    s.setPosition(centerX - icon->getSize().x * scale / 2.f, centerY - icon->getSize().y * scale / 2.f);
    window.draw(s);
}

void drawText(sf::RenderWindow& window, const sf::Font& font, const std::string& text, unsigned size, float x, float y, sf::Color color)
{
    sf::Text t(text, font, size);
    t.setFillColor(color);
    t.setPosition(x, y);
    window.draw(t);
}

void drawCenteredText(sf::RenderWindow& window, const sf::Font& font, const std::string& text, unsigned size, float centerX, float y, sf::Color color)
{
    sf::Text t(text, font, size);
    t.setFillColor(color);
    t.setPosition(centerX - t.getLocalBounds().width / 2.f, y);
    window.draw(t);
}

void drawPanel(sf::RenderWindow& window, float x, float y, float w, float h, float fade)
{
    sf::RectangleShape outer(sf::Vector2f(w, h));
    outer.setPosition(x, y);
    outer.setFillColor(sf::Color(10, 8, 14, (sf::Uint8)(230.f * fade)));
    outer.setOutlineColor(sf::Color(120, 96, 72, (sf::Uint8)(255.f * fade)));
    outer.setOutlineThickness(2.f);
    window.draw(outer);

    sf::RectangleShape inner(sf::Vector2f(w - 8.f, h - 8.f));
    inner.setPosition(x + 4.f, y + 4.f);
    inner.setFillColor(sf::Color::Transparent);
    inner.setOutlineColor(sf::Color(58, 44, 42, (sf::Uint8)(255.f * fade)));
    inner.setOutlineThickness(1.f);
    window.draw(inner);
}

void drawBar(sf::RenderWindow& window, float x, float y, float w, float h, int value, int maxValue, sf::Color fill, float fade)
{
    sf::RectangleShape back(sf::Vector2f(w, h));
    back.setPosition(x, y);
    back.setFillColor(sf::Color(30, 26, 32, (sf::Uint8)(255.f * fade)));
    back.setOutlineColor(sf::Color(90, 80, 75, (sf::Uint8)(255.f * fade)));
    back.setOutlineThickness(1.f);
    window.draw(back);

    float ratio = (maxValue > 0) ? (float)value / (float)maxValue : 0.f;
    if (ratio < 0.f) ratio = 0.f;
    if (ratio > 1.f) ratio = 1.f;
    sf::RectangleShape front(sf::Vector2f(w * ratio, h));

    front.setPosition(x, y);
    fill.a = (sf::Uint8)(fill.a * fade);
    front.setFillColor(fill);
    window.draw(front);
}

void drawTriangle(sf::RenderWindow& window, float x, float y, float size, bool pointDown, sf::Color color)
{
    sf::ConvexShape tri(3);
    if (pointDown)
    {
        tri.setPoint(0, sf::Vector2f(0.f, 0.f));
        tri.setPoint(1, sf::Vector2f(size, 0.f));
        tri.setPoint(2, sf::Vector2f(size / 2.f, size));
    }
    else   //menu cursor points right
    {
        tri.setPoint(0, sf::Vector2f(0.f, 0.f));
        tri.setPoint(1, sf::Vector2f(size, size / 2.f));
        tri.setPoint(2, sf::Vector2f(0.f, size));
    }
    tri.setPosition(x, y);
    tri.setFillColor(color);
    window.draw(tri);
}

void drawSprite(sf::RenderWindow& window, sf::Texture* tex, float centerX, float feetY, float height, sf::Color tint, float stretchY, float rotation,
float& left, float& top, float& w, float& h)
{
    sf::Vector2u size = tex->getSize();
    float scale = height / (float)size.y;
    w = size.x * scale;
    h = height * stretchY;
    left = centerX - w / 2.f;
    top = feetY - h;

    sf::Sprite sprite(*tex);
    sprite.setOrigin(size.x / 2.f, (float)size.y);      //pivot feet
    sprite.setPosition(centerX, feetY);
    sprite.setScale(scale, scale * stretchY);
    sprite.setRotation(rotation);
    sprite.setColor(tint);
    window.draw(sprite);
}

std::vector<std::string> wrapText(const sf::Font& font, const std::string& text, unsigned size, float maxWidth)
{
    std::vector<std::string> lines;
    std::string current, word;
    for (size_t i = 0; i <= text.size(); i++)
    {
        if (i < text.size() && text[i] != ' ') { word += text[i]; continue; }
        if (word.empty()) continue;

        std::string attempt = current.empty() ? word : current + " " + word;
        sf::Text t(attempt, font, size);
        if (t.getLocalBounds().width > maxWidth && !current.empty()) { lines.push_back(current); current = word; }
        else current = attempt;
        word.clear();
    }
    if (!current.empty()) lines.push_back(current);
    return lines;
}

void drawWrapped(sf::RenderWindow& window, const sf::Font& font, const std::vector<std::string>& paragraphs, float x, float y, float width, int maxLines, unsigned size)
{
    std::vector<std::string> lines;
    for (size_t p = 0; p < paragraphs.size(); p++)
    {
        std::vector<std::string> part = wrapText(font, paragraphs[p], size, width);
        lines.insert(lines.end(), part.begin(), part.end());
    }
    int first = ((int)lines.size() > maxLines) ? (int)lines.size() - maxLines : 0;
    for (int i = first; i < (int)lines.size(); i++) drawText(window, font, lines[i], size, x, y + (i - first) * (size + 7.f));
}
