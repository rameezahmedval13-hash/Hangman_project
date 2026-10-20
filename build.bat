@echo off
echo Compiling Hangman Project with Raylib Compiler...
C:\raylib\w64devkit\bin\gcc.exe main.c backend.c -o hangman.exe -I C:\raylib\raylib\src -L C:\raylib\raylib\src -lraylib -lopengl32 -lgdi32 -lwinmm

if %ERRORLEVEL% EQU 0 (
    echo.
    echo ===================================
    echo  Build Successful! Launching Game...
    echo ===================================
    echo.
    hangman.exe
) else (
    echo.
    echo [ERROR] Compilation failed.
)