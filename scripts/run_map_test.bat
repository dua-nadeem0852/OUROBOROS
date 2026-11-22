@echo off
rem Builds and runs the map / save / inventory tests (uses the real Player.cpp, no window opens).
rem Needs SFML because Map.cpp uses its types.
if not defined MINGW set MINGW=C:\mingw64
if not defined SFML  set SFML=C:\SFML-2.6.2
set PATH=%MINGW%\bin;%PATH%
cd /d "%~dp0.."

if not exist sfml-graphics-2.dll copy "%SFML%\bin\*.dll" . >nul
if not exist libgcc_s_seh-1.dll  copy "%MINGW%\bin\libgcc_s_seh-1.dll" . >nul
if not exist libstdc++-6.dll     copy "%MINGW%\bin\libstdc++-6.dll" . >nul
if not exist libwinpthread-1.dll copy "%MINGW%\bin\libwinpthread-1.dll" . >nul

g++ -I src -I "%SFML%\include" tests/TestMap.cpp src/Map.cpp src/Battle.cpp src/Enemy.cpp src/Player.cpp src/SaveSystem.cpp -o test_map -L "%SFML%\lib" -lsfml-graphics -lsfml-window -lsfml-system
if errorlevel 1 (
    echo.
    echo Build FAILED - read the errors above.
    pause
    exit /b 1
)
test_map.exe
pause
