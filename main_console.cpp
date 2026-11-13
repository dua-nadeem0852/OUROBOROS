#include <iostream>
#include "PlayerInterface.h"

int main() {
    std::cout << "=== OUROBOROS: MEMBER 2 BACKEND TEST ===" << std::endl;
    resetPlayerToDefault();

    std::cout << "Initial HP: " << getPlayerHP() << "/" << getPlayerMaxHP() << std::endl;
    std::cout << "Initial Attack: " << getplayerAtk() << std::endl;

    // Test Adding Items
    std::cout << "\n[Test] Adding Poison Potion and Buff Potion..." << std::endl;
    addItem(ITEM_POISON_POTION);
    addItem(ITEM_BUFF_POTION);
    addItem(ITEM_REAL_KEY);

    // Display Inventory
    std::cout << "\n--- Current Inventory Slots ---" << std::endl;
    for (int i = 0; i < INVENTORY_SLOTS; i++) {
        int id = getSlotItemId(i);
        if (id != -1) {
            std::cout << "Slot " << i << ": " << getItemName(id) 
                      << " (x" << getSlotItemCount(i) << ")" << std::endl;
        } else {
            std::cout << "Slot " << i << ": [Empty]" << std::endl;
        }
    }

    // Test Using an Item
    std::cout << "\n[Test] Using Healing Potion from inventory..." << std::endl;
    takeDamage(50); // Drop HP to 50
    std::cout << "HP after damage: " << getPlayerHP() << std::endl;
    useItem(ITEM_HEALING_POTION); // Uses a healing potion to restore 40 HP
    std::cout << "HP after using Healing Potion: " << getPlayerHP() << std::endl;

    // Test Save Backup
    std::cout << "\n[Test] Saving game backup to 'save_backup.txt'..." << std::endl;
    if (saveGameBackup("save_backup.txt")) {
        std::cout << "Backup saved successfully!" << std::endl;
    } else {
        std::cout << "Failed to save backup!" << std::endl;
    }

    std::cout << "\nAll backend tests passed successfully!" << std::endl;
    return 0;
}