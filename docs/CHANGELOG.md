# Changelog (development notes)

Older entries mention the flat folder layout. Files are now in `src/`, `scripts/` and `assets/` (see the README).

## First merged version

Run `scripts\build_game.bat` to build and play the whole game.

| File | What it is |
|---|---|
| `main.cpp`, `Game.h/.cpp` | window, main loop, game states (menu, explore, battle, bag, dialogue, game over, credits) |
| `Map.h/.cpp` | the map, walking, traps, holes, bushes, keys, relics, the chest, drawing the tiles |
| `Screens.h/.cpp` | menu, side panel (HUD), bag screen, story box, pause, game over, credits |
| `Story.h/.cpp` | relic stories, intro, ending, credits text (first draft, rewrite it) |

Changed:
- `PlayerInterface.h`: added `PROGRESS_HAS_REAL_KEY`, `PROGRESS_HAS_RELIC`, `PROGRESS_BEAT_GAME` (Player.cpp did not build without them)

Combat files (Battle, Enemy, BattleScreen, BattleSound, UiDraw) are Dua's. Only the key rewards changed in Battle.cpp (scientist = fake key, hydra = real key).

Map notes for Amina:
- the 5 extra `E` tiles were removed, there are 3 bosses so 3 enemy tiles (scientist 2,2 - bat 1,8 - hydra 9,6)
- keys follow the proposal: the key tile, the Mad Scientist and the Bat give fake keys, only the Hydra (final boss) gives the real key, and only the real key opens the chest

## Added in the latest round
- `SaveSystem.h/.cpp` (from the File Handling add-on, fixed): save/load = Zarnib's player save + the map state (`save.txt` and `save.txt.map`), plus the high score (`highscores.txt`). Add-on fixes: missing `#include <string>`, it called functions Player.cpp doesn't have, and it didn't save the level.
- Map: walls `#`, NPCs `N`, clues `?`, extra traps on Hard / fewer on Easy, `everythingReachable()` check so walls can't lock the game, `saveMapState/loadMapState`.
- Game: Continue in the menu, Save (S) in the pause menu, score + high score, level-up message, resizable window that scales to any screen (black bars if the shape differs), Fullscreen/Windowed in the menu and with F11.
- Boss hit sounds: `SND_SCIENTIST_HIT`, `SND_BAT_HIT`, `SND_HYDRA_HIT` play when that boss gets hurt (right after the sword hit sound). Files: `assets/sounds/scientist_hit.wav`, `bat_hit.wav`, `hydra_hit.wav` (placeholder sounds, replace with real ones).
- Bug fix in `Player.cpp`: a save slot with item -1 but count > 0 (or an item with count 0) is now rejected on load.
- Every file now starts with a comment saying what it does.

## Still not done
- relic journal in the bag, optional extra enemies, real art/audio credits, real sound/music files instead of placeholders

## Round 3 (changes from the "TO FIX" PDF + PF concepts)
- **Tile struct** (`Map.h/.cpp`): the map is now `Tile gameMap[10][10]` (type, visited, enemyDefeated, revealed, triggered) instead of 3 separate arrays.
- **Item struct + bubble sort** (`Player.cpp`): a bag slot is an `Item {id, name, quantity}`; `sortInventory()` bubble-sorts the bag A-Z by name (empty slots last). Press **S** in the bag. Also fixed `saveGameBinary`, which would have written the raw struct (a string pointer) once slots became structs.
- **Chest/hydra** (`Map.cpp`, `Game.cpp`): the chest is invisible and not interactable until the Bat's decoy key is in the bag. The Hydra no longer stands on the map at 9,6: pressing E on the chest shows the dialogue, then the fight starts (it never starts by walking onto the chest). The chest is at 5,8. Stale clue text fixed ("the first key you are given is a lie", chest in the far east).
- **Hidden traps and holes**: they look like plain floor until they go off, then their picture stays on the map. A sprung trap does not hurt again. Saved in the map save file (older saves still load).
- **Music**: `playMusicTrack()` in `BattleSound.cpp`. `assets/music/menu.wav` plays in the menu/credits, `assets/music/explore.wav` while exploring; battle music is unchanged. Drop in `menu.ogg` / `explore.ogg` to replace the placeholders (.ogg is tried first).
- **Textures**: `assets/images/floor.png`, `wall.png`, `npc.png`, `clue.png`, `relic.png`, `relic_bag.png` are used if present (coloured squares otherwise). The ones here are placeholders, replace them with your real art using the same names.
- **Relic bag**: all relics are one bag item ("Relic Bag x3") with its own picture in the inventory.
- The old `.exe` files were removed because they were built from the old code. Run `scripts\build_game.bat`.
- UI sounds (`ui_move/select/back`) now also play in the main menu, bag, pause/save, E-interact and text boxes (`Game.cpp`). See `ASSET_GUIDE.md` for every asset, where it goes and what is still missing.
- Relics: each of the 7 relic tiles has its own picture (`assets/images/relic_0..6.png`) and always gives its own story, so the picture matches the description (`RELIC_TILE_...` tables in `Map.cpp`). `assets/images/relic.png` is the fallback.

## Repo layout
- Code moved to `src/`, `.bat` files to `scripts/`, all pictures to `assets/images/` (the old `Images/` folder is gone, code paths updated).
- Added README, `.gitignore`, `.gitattributes`, `docs/`.
- Fixed the build error `src/ouroboros.rc: No such file` (the build script already expected `src/`, the files were still in one flat folder).
- Fixed the inventory pictures (the code looked in `assets/images/` but the pictures were in `Images/`).
- Menu video source moved to `assets/source/mainmenu_bg.mp4` (file name had spaces).
- Added generated placeholder sound effects: `ui_move`, `ui_select`, `ui_back`, `block`, `heal`, `buff`, `laugh`, `screech`, `victory`, `defeat`.
- Review fixes: Viktor now starts with no relics (the bag started with one while the HUD said 0/7); default player name is "Viktor" everywhere (was "Hunter" in `Player.cpp`); removed the unused key tile (`K`) drawing and the old chest tile (`C`) from the map; old saves that still contain `K`/`C` tiles load them as floor (before, they were rejected); removed unused `BATTLE_WINDOW_W/H`.
- Item icons (potions, relic bag, key) are now drawn in the battle item grid too. The drawing code is shared (`drawItemIcon` in `UiDraw.cpp`) with the bag screen. Both keys use the same picture so the real one can't be told apart.
- Map sounds: traps and holes play `hurt.wav`, dying on the map plays `defeat.wav`, drinking a healing potion in the bag plays `heal.wav` (new `playEffect()` in `BattleSound.cpp`).

## Round 4 (potions and docs)
- **Poison potion in a fight** (`Battle.cpp`, `useBattleItem`): the battle log now says "You lose 15 HP" and the HP lost counts as damage taken that turn, so the damage number shows like any other hit. It still plays `hurt.wav`, and if it kills Viktor the fight is lost (defeat sound, game over) as before.
- Potion rules checked: healing works in a fight and in the bag (`heal.wav`); the buff (`buff.wav`) and poison (`hurt.wav`) potions are fights only. In the bag they show "Better to save this one for a fight".
- Docs: `ASSET_GUIDE.md` (potion sounds, how to pick the font), `README.md` (potions, font note), `assets/fonts/README.md` (font tips).
- Font added: `assets/fonts/game.ttf` = Crimson Text Regular (+ `OFL.txt` licence). All characters used in the game text are in the font. Credited in `src/Story.cpp`, the README and the asset guide.
- Credits filled in (`src/Story.cpp`, README, asset guide): AI-generated assets (ChatGPT), Mixkit and UpperBeat sound effects, Crimson Text font.
- Still missing: `battle_0` / `battle_1` music.
