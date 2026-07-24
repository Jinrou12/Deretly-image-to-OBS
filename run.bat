@echo off
title Clipboard Image Auto-Saver for OBS & Filmora
cd /d "%~dp0"

echo ===================================================
echo   Clipboard Image Auto-Saver for OBS & Filmora
echo ===================================================
echo.

IF NOT EXIST ".venv\Scripts\python.exe" (
    echo Setting up environment...
    IF EXIST "C:\Users\Visal\.local\bin\uv.exe" (
        "C:\Users\Visal\.local\bin\uv.exe" venv .venv
        "C:\Users\Visal\.local\bin\uv.exe" pip install -r requirements.txt
    ) ELSE (
        uv venv .venv
        uv pip install -r requirements.txt
    )
)

echo Starting Clipboard Auto-Saver...
start "Clipboard Auto-Saver" ".venv\Scripts\python.exe" "main.py"
echo Done! App is now running. Keep the console window open or minimized.
timeout /t 3 >nul
