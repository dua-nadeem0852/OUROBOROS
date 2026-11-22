# Code guide - what every file does

Everything is in `src/` unless the table says otherwise. The game is split by job so each teammate can work in their own files.
Every file also starts with a comment saying what it does.

## Game loop and screens

| File | What it does |
|---|---|
| `main.cpp` | starts the game, calls `runGame()` |
| `Game.h/.cpp` | window (resizable, F11 fullscreen), main loop, game states (menu, explore, battle, bag, dialogue, game over, credits), keys, score, music, UI sounds |
| `Screens.h/.cpp` | draws the menu (with the video background), side panel, bag, story box, pause menu, game over, credits. Drawing only |
| `Story.h/.cpp` | all story text: intro, ending, relic stories, credits |
| `UiDraw.h/.cpp` | small drawing helpers (text, panels, bars) and the picture loader/cache |

## Map and player (Amina, Zarnib)

| File | What it does |
|---|---|
| `Map.h/.cpp` | the 10x10 map as a `Tile` array, walking, walls, hidden traps and holes, bushes, relics, NPCs, clues, the chest, saving/loading the map, drawing the tiles |
| `PlayerInterface.h` | the list of player/bag functions everyone calls (and item / difficulty constants) |
| `Player.cpp` | hp, level, exp, difficulty, the bag (`Item` struct, bubble sort), and saving/loading those |
| `SaveSystem.h/.cpp` | `saveGame` / `loadGame` (player + map) and the high score file |

## Combat (Dua)

| File | What it does |
|---|---|
| `Battle.h/.cpp` | the combat rules: turns, damage, boss attacks, rewards, the sound queue. No drawing |
| `Enemy.h/.cpp` | boss stats and behaviour, remembers which bosses are beaten |
| `BattleScreen.h/.cpp` | draws and controls the battle screen. Rules stay in `Battle.cpp` |
| `BattleSound.h/.cpp` | sound effects, UI sounds and music (battle, menu, exploration) |

## Build files

| File | What it does |
|---|---|
| `src/ouroboros.rc` | gives `ouroboros.exe` its icon (`assets/icon.ico`) |
| `scripts/build_game.bat` | builds and runs the game |
| `scripts/run_tests.bat`, `scripts/run_map_test.bat` | build and run the tests |

## Tests (`tests/`)

| File | What it does |
|---|---|
| `TestBattle.cpp` | combat rules, with the fake player below |
| `PlayerStub.cpp` | a fake player used **only** by `TestBattle`. Not part of the game |
| `TestMap.cpp` | map, walls, traps, chest, save/load, high score, bag sorting, using the real `Player.cpp` |

## Who calls whom

```
main -> Game -> Map, Screens, Story, BattleScreen, BattleSound, SaveSystem
Map, Battle, BattleScreen, Screens, SaveSystem -> PlayerInterface (Player.cpp)
BattleScreen -> Battle, Enemy, UiDraw
```
The map never fights and the battle never knows about the map: `Game.cpp` is the only file that connects them.

## Run-time files (not in git)

`save.txt` + `save.txt.map` (the saved game), `highscores.txt`, `unlocked.txt` (Hard mode unlocked). They are created
in the folder the game is started from, the project root.
