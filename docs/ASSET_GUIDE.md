# Asset guide

Everything the game loads from disk, where it goes, what format it should be, and whether you need to touch the code.
All paths are relative to the **project root** (the folder with `assets/`, `src/`, `scripts/`).

**Rules**
- A missing file never crashes the game: missing pictures become coloured squares/letters, missing sounds and music are silent. The one exception is the font (section 7).
- **To replace a file keep the exact file name** and overwrite it: no code change, no rebuild.
- Sounds/music: `.ogg`, `.wav` or `.flac`. **Not `.mp3`** (SFML 2 can't play it). Pictures: `.png` (transparency works).
- **Use exactly these lower-case names.** Windows ignores capital letters but Linux/macOS (and GitHub tooling) do not.
- Status: **OK** = real file in the repo, **MISSING** = nothing there yet (the game stays silent / uses a fallback).

Layout:
```
assets/
├── images/        map tiles, relics, potion icons, character
├── sprites/       battle pictures (bosses, Viktor)
├── backgrounds/   battle backgrounds + bg_mainmenu/ (menu video frames)
├── music/         menu, exploration, battle songs
├── sounds/        sound effects
├── fonts/         game.ttf
├── source/        originals not loaded by the game (menu video .mp4)
└── icon.ico, icon.png
```

---

## 1. Map and inventory pictures - `assets/images/`

Drawn 64x64 on the map. Any size works, but the picture is **squeezed to a square**, so make them square.

| File | What it is | Status |
|---|---|---|
| `floor.png` | ground under every revealed tile (should tile without seams) | OK |
| `wall.png` | wall (`W`), fills the whole tile | OK |
| `npc.png` | NPC (`N`), transparent background | OK |
| `clue.png` | clue / sign (`?`), transparent background | OK |
| `bush.png` | bush (`B`) | OK |
| `trap.png` | trap. **Only shown after it went off** (traps are hidden until stepped on) | OK |
| `hole.png` | hole. **Only shown after the player fell in** | OK |
| `treasurechest.png` | the chest, only appears once a key is found | OK |
| `mist.png` | fog on unseen tiles | OK |
| `character.png` | the player on the map (transparent PNG works) | OK |
| `key.png` | the key icon in the bag and the battle item grid (the real key and the decoy key use the **same** picture on purpose) | OK |
| `relic.png` | shared fallback if a `relic_N.png` is missing | OK |
| `relic_0.png` ... `relic_6.png` | one picture per relic, matching its story (table below) | OK |
| `relic_bag.png` | the "Relic Bag" icon in the bag and the battle item grid (all relics are one bag item) | OK |
| `potion_healing.png`, `potion_poison.png`, `potion_buff.png` | potion icons in the bag and the battle item grid | OK |

**Relic pictures and stories.** Every relic always gives the same story and uses the same picture, whatever order they are found in:

| File | Story (`Story.cpp`) | Where (row, col) |
|---|---|---|
| `relic_0.png` | A Tarnished Locket | 2,4 (close to the start) |
| `relic_1.png` | A Torn Notebook Page (the Scientist's) | 3,1 (beside the Scientist) |
| `relic_2.png` | A Child's Drawing (three heads = the Hydra) | 5,7 (beside the chest) |
| `relic_3.png` | A Bat-Shaped Brooch | 2,7 (beside the Bat) |
| `relic_4.png` | A Water-Stained Ledger | 3,6 |
| `relic_5.png` | An Empty Key Mould | 8,4 |
| `relic_6.png` | A Warm Stone (the last one) | 8,9 (far corner) |

To move a relic or change which story it gets, edit the `RELIC_TILE_...` tables at the top of `src/Map.cpp`.
If you change a story in `src/Story.cpp`, change its picture too.

---

## 2. Battle pictures - `assets/sprites/`, `assets/backgrounds/`

| File | Used for | Status |
|---|---|---|
| `sprites/scientist.png`, `bat.png`, `hydra.png` | the three bosses (transparent PNG, any size, scaled to fit) | OK |
| `sprites/viktor_battle.png` | the player in battle | OK |
| `backgrounds/battle_0.png` / `battle_1.png` / `battle_2.png` | Scientist / Bat / Hydra background | OK |
| `backgrounds/battle.png` | one shared background for any boss without its own | optional, not needed |

- Backgrounds: about 2.4:1 (the ones here are about 1942x809). The battle screen adapts to any window shape.
- Pixel-art look for the bosses: set `SMOOTH_BOSS_SPRITES = false` in `src/BattleScreen.cpp`.

---

## 3. Main menu video - `assets/backgrounds/bg_mainmenu/`

SFML can't play mp4, so the menu "video" is a folder of numbered pictures shown at 12 fps.
- Names must be `frame_000.jpg`, `frame_001.jpg`, ... with **no gaps** (loading stops at the first missing number).
- Status: OK, 120 frames (000-119), 640x360, scaled to cover the screen.
- The original video is kept in `assets/source/mainmenu_bg.mp4` (the game does not read it).
- New video: export frames, e.g. `ffmpeg -i assets/source/mainmenu_bg.mp4 -vf fps=12,scale=640:360 assets/backgrounds/bg_mainmenu/frame_%03d.jpg`, then renumber so it starts at `000`. No code change. To change the speed edit `MENU_VIDEO_FPS` in `src/Screens.cpp`.

---

## 4. Music - `assets/music/`

| File | Plays when | Status |
|---|---|---|
| `menu.wav` | main menu and credits | OK |
| `explore.ogg` | walking around, bag, story boxes | OK |
| `battle_2.ogg` | Hydra fight | OK |
| `battle_0.ogg` | Scientist fight | OK |
| `battle_1.ogg` | Bat fight | OK |

- The game tries `.ogg` first, then `.wav`, then `.flac` for every track.
- Music loops. The game over screen is silent on purpose. After a fight the exploration song comes back by itself.
- Volume: `MUSIC_VOLUME` (0-100) at the top of `src/BattleSound.cpp`.

---

## 5. Sound effects - `assets/sounds/`

Short `.wav` files (mostly under 2 s; the longest are `laugh.wav` and `wail.wav`). The names are fixed by the table at the top of `src/BattleSound.cpp`.

| File | Plays when | Status |
|---|---|---|
| `hit.wav` | the player's attack lands | OK |
| `hurt.wav` | the player takes damage (in a fight, from a trap / hole on the map, or by drinking a poison potion in a fight) | OK |
| `block.wav` | the player takes damage while defending | OK |
| `heal.wav` | healing potion (in a fight or in the bag) | OK |
| `buff.wav` | buff potion (fights only) | OK |
| `laugh.wav` | Scientist's laugh | OK |
| `screech.wav` | Bat's screech | OK |
| `wail.wav` | Hydra's wail | OK |
| `victory.wav` | boss defeated | OK |
| `defeat.wav` | player defeated (in a fight, or killed by a trap / hole) | OK |
| `scientist_hit.wav` / `bat_hit.wav` / `hydra_hit.wav` | that boss gets hurt (right after `hit.wav`) | OK |
| `defend.wav` | Viktor braces (Defend) | OK |
| `step.wav` | one footstep when Viktor walks in explore mode (played by `Game.cpp`, quiet, pitch varies a little) | OK |
| `ui_move.wav` | cursor / selection moved | OK |
| `ui_select.wav` | something chosen | OK |
| `ui_back.wav` | cancelled, or "not allowed" | OK |

Volume: `SFX_VOLUME` at the top of `src/BattleSound.cpp`.

**Potion sounds.** Healing works in a fight **and** in the bag (`heal.wav` both times). The buff and poison potions are **fights only**:
in the bag they show "Better to save this one for a fight" and play `ui_back.wav`. In a fight the buff plays `buff.wav` and the poison plays `hurt.wav`
(the battle log says how much HP you lost: 15 on Normal, more on Hard, and the damage number shows).

**UI sounds** (`ui_move`, `ui_select`, `ui_back`) play in the battle screen **and** in `src/Game.cpp`:

| Place | move | select | back |
|---|---|---|---|
| Main menu | arrow keys | Enter / Space | Enter on "Continue" with no save |
| Exploring (E) | | E picked up / talked / read / opened | E found nothing, chest locked, bag full |
| Bag (I) / pause (Esc) | | opens | |
| Bag | cursor between slots | drinking a potion, sorting (S) | closing, empty slot, item can't be used |
| Pause menu | | S saved, M quit | Esc resume, save failed |
| Story boxes | | advancing a text box | |

To change when a sound plays, edit the `playUiSound(UI_SOUND_...)` lines in `src/Game.cpp` (`menuKey`, `exploreKey`, `bagKey`, `sceneKey`).

---

## 6. Icon - `assets/icon.ico`, `assets/icon.png`

- `icon.ico` = the icon of `ouroboros.exe` in Explorer. Built in by `src/ouroboros.rc` (see `scripts/build_game.bat`). Change it, rebuild.
- `icon.png` = the icon in the window's title bar / taskbar (loaded at run time, 256x256).

## 7. Font - `assets/fonts/game.ttf`

Status: **OK**. `game.ttf` is **Crimson Text Regular** (SIL Open Font License, see `assets/fonts/OFL.txt`; keep that file next to the font).
The game tries `assets/fonts/game.ttf` first, then `C:/Windows/Fonts/georgia.ttf`, then `arial.ttf`, so it also runs on a computer without Georgia/Arial.
To use another font, name it `game.ttf` and replace the file (use a static "Regular" `.ttf`, no code change) and update the credits in `src/Story.cpp`.

## 8. When do I have to change the code?

| I want to... | Code change? |
|---|---|
| replace any file above with my own, same name | **No** |
| add the missing sounds / music from the tables | **No**, just add files with those names |
| use another file name or folder | Yes, the path string is in the file named below |
| change the volume | Yes, `SFX_VOLUME` / `MUSIC_VOLUME` in `src/BattleSound.cpp` |
| pixelated boss sprites | Yes, `SMOOTH_BOSS_SPRITES` in `src/BattleScreen.cpp` |
| add a new sound effect | Yes: an id in `Battle.h`, a line in `SOUND_FILES` in `BattleSound.cpp` (same position as the id!), and `addSound(...)` in `Battle.cpp` |
| a new tile type on the map | Yes, `src/Map.cpp` (`drawMap`) |

Where each path lives: map pictures `src/Map.cpp` - item icons (bag + battle) `src/UiDraw.cpp` (`ITEM_ICON`) - menu video `src/Screens.cpp` - sprites and backgrounds `src/BattleScreen.cpp` - sounds and music `src/BattleSound.cpp` - font and window icon `src/Game.cpp`.

## 9. Credits and licences

The credits screen text is in `src/Story.cpp` (`CREDITS_LINES`). Where the assets come from:

| Assets | Source |
|---|---|
| pictures, sprites, backgrounds, menu video, music and most sound effects | AI-generated with ChatGPT and Claude |
| `sounds/hit.wav` (attack / sword slash swoosh) | Mixkit |
| `sounds/wail.wav` (Hydra wail, monster scream) and `sounds/hurt.wav` (pain grunt) | UpperBeat |
| `fonts/game.ttf` | Crimson Text, SIL Open Font License (`fonts/OFL.txt`) |
