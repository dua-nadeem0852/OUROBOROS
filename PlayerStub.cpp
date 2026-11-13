//only for testing! temp stand in for inventory code
// note to self: delete this file from the project once zarnibs code is integrated.
#include "PlayerInterface.h"

static int hp = 100, maxHp = 100, level = 1, diff = DIFF_NORMAL, exp_ = 0;

static int capacity = INVENTORY_SLOTS;      //inventory slots to be used
static int slotId[INVENTORY_SLOTS] = { ITEM_HEALING_POTION, ITEM_BUFF_POTION, -1, -1, -1, -1, -1, -1 };
static int slotCount[INVENTORY_SLOTS] = { 2, 1, 0, 0, 0, 0, 0, 0 };

int getPlayerHP() { return hp; }
int getPlayerMaxHP() { return maxHp; }
int getplayerAtk() { return 10 + (level - 1) * 5; }   //10,15,20
int getplayerAtkLevel() { return level; }
int getPlayerMaxAttackLevel() { return 3; }
int getDifficulty() { return diff; }

void takeDamage(int a) { hp -= a; if (hp < 0) hp = 0; }
void healPlayer(int a) { hp += a; if (hp > maxHp) hp = maxHp; }
void addExperience(int a) { exp_ += a; }
void increasePlayerMaxHP(int a) { maxHp += a; }
void increaseAttackLevel() { if (level < 3) level++; }

static int findSlot(int id)
{
    for (int i = 0; i < INVENTORY_SLOTS; i++) if (slotId[i] == id && slotCount[i] > 0) return i;
    return -1;
}

static int slotsUsed()
{
    int n = 0;
    for (int i = 0; i < INVENTORY_SLOTS; i++) if (slotCount[i] > 0) n++;
    return n;
}

int getSlotItemId(int slot) { return (slot >= 0 && slot < INVENTORY_SLOTS && slotCount[slot] > 0) ? slotId[slot] : -1; }
int getSlotItemCount(int slot) { return (slot >= 0 && slot < INVENTORY_SLOTS) ? slotCount[slot] : 0; }

bool hasItem(int id) { return findSlot(id) >= 0; }
int  getItemCount(int id) { int s = findSlot(id); return s >= 0 ? slotCount[s] : 0; }

bool useItem(int id)
{
    int s = findSlot(id);
    if (s < 0) return false;
    slotCount[s]--;
    if (slotCount[s] == 0) slotId[s] = -1;
    if (id == ITEM_HEALING_POTION) healPlayer(40);
    if (id == ITEM_POISON_POTION)  takeDamage(15);
    return true;      
}

bool addItem(int id)
{
    if (id < 0 || id >= ITEM_TYPE_COUNT) return false;
    int s = findSlot(id);
    if (s >= 0) { slotCount[s]++; return true; }          //stack copies
    if (slotsUsed() >= capacity) return false;            //inventory is full
    for (int i = 0; i < INVENTORY_SLOTS; i++)
    if (slotCount[i] == 0) { slotId[i] = id; slotCount[i] = 1; return true; }
    return false;
}

std::string getItemName(int id)
{
    switch (id)
    {
    case ITEM_HEALING_POTION: return "Healing Potion";
    case ITEM_POISON_POTION: return "Poison Potion";
    case ITEM_BUFF_POTION: return "Buff Potion";
    case ITEM_RELIC: return "Relic";
    case ITEM_REAL_KEY: return "Key";
    case ITEM_DECOY_KEY: return "Key";
    }
    return "?";     //"unknown" for non void functions
}

//some stuff to help me test not actually a part of interface
void stubSetHP(int v)  { hp = v; }
void stubSetDifficulty(int d) { diff = d; }
void stubSetCapacity(int c)  { capacity = c; }
int stubExp()  { return exp_; }
int stubCount(int id) { return getItemCount(id); }
static void clearInventory()
{
    for (int i = 0; i < INVENTORY_SLOTS; i++) { slotId[i] = -1; slotCount[i] = 0; }
}
void stubSetCount(int id, int n)      //put exactly n copies of an item in the pack
{
    int s = findSlot(id);
    if (s >= 0) { slotCount[s] = 0; slotId[s] = -1; }
    for (int k = 0; k < n; k++) addItem(id);
}
void stubSetPlayer(int maxHP, int atkLevel, int healing, int buff, int poison)
{
    maxHp = maxHP; hp = maxHP; level = atkLevel; exp_ = 0;
    capacity = INVENTORY_SLOTS;
    clearInventory();
    stubSetCount(ITEM_HEALING_POTION, healing);
    stubSetCount(ITEM_BUFF_POTION, buff);
    stubSetCount(ITEM_POISON_POTION, poison);
}
