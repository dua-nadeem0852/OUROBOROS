# Fonts

`game.ttf` is **Crimson Text Regular** by The Crimson Text Project Authors, under the SIL Open Font License (`OFL.txt`, keep it in this folder).

To use a different font, name it `game.ttf` and replace this file (a static "Regular" `.ttf`, not a variable font).
If `game.ttf` is missing the game falls back to `C:/Windows/Fonts/georgia.ttf`, then `arial.ttf`, and quits with an error if none exist.
If you change the font, update the credits in `src/Story.cpp` and `docs/ASSET_GUIDE.md`.
