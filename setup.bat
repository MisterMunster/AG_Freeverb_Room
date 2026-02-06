@echo off
:: Free-Verb-Room Installer - Robust Version
:: Strategy: Copy files to %TEMP% to prevent IExpress cleanup race conditions.

:: Check if we are already running in the safe temp location with the ELEVATED flag
if "%1"=="ELEVATED" goto :ADMIN_MODE

:: ==========================================
:: USER MODE (Runs inside IExpress temp folder)
:: ==========================================
echo Preparing installer...
set "SAFE_DIR=%TEMP%\FreeVerbSetup"

:: 1. Clean and Create Temp Dir
if exist "%SAFE_DIR%" rmdir /s /q "%SAFE_DIR%"
mkdir "%SAFE_DIR%"

:: 2. Copy Files (Safe from IExpress cleanup)
copy "%~dp0data.zip" "%SAFE_DIR%\" >nul
copy "%~f0" "%SAFE_DIR%\setup.bat" >nul

:: 3. Check Copy Installation
if not exist "%SAFE_DIR%\data.zip" (
    echo ERROR: Failed to stage installer files.
    pause
    exit /b
)

:: 4. Elevate
echo asking for Admin privileges...
:: Use /k to keep the window open so we can see what happens!
powershell -Command "Start-Process cmd -ArgumentList '/k \"\"%SAFE_DIR%\setup.bat\"\" ELEVATED' -Verb RunAs"
exit /b


:: ==========================================
:: ADMIN MODE (Runs inside %TEMP%\FreeVerbSetup)
:: ==========================================
:ADMIN_MODE
color 0f
echo.
echo ==================================================
echo      Free-Verb-Room Installer (Admin)
echo ==================================================
echo.

set "CURRENT_DIR=%~dp0"
set "VST3PATH=C:\Program Files\Common Files\VST3"
if not exist "%VST3PATH%" mkdir "%VST3PATH%"

:: 1. Collision Check (Old 'Antigrav Reverb')
if exist "%VST3PATH%\Antigrav Reverb.vst3" (
    echo Found legacy version: Antigrav Reverb
    echo Deleting legacy version...
    rmdir /s /q "%VST3PATH%\Antigrav Reverb.vst3"
)

:: 2. Collision Check (Existing 'Free-Verb-Room')
if exist "%VST3PATH%\Free-Verb-Room.vst3" (
    echo Found existing version: Free-Verb-Room
    echo Deleting existing version...
    rmdir /s /q "%VST3PATH%\Free-Verb-Room.vst3"
)

:: 3. Install
echo.
echo Installing from: %CURRENT_DIR%
echo Installing to:   %VST3PATH%

powershell -Command "Expand-Archive -Path '%CURRENT_DIR%\data.zip' -DestinationPath '%VST3PATH%' -Force"

if %ERRORLEVEL% NEQ 0 (
    echo.
    echo ERROR: Extraction failed.
    echo.
    pause
    exit /b
)

:: Verify Installation
if not exist "%VST3PATH%\Free-Verb-Room.vst3" (
    echo.
    echo ERROR: Files do not exist after extraction?
    pause
    exit /b
)

echo.
echo ==================================================
echo      INSTALLATION SUCCESSFUL
echo ==================================================
echo plugin installed to: %VST3PATH%\Free-Verb-Room.vst3
echo.
echo You may close this window.
pause
