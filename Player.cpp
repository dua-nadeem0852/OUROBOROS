#include "PlayerInterface.h"
#include <fstream>

// --- Core Player Variables ---
static int hp = 100;
static int maxHp = 100;
static int level = 1;
static int atkLevel = 1;
static int maxAtkLevel = 3;
static int expPoints = 0;
static int difficulty = DIFF_NORMAL;

// --- 8-Slot Inventory Variables ---
static int slotId[INVENTORY_SLOTS] = { -1, -1, -1, -1, -1, -1, -1, -1 };
static int slotCount[INVENTORY_SLOTS] = { 0, 0, 0, 0, 0, 0, 0, 0 };

// --- Helper Functions ---
static int findSlot(int id) {
    for (int i = 0; i < INVENTORY_SLOTS; i++) {
        if (slotId[i] == id && slotCount[i] > 0) return i;
    }
    return -1;
}

static int slotsUsed() {
    int count = 0;
    for (int i = 0; i < INVENTORY_SLOTS; i++) {
        if (slotCount[i] > 0) count++;
    }
    return count;
}

// --- Shared Interface Functions (Used by Member 3) ---
int getPlayerHP() { return hp; }
int getPlayerMaxHP() { return maxHp; }
int getplayerAtk() { return 10 + (atkLevel - 1) * 5; }
int getplayerAtkLevel() { return atkLevel; }
int getPlayerMaxAttackLevel() { return maxAtkLevel; }
int getDifficulty() { return difficulty; }

void takeDamage(int amount) {
    hp -= amount;
    if (hp < 0) hp = 0;
}

void healPlayer(int amount) {
    hp += amount;
    if (hp > maxHp) hp = maxHp;
}

void addExperience(int amount) {
    expPoints += amount;
    while (expPoints >= level * 100) {
        expPoints -= level * 100;
        level++;
        maxHp += 20;
        hp = maxHp;
    }
}

void increasePlayerMaxHP(int amount) {
    maxHp += amount;
    hp += amount;
}

void increaseAttackLevel() {
    if (atkLevel < maxAtkLevel) {
        atkLevel++;
    }
}

int getSlotItemId(int slot) {
    if (slot >= 0 && slot < INVENTORY_SLOTS && slotCount[slot] > 0) {
        return slotId[slot];
    }
    return -1;
}

int getSlotItemCount(int slot) {
    if (slot >= 0 && slot < INVENTORY_SLOTS) {
        return slotCount[slot];
    }
    return 0;
}

bool hasItem(int itemId) {
    return findSlot(itemId) >= 0;
}

int getItemCount(int itemId) {
    int s = findSlot(itemId);
    return (s >= 0) ? slotCount[s] : 0;
}

bool useItem(int itemId) {
    int s = findSlot(itemId);
    if (s < 0) return false;

    slotCount[s]--;
    if (slotCount[s] == 0) {
        slotId[s] = -1;
    }

    if (itemId == ITEM_HEALING_POTION) {
        healPlayer(40);
    } else if (itemId == ITEM_POISON_POTION) {
        takeDamage(15);
    }
    return true;
}

bool addItem(int itemId) {
    if (itemId < 0 || itemId >= ITEM_TYPE_COUNT) return false;

    int s = findSlot(itemId);
    if (s >= 0) {
        slotCount[s]++;
        return true;
    }

    if (slotsUsed() >= INVENTORY_SLOTS) return false;

    for (int i = 0; i < INVENTORY_SLOTS; i++) {
        if (slotCount[i] == 0) {
            slotId[i] = itemId;
            slotCount[i] = 1;
            return true;
        }
    }
    return false;
}

std::string getItemName(int itemId) {
    switch (itemId) {
        case ITEM_HEALING_POTION: return "Healing Potion";
        case ITEM_POISON_POTION:  return "Poison Potion";
        case ITEM_BUFF_POTION:    return "Buff Potion";
        case ITEM_RELIC:          return "Relic";
        case ITEM_REAL_KEY:       return "Key";
        case ITEM_DECOY_KEY:      return "Key";
        default:                  return "Empty";
    }
}

// --- Member 2 Additional & Backup Functions ---
int getPlayerExp() { return expPoints; }
int getPlayerLevel() { return level; }
void setDifficulty(int diff) { difficulty = diff; }

void resetPlayerToDefault() {
    hp = 100;
    maxHp = 100;
    level = 1;
    atkLevel = 1;
    expPoints = 0;
    difficulty = DIFF_NORMAL;
    for (int i = 0; i < INVENTORY_SLOTS; i++) {
        slotId[i] = -1;
        slotCount[i] = 0;
    }
    addItem(ITEM_HEALING_POTION);
    addItem(ITEM_HEALING_POTION);
    addItem(ITEM_RELIC);
}

bool saveGameBackup(const std::string& filename) {
    std::ofstream out(filename);
    if (!out.is_open()) return false;

    out << hp << " " << maxHp << " " << level << " "
        << atkLevel << " " << expPoints << " " << difficulty << "\n";

    for (int i = 0; i < INVENTORY_SLOTS; i++) {
        out << slotId[i] << " " << slotCount[i] << "\n";
    }
    out.close();
    return true;
}

bool loadGameBackup(const std::string& filename) {
    std::ifstream in(filename);
    if (!in.is_open()) return false;

    in >> hp >> maxHp >> level >> atkLevel >> expPoints >> difficulty;
    for (int i = 0; i < INVENTORY_SLOTS; i++) {
        in >> slotId[i] >> slotCount[i];
    }
    in.close();
    return true;
}