@echo off
echo === Building Tic-Tac-Toe for Windows ===
echo.
echo Installing PyInstaller...
pip install pyinstaller >nul 2>&1
echo Building executable...
pyinstaller --onefile --name tictactoe --clean tictactoe.py
echo.
if exist "dist\tictactoe.exe" (
    echo SUCCESS! Executable is at: dist\tictactoe.exe
    echo You can share this single file with anyone.
) else (
    echo BUILD FAILED. Make sure Python 3.6+ and pip are installed.
)
echo.
pause
