@echo off
cd /d %~dp0
echo Building Mission Impossible...
g++ -o mission.exe main.cpp
if %ERRORLEVEL% NEQ 0 (
    echo Build failed!
    pause
    exit /b 1
)
echo Done. Launching...
start "" mission.exe
