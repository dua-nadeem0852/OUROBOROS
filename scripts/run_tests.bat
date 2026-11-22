@echo off
rem Builds and runs the battle logic tests (uses a fake player, no SFML needed).
rem -static puts the compiler's runtime inside the .exe, so it runs anywhere (even double-clicked).
if not defined MINGW set MINGW=C:\mingw64
set PATH=%MINGW%\bin;%PATH%
cd /d "%~dp0.."

g++ -static -I src tests/TestBattle.cpp tests/PlayerStub.cpp src/Battle.cpp src/Enemy.cpp -o test_battle
if errorlevel 1 (
    echo.
    echo Build FAILED - read the errors above.
    pause
    exit /b 1
)
test_battle.exe
pause
