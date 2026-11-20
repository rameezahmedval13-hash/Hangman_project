# Hangman Game 

A multi-screen, GUI-based desktop Hangman game built in C using the **Raylib** library for First Semester Final Project of **Programming Fundamentals (Fall 2026)** at **FAST-NUCES Karachi**. Unlike basic console Hangman programs where words are hardcoded and progress resets on exit, this project uses plain-text file handling to persist user accounts, match statistics, categorized word banks, and recharging hints across sessions.

---

## Features

- **User Authentication & Masked Passwords:** Register new accounts and log in with duplicate-username checks (`players.txt`) and `*` password masking during input.
- **Persistent Stats & Top-5 Leaderboard:** Automatically updates total games, wins, losses, and points (`+100` per win) in `scores.txt` and ranks the Top 5 players in descending order using Bubble Sort.
- **12 Categories & 3 Difficulty Levels:** Filters words from `words.txt` across 12 topics (*Tech, Math, Science, Animals, Nature, Geography, Places, Objects, People, Jobs, Society, Arts*) and 3 length-based difficulties (`Easy`: 4–5 letters, `Medium`: 6–8 letters, `Hard`: 9+ letters).
- **Time-Based Hint Recharge System:** Tracks up to 3 hints per player in `hints.txt`, automatically restoring 1 spent hint every 8 hours (`28,800` seconds) using `time.h`.
- **Procedural Audio & 60 FPS GUI:** Driven by an enum-based finite state machine (`GameScreen`) across 7 screens with in-memory square-wave sound synthesis (`malloc`) and built-in fallback defaults if external files are missing.

---

## Project Structure

```text
Hangman_project/
├── main.c          # Raylib GUI rendering, input handling, audio synthesis, and screen state machine
├── backend.h       # Player struct definition and backend function prototypes
├── backend.c       # File I/O (players, scores, words, hints), authentication, and Bubble Sort leaderboard
├── build.bat       # Windows GCC build script linking Raylib
├── words.txt       # Categorized word bank (uses fallback words if missing)
├── players.txt     # Generated at runtime to store registered usernames and passwords
├── scores.txt      # Generated at runtime to store player match history and scores
└── hints.txt       # Generated at runtime to store hint balances and UNIX timestamps
```

---

## Prerequisites

- **C Compiler:** GCC via **MinGW-w64** or **w64devkit** (added to your system `PATH`)
- **Graphics Library:** **Raylib** (headers and static/shared libraries configured in your compiler path — <https://www.raylib.com/>)
- **OS:** Windows (for `build.bat` and Win32 linker flags)

---

## How to Build & Run

### 1. Clone the Repository

Open **Command Prompt**, **PowerShell**, or **Git Bash** and run:

```bash
git clone https://github.com/rameezahmedval13-hash/Hangman_project.git
cd Hangman_project
```

### 2. Compile the Project

**Option A — Using the included build script (Recommended):**

```bat
build.bat
```

**Option B — Compiling manually with GCC:**

If Raylib is in your compiler `PATH`:

```bash
gcc main.c backend.c -o hangman.exe -lraylib -lopengl32 -lgdi32 -lwinmm
```

If Raylib is installed in `C:\raylib\raylib`:

```bash
gcc main.c backend.c -o hangman.exe -IC:\raylib\raylib\src -LC:\raylib\raylib\src -lraylib -lopengl32 -lgdi32 -lwinmm
```

### 3. Run the Game

```bat
hangman.exe
```

*(Note: Keep `words.txt` in the same folder as `hangman.exe` to load the full 12-category word bank; if missing, the game automatically uses built-in fallback words. `players.txt`, `scores.txt`, and `hints.txt` are generated automatically when you register and play.)*

---

## Controls

| Action | Control |
| :--- | :--- |
| Navigate menus & select buttons | **Left Mouse Click** |
| Switch between Username & Password fields | **Tab** or **Mouse Click** |
| Delete typed characters | **Backspace** |
| Guess a letter during gameplay | **A – Z** keys |
| Use a hint | Click **USE HINT** button |
| Restart after win/loss | **R** |
| Pause game / Exit | **ESC** |

---

## Authors

- **Rameez Ahmed** (`26K-2527`) — Core multi-file architecture, build pipeline (`build.bat`), user registration & login authentication (`players.txt`, `scores.txt`), `GameScreen` state machine, masked login UI, and core gameplay loop.
- **Muhammad Usaid** (`26K-2500`) — Category & difficulty word filtering (`words.txt`), 8-hour recharging hint system (`hints.txt`), Top-5 Bubble Sort leaderboard, progressive gallows drawing, procedural audio synthesis, and pause/account-switch dialogs.
