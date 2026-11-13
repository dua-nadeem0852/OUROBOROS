@echo off
rem Builds and runs the battle demo. Double-click it, or run  .\build_demo.bat  in PowerShell.
rem Assumes: compiler in C:\mingw64   and   SFML 2.6.2 in C:\SFML-2.6.2
set PATH=C:\mingw64\bin;%PATH%

rem SFML's own DLLs
if not exist sfml-graphics-2.dll copy C:\SFML-2.6.2\bin\*.dll . >nul
rem The compiler's runtime DLLs (SFML needs them too) - copied so the .exe also runs when double-clicked
if not exist libgcc_s_seh-1.dll copy C:\mingw64\bin\libgcc_s_seh-1.dll . >nul
if not exist libstdc++-6.dll    copy C:\mingw64\bin\libstdc++-6.dll . >nul
if not exist libwinpthread-1.dll copy C:\mingw64\bin\libwinpthread-1.dll . >nul

g++ BattleDemo.cpp BattleScreen.cpp UiDraw.cpp BattleSound.cpp Battle.cpp Enemy.cpp PlayerStub.cpp -o battledemo -I C:\SFML-2.6.2\include -L C:\SFML-2.6.2\lib -lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio
if errorlevel 1 (
    echo.
    echo Build FAILED - read the errors above.
    pause
    exit /b 1
)
echo Build OK - starting the demo...
battledemo.exe
echo.
echo The demo closed (exit code %errorlevel%).
pause
