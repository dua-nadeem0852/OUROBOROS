# OUROBOROS

# IMPLEMENTATION
sfml version used: 2.6.2

**(26L-0852) member 3 :**
I made the whole New_Combat folder. (accidentally forgot to submit the whole folder and so there exists some of its files outside the folder, i didn't delete them just in case the TA would ask for why those files are deleted).

So far, all the combat mechanics have been implemented. All that's really left is to make the game look prettier by adding backgrounds etc and sounds. I have implemented a few just to test things out, so they're more of placeholders while some will stay in the game when finalized.

basic combat for the player:
attack, defend, use item, run(decoy)

items:
potions for healing, buffs and poisoning

boss specific mechanics:
the mad scientist basically consumes his own hp to buff his attacks, rng handles that
the humanoid bat can summon other bats for extra damage
the human hydra has a wail mechanic that essentially does more attack, it is triggered every two turns of the hydra. all the heads of hydra cannot perform the same action (heal, attack, defend) in one turn but two can.

exp, rewards etc:
attack levels and other things like reward handling and exp have been implemented as well.

dialogue:
the mad scientist's (boss 1) dialogue has been handled, the other two bosses can't really speak so no dialogue for them.

sound and ui:
i added code to change the pitch sometimes like when the player (viktor) is hurt
and have to just add the other sound effects that will be played when they are called.
implemented a boss theme as well (hydra)

testing:
i have a demo made to test the combat (exe so github wont let me upload it)


File	                                          What it does
Battle.cpp	    Main battle system. Handles player turns, attacks, defending, items, boss turns, rewards and battle outcomes.

Battle.h      	Header for the battle system. Declares battle functions, actions, states and constants.

BattleDemo.cpp	Main program that launches the battle demo and creates the game window.

BattleScreen.cpp	Handles the visual battle screen, including backgrounds, characters, animations, health bars and menus.

BattleScreen.h	  Declares the functions and screen dimensions used by the battle screen.

BattleSound.cpp	 Handles background music and sound effects during battles.

BattleSound.h    Declares the sound and music functions.

Enemy.cpp       Handles enemy statistics, health, damage, special abilities and defeated-boss tracking.

Enemy.h	        Declares enemy types, enemy-related constants and functions.

PlayerInterface.h	   Defines the interface between the battle system and the player's stats and inventory.

PlayerStub.cpp    	Temporary implementation of player statistics and inventory for testing the battle system.

UiDraw.cpp	    Contains reusable drawing functions for text, panels, health bars, sprites and other visual elements.

UiDraw.h	      Declares the drawing helper functions used by UiDraw.cpp.

TestBattle.cpp	 Contains automated tests to check whether the battle system works correctly.

the assets are pretty self explanatory
_________________________________________________________________________________________________________________________________


# PROJECT PROPOSAL
OUROBOROS is a 2D GUI-based exploration and adventure game developed in C++ using the SFML
library. The player explores a mysterious, abandoned world through a tile-based map while searching for
a hidden treasure chest.
The game combines exploration, item collection, enemies, and turn-based combat. Most areas of the
map are initially hidden, and the player gradually reveals them by moving through the environment.
The player initially believes that the objective is simply to find the treasure. However, as the game
progresses, they discover that there is more to this dark world than meets the eye.
Purpose:
The game will incorporate programming concepts covered during the course, including variables,
conditional statements, loops, functions, arrays, 2D arrays etc where applicable and also provide a fun
gaming experience.

Objectives:
The main objectives of the game are:
● Explore the game world and reveal hidden areas.
● Find clues, items, and useful relics.
● Encounter and defeat enemies.
● Gain experience from battles.
● Improve the player's abilities through level ups.
● Find the key required to progress.
● Defeat the final Hydra boss.
● Open the final treasure chest.
● Discover the story and history of the mysterious world through the relics.
Features:
1. 2D GUI Map
The game will use an SFML-based graphical interface with a tile-based map. Each tile can contain
different objects, locations, enemies, traps, or items.

2. Hidden and Revealed Tiles
Most of the map will initially be hidden. As the player moves around the map, tiles will become visible.
3. Player Movement
The player will control the main character using keyboard controls and move through the grid.
4. Exploration
The player can explore different areas of the map to find enemies, clues, items, traps, and important
locations. Even meet npcs.
5. Inventory System
The player will be able to collect and store items such as:
● Keys
● Relics
● Health items
● Special items
● Story-related objects
6. Health and Attack System
The player will have health and attack statistics. Certain encounters and rewards can modify these
statistics.
7. Experience System
Defeating enemies will make the player gain experience (exp) which will increase the player's level,
making their attacks stronger and increase health.
8. Turn-Based Combat
When the player encounters an enemy, the game will enter a battle state. The player will be able to
select actions such as:
● Attack
● Defend
● Use Item
● Run, where applicable
The enemy will respond after the player's turn.
9. Boss Battles

The game will contain several important enemies with different purposes, the player can defeat them
using the turn based combat system in the game.
10. Relics and Story System
The player will discover relics throughout the game. Relics will contain descriptions and information that
gradually reveal the history of the world and the player's connection to it.
11. The Treasure Chest
After defeating the final boss, the player obtains the real key and can finally open the treasure chest. The
contents of the chest provide the final story reveal.
12. Game States
The game will have multiple states, including:
● Main Menu (including difficulty level, start game, exit etc)
● Exploration
● Battle
● Inventory
● Dialogue/Story
● Game Over
● Victory/Ending and then credit scene
13. Difficulty levels
The player first has to beat the game on the default difficulty level (normal) to unlock harder difficulty
levels. The higher the difficulty level, the more the obstacles the player encounters.
14. Controls
Basic AWSD moving mechanics, E to interact, Enter for progressing the dialogue boxes, I for inventory,
Esc for pausing etc. During battles, the player will use the designated keyboard keys to select combat
actions.

Game Rules:
1. The player must remain within the boundaries of the game map.
2. Hidden tiles are revealed when the player explores them.
3. Items are collected by interacting with their corresponding tiles.
4. Enemy encounters initiate the battle state.
5. The player loses health when receiving damage.
6. If the player's health reaches zero, the game enters the Game Over state.
7. Optional enemies can be defeated to gain experience and upgrades.
8. Find the key to the chest by exploration and defeating enemies.

9. Opening the chest triggers the final story sequence.

Technologies Used:
Programming Language: C++
Graphics/Game Library: SFML
IDE: Visual Studio Code/Visual Studio/VS code
Compiler: MinGW g++
Graphics: 2D sprites and tile-based graphical assets.
Others: Music and sound effects from external sources
Data Storage: Text files for appropriate saved information such as high scores or game data

Installation and Setup:
● Install a C++ compiler such as MinGW.
● Install Visual Studio Code or another suitable C++ development environment.
● Install and configure the SFML library.
● Download or open the project folder.
● Ensure that the required SFML libraries and asset folders are located in the correct directories.
● Compile the C++ source files using the configured compiler.
● Run the resulting executable file.
● The game will open in an SFML graphical window.

Usage:
When the game starts, the player will be presented with the main menu.
After selecting Start Game, the player will enter the exploration map.
The player uses the keyboard to move around the map. Hidden tiles are revealed as the player explores.
Depending on the tile, the player may find an item, trigger a trap, discover a clue, or encounter an
enemy.
When an enemy is encountered, the game changes from the Exploration State to the Battle State. The
player selects an action and the enemy responds.
The player can return to exploration after winning a battle.
The player can also access their inventory to view collected items and relics.
Eventually, the player progresses toward the final encounter with the last boss and has to defeat it.
The game then enters the Ending State, where the collected relics reveal the final story.
