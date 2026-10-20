@echo off

if exist "C:\raylib\w64devkit\bin" (
    set "PATH=C:\raylib\w64devkit\bin;%PATH%"
)

gcc main.c backend.c -Iinclude -Llib -lraylib -lopengl32 -lgdi32 -lwinmm -o HangmanGame.exe

if %ERRORLEVEL% equ 0 (
    echo [SUCCESS] Build completed successfully.
) else (
    echo [ERROR] Compilation failed.
)