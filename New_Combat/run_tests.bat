@echo off
rem Builds and runs the battle logic tests (no SFML needed).
rem -static puts the compiler's runtime inside the .exe, so it runs anywhere (even double-clicked).
set PATH=C:\mingw64\bin;%PATH%
g++ -static TestBattle.cpp Battle.cpp Enemy.cpp PlayerStub.cpp -o test_battle
if errorlevel 1 (
    echo.
    echo Build FAILED - read the errors above.
    pause
    exit /b 1
)
test_battle.exe
pause
