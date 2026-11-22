# OUROBOROS

A small story-driven exploration and boss-battle game written in **C++ with SFML 2.6.2**.
You play Viktor, lost in a fogged 10x10 ruin: find the relics that piece the story together, survive the
traps, beat the guardians and work out which key really opens the chest.

Programming Fundamenals project, **Group 8** . (FAST NUCES-2026)

**old/ earlier files from before the merge, kept for reference (not part of the build).**

| Part | Who |
|---|---|
| Map, exploring, tiles | Amina (26L-0760) |
| Player, inventory, save data | Zarnib (26L-0769) |
| Combat, battle screen, sound | Dua (26l-0852) |

## Quick start (Windows)

1. Install **MinGW-w64** (g++ and windres, default folder `C:\mingw64`) and **SFML 2.6.2 for MinGW** (default folder `C:\SFML-2.6.2`).
   If yours are somewhere else, set the `MINGW` and `SFML` environment variables or edit the top of `scripts\build_game.bat`.
2. Clone the repo and double-click **`scripts\build_game.bat`** (or run it from the project root in PowerShell: `.\scripts\build_game.bat`).
   It copies the DLLs next to the exe, builds the icon and the game, and starts it.

The game must be started from the **project root** (the folder that contains `assets/`), the build script already does this.
Do not move `ouroboros.exe` away from `assets/`.

## Download ZIP from github

1. Extract into a new folder.
2. Go to scripts, then double click `build_game`.
3. Give the required permissions.
4. Once this runs, the `ouroboros.exe` will be built (outside the sub-folders).
5. You can either run it through the `build_game` (will have to give permission every time), but application is preferred, so simply double clicking the `ouroboros.exe` will run the application (no need to give permission every time).

## Controls

| Where | Keys |
|---|---|
| Menu | Up / Down move, Left / Right change difficulty or screen mode, Enter choose |
| Exploring | WASD or arrows move, **E** interact (pick up, talk, read, open), **I** bag, **Esc** pause |
| Pause | **S** save, **M** quit to menu, Esc resume |
| Bag | arrows move, Enter use, **S** sort A-Z, Esc / I close |
| Battle | Up / Down choose an action, Enter confirm, Left / Right pick a head (Hydra), Esc back |
| Anywhere | **F11** fullscreen. The window can also be resized, the game scales to any screen |

## Features

- Fog of war map with hidden traps and holes, bushes with potions, walls, NPCs and clue tiles
- 7 relics, each with its own picture and story
- 3 boss battles (Mad Scientist, Humanoid Bat, Hydra) with their own music and sounds
- Inventory with potions and keys, sortable with a bubble sort
- Potions: healing works anywhere (fight or bag); buff (+50% damage for 3 rounds) and poison (-15 HP) are for fights only
- 3 difficulties (Hard unlocks after beating the game on Normal)
- Save / Continue, score and high score
- Menu, exploration and battle music, UI sounds

## Project layout

```
OUROBOROS/
├── src/        game source code (.cpp / .h) and the exe icon resource (ouroboros.rc)
├── scripts/    build script (.bat)
├── assets/     everything the game loads: images, sprites, backgrounds, music, sounds, fonts, icon
│   └── source/   original files that are not loaded by the game (e.g. the menu video)
├── docs/       guides: what every file does, every asset and where it goes, changelog
├── README.md
├── .gitignore  .gitattributes
```

Files the game and the build create (`ouroboros.exe`, DLLs, `save.txt`, `highscores.txt`, ...) are listed in `.gitignore`
and are never committed.

## Assets

All the assets the game needs are included. A missing sound or picture never crashes the game. **`docs/ASSET_GUIDE.md`** lists every asset, its exact path, what is still missing
and whether replacing it needs a code change (usually not: keep the file name).

## More docs

- [`docs/CODE_GUIDE.md`](docs/CODE_GUIDE.md) - what every source file does
- [`docs/ASSET_GUIDE.md`](docs/ASSET_GUIDE.md) - every image, sound and music file
- [`docs/CHANGELOG.md`](docs/CHANGELOG.md) - what changed between versions
- [`docs/OLD_README.md`](docs/OLD_README.md) - older implementation logs
## Credits

- Pictures, sprites, backgrounds, menu video, music and most sound effects: AI-generated with ChatGPT.
- Attack / sword slash swoosh sound: Mixkit.
- Hydra wail (monster scream) and pain grunt sounds: UpperBeat.
- Scientist battle theme (`assets/music/battle_0.ogg`): composed with Claude (Anthropic).
- Font: Crimson Text by The Crimson Text Project Authors, SIL Open Font License (`assets/fonts/OFL.txt`).

The same list is on the in-game credits screen (`src/Story.cpp`) and in `docs/ASSET_GUIDE.md` 
