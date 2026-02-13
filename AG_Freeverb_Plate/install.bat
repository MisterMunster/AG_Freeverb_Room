@echo off
REM ── AG Freeverb Plate Installer Launcher ──
REM Runs install.sh via Git Bash from the project directory

set "SCRIPT_DIR=C:\Users\rvanover\Documents\GitHub\AntigravReverb"

if not exist "%SCRIPT_DIR%\install.sh" (
    echo ERROR: install.sh not found at %SCRIPT_DIR%
    pause
    exit /b 1
)

"C:\Program Files\Git\bin\bash.exe" -c "cd '%SCRIPT_DIR%' && ./install.sh"

pause
