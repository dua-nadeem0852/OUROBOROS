
// This file only DRAWS things. All the calculations are in player.cpp.


#include "inventory_sfml.h"
#include "PlayerInterface.h"

static sf::Font font;                    // one font for all the text
static sf::Clock messageClock;           // how long the bottom message has been visible
static std::string message = "";         // text of the bottom message box
static std::string inventoryNote = "";   // feedback line inside the inventory screen
static int selectedSlot = 0;             // highlighted inventory slot (0 to 7)

// Colours (dark gothic style)
static const sf::Color COL_BOX(20, 16, 22);
static const sf::Color COL_BORDER(110, 82, 48);
static const sf::Color COL_TEXT(222, 208, 172);
static const sf::Color COL_DIM(130, 120, 105);
static const sf::Color COL_GOLD(244, 204, 92);

// colour square stands in for each item picture (same order as the ITEM_ numbers).
static const sf::Color itemColor[ITEM_TYPE_COUNT] = {
    sf::Color(200, 50, 60),    // healing potion
    sf::Color(90, 190, 80),    // poison potion
    sf::Color(120, 90, 210),   // buff potion
    sf::Color(230, 225, 210),  // relic
    sf::Color(212, 170, 60),   // real key
    sf::Color(212, 170, 60)    // decoy key (looks the same on purpose)
};


//  Two small helpers used everywhere ...

static void drawText(sf::RenderWindow& win, const std::string& str, float x, float y,
                     unsigned int size, sf::Color color) {
    sf::Text t;
    t.setFont(font);
    t.setString(str);
    t.setCharacterSize(size);
    t.setFillColor(color);
    t.setPosition(x, y);
    win.draw(t);
}

static void drawBox(sf::RenderWindow& win, float x, float y, float w, float h,
                    sf::Color fill, sf::Color border) {
    sf::RectangleShape box(sf::Vector2f(w, h));
    box.setPosition(x, y);
    box.setFillColor(fill);
    box.setOutlineThickness(-2);          // negative = border drawn inside the box
    box.setOutlineColor(border);
    win.draw(box);
}

// A bar filled by "fraction" (0.0 to 1.0) with a text label on top
static void drawBar(sf::RenderWindow& win, float x, float y, float w, float h,
                    float fraction, sf::Color color, const std::string& label) {
    drawBox(win, x, y, w, h, sf::Color(10, 8, 12), COL_BORDER);
    if (fraction > 1) fraction = 1;
    if (fraction > 0) drawBox(win, x, y, w * fraction, h, color, color);
    drawText(win, label, x + 8, y + 1, 14, COL_TEXT);
}


//  Setup and messages 

bool initPlayerUI(const std::string& fontPath) {
    if (font.loadFromFile(fontPath)) {
        return true;
    }
    return font.loadFromFile("C:/Windows/Fonts/arial.ttf");   // Windows fallback
}

void setMessage(const std::string& text) {
    message = text;
    messageClock.restart();
}

// caled by member 1 when the player walks onto an item tile.../..
bool pickUpItem(int itemId) {
    if (addItem(itemId)) {
        setMessage("You picked up: " + getItemName(itemId));
        return true;
    }
    setMessage("Your bag is full!");
    return false;
}

// relicNumber is 1, 2 or 3
bool collectRelic(int relicNumber) {
    const unsigned int flags[3] = { PROGRESS_RELIC_1, PROGRESS_RELIC_2, PROGRESS_RELIC_3 };
    const char* names[3] = { "The Beginning", "The Experiment", "The Lost World" };

    if (relicNumber < 1 || relicNumber > 3 || hasProgressFlag(flags[relicNumber - 1])) {
        return false;                                  // wrong number, or already found
    }
    if (!addItem(ITEM_RELIC)) {
        setMessage("Your bag is full!");
        return false;
    }
    setProgressFlag(flags[relicNumber - 1]);
    setMessage(std::string("Relic found: ") + names[relicNumber - 1]);
    return true;
}

// called by member 3 after a battle
void gainExperience(int amount) {
    int levelBefore = getPlayerLevel();
    addExperience(amount);
    if (getPlayerLevel() > levelBefore) {
        setMessage("LEVEL UP! You are now level " + std::to_string(getPlayerLevel()));
    }
    else {
        setMessage("You gained " + std::to_string(amount) + " EXP");
    }
}


// HUD and message box 

void drawHUD(sf::RenderWindow& win) {
    drawBox(win, 12, 12, 280, 124, COL_BOX, COL_BORDER);
    drawText(win, std::string(getPlayerName()) + "   Lv " + std::to_string(getPlayerLevel()),
             28, 18, 20, COL_GOLD);

    // HP bar turns red when HP is under 30%
    float hpFraction = (float)getPlayerHP() / getPlayerMaxHP();
    sf::Color hpColor = (hpFraction < 0.3f) ? sf::Color(190, 52, 52) : sf::Color(78, 170, 80);
    drawBar(win, 28, 48, 248, 20, hpFraction, hpColor,
            "HP " + std::to_string(getPlayerHP()) + " / " + std::to_string(getPlayerMaxHP()));

    drawBar(win, 28, 72, 248, 20, (float)getPlayerExp() / getExpNeededForNextLevel(),
            sf::Color(96, 108, 210),
            "EXP " + std::to_string(getPlayerExp()) + " / " + std::to_string(getExpNeededForNextLevel()));

    drawText(win, "ATK " + std::to_string(getplayerAtk()) +
                  "  (Lv " + std::to_string(getplayerAtkLevel()) + ")", 28, 98, 15, COL_TEXT);
}

void drawMessageBox(sf::RenderWindow& win) {
    drawBox(win, 12, 576, 936, 52, COL_BOX, COL_BORDER);
    if (messageClock.getElapsedTime().asSeconds() < 4) {       // show for 4 seconds
        drawText(win, message, 30, 590, 20, COL_TEXT);
    }
    drawText(win, "[I] Inventory", 820, 592, 16, COL_DIM);
}


// - Inventory screen ../,/,/.

void openInventory() {
    selectedSlot = 0;
    inventoryNote = "";
}

// Uses the highlighted item and writes what happened into inventoryNote
static void useSelectedItem() {
    int id = getSlotItemId(selectedSlot);

    if (id == -1) {
        inventoryNote = "That slot is empty.";
    }
    else if (id == ITEM_RELIC || id == ITEM_REAL_KEY || id == ITEM_DECOY_KEY) {
        inventoryNote = "This can't be used.";
    }
    else if (useItem(id)) {                 // player.cpp does the real work
        inventoryNote = "You used: " + getItemName(id);
    }
    else {
        inventoryNote = "You can't use that right now.";   // e.g. HP already full
    }
}

// Returns true when the player closes the inventory
bool handleInventoryEvent(const sf::Event& e) {
    if (e.type != sf::Event::KeyPressed) {
        return false;
    }
    inventoryNote = "";                     // clear the old note on every key press

    switch (e.key.code) {
    case sf::Keyboard::Left:   if (selectedSlot > 0)  selectedSlot--;     break;
    case sf::Keyboard::Right:  if (selectedSlot < 7)  selectedSlot++;     break;
    case sf::Keyboard::Up:     if (selectedSlot >= 4) selectedSlot -= 4;  break;
    case sf::Keyboard::Down:   if (selectedSlot < 4)  selectedSlot += 4;  break;
    case sf::Keyboard::Return: useSelectedItem();                         break;
    case sf::Keyboard::S:      sortInventory(false); inventoryNote = "Sorted by item type."; break;
    case sf::Keyboard::C:      sortInventory(true);  inventoryNote = "Sorted by amount.";    break;
    case sf::Keyboard::I:
    case sf::Keyboard::Escape: return true;
    default: break;
    }
    return false;
}

void drawInventory(sf::RenderWindow& win) {
    // darken the game behind the inventory
    sf::RectangleShape dim(sf::Vector2f(960, 640));
    dim.setFillColor(sf::Color(0, 0, 0, 170));
    win.draw(dim);

    drawBox(win, 160, 60, 640, 520, COL_BOX, COL_BORDER);
    drawText(win, "INVENTORY", 180, 72, 28, COL_GOLD);

    // 8 slots: 4 per row, 2 rows
    for (int i = 0; i < INVENTORY_SLOTS; i++) {
        float x = 177 + (i % 4) * 155;
        float y = 120 + (i / 4) * 125;
        drawBox(win, x, y, 140, 110, sf::Color(30, 24, 32), (i == selectedSlot) ? COL_GOLD : COL_BORDER);

        int id = getSlotItemId(i);
        if (id != -1) {
            drawBox(win, x + 45, y + 10, 50, 50, itemColor[id], COL_TEXT);          // item "picture"
            drawText(win, getItemName(id), x + 8, y + 68, 14, COL_TEXT);
            drawText(win, "x" + std::to_string(getSlotItemCount(i)), x + 8, y + 88, 14, COL_GOLD);
        }
    }

    // description of the highlighted item
    drawBox(win, 177, 375, 605, 100, COL_BOX, COL_BORDER);
    int id = getSlotItemId(selectedSlot);
    if (id == -1) {
        drawText(win, "Empty slot", 192, 385, 20, COL_DIM);
    }
    else {
        drawText(win, getItemName(id), 192, 385, 20, COL_GOLD);
        drawText(win, getItemDescription(id), 192, 418, 16, COL_TEXT);
    }

    drawText(win, inventoryNote, 177, 490, 16, COL_TEXT);
    drawText(win, "Arrows: Select  Enter: Use  S: Sort type  C: Sort amount  I/Esc: Close",
             177, 545, 14, COL_DIM);
}
