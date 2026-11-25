@echo off
rem Builds and runs the whole game (map + player + combat).
rem Double-click it, or run  scripts\build_game.bat  from the project root in PowerShell / cmd.
rem
rem Needs: MinGW-w64 (g++, windres) and SFML 2.6.2 for MinGW.
rem If they are not in the default folders, either change the two paths below
rem or set the MINGW and SFML environment variables before running this.
if not defined MINGW set MINGW=C:\mingw64
if not defined SFML  set SFML=C:\SFML-2.6.2

set PATH=%MINGW%\bin;%PATH%

rem always work from the project root (the folder that contains src, assets and scripts),
rem the game loads assets\... relative to this folder
cd /d "%~dp0.."

rem SFML's own DLLs and the compiler's runtime DLLs go next to the exe (git ignores them)
if not exist sfml-graphics-2.dll copy "%SFML%\bin\*.dll" . >nul
if not exist libgcc_s_seh-1.dll  copy "%MINGW%\bin\libgcc_s_seh-1.dll" . >nul
if not exist libstdc++-6.dll     copy "%MINGW%\bin\libstdc++-6.dll" . >nul
if not exist libwinpthread-1.dll copy "%MINGW%\bin\libwinpthread-1.dll" . >nul

rem the game icon (file icon of the exe): turns src\ouroboros.rc + assets\icon.ico into an object file
windres src/ouroboros.rc -O coff -o ouroboros_res.o
if errorlevel 1 (
    echo.
    echo Icon build FAILED - read the errors above.
    pause
    exit /b 1
)

rem -mwindows = build the exe as a window program, so no black terminal opens behind the game.
rem Delete it if you want the terminal back (the cout error messages only show there).
g++ src/main.cpp src/Game.cpp src/Map.cpp src/Screens.cpp src/Story.cpp src/BattleScreen.cpp src/BattleSound.cpp src/UiDraw.cpp src/Battle.cpp src/Enemy.cpp src/Player.cpp src/SaveSystem.cpp ouroboros_res.o -o ouroboros -I src -I "%SFML%\include" -L "%SFML%\lib" -lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio -mwindows
if errorlevel 1 (
    echo.
    echo Build FAILED - read the errors above.
    pause
    exit /b 1
)
echo Build OK - starting the game...
rem "start" opens the game on its own, so this window closes and no terminal stays behind
start "" ouroboros.exe
