# VAULT (v1.0)

> A modular, terminal-based CLI environment and utility suite built in C (C99) for **CSE 1102: Structured Programming Language**.

---

## Overview

**VAULT** is an interactive, multi-room terminal application that simulates a custom shell interface. Designed to demonstrate fundamental and advanced structured programming concepts in C—including custom data structures, pointers and pointer arithmetic, binary and text file I/O, recursion/simulation algorithms, cross-platform terminal handling, and modular project organization.

```
  ██╗   ██╗ █████╗ ██╗   ██╗██╗  ████████╗
  ██║   ██║██╔══██╗██║   ██║██║  ╚══██╔══╝
  ██║   ██║███████║██║   ██║██║     ██║   
  ╚██╗ ██╔╝██╔══██║██║   ██║██║     ██║   
   ╚████╔╝ ██║  ██║╚██████╔╝███████╗██║   
    ╚═══╝  ╚═╝  ╚═╝ ╚═════╝ ╚══════╝╚═╝   
```

---

## Features & Modules ("Rooms")

The application is structured into specialized functional units known as **Rooms**:

### 1. Attendance Room (`attendance`)
* **File:** `src/room_attendance.c`
* Reads attendance rosters from `data/attendance.txt`.
* Parses participant names and status indicators (`P` for present, `A` for absent).
* Outputs a color-coded status summary and aggregates attendance percentages.

### 2. User Records Room (`records`)
* **File:** `src/room_records.c`
* Searches the registered student/user registry in `data/users.txt`.
* Performs substring matching on search terms and displays all matching entries.

### 3. Runoff Election Room (`election`)
* **File:** `src/room_election.c`
* Implements an **Instant-Runoff Voting (IRV)** algorithm.
* Loads candidates from `data/candidates.txt` and ballots from `data/votes.txt`.
* Iteratively eliminates the candidate with the lowest vote count until a candidate secures an absolute majority (> 50%).
* Persists the elected leader to `data/leader.txt`.

### 4. Readability Analyzer (`readability`)
* **File:** `src/room_readability.c`
* **Access Control:** Restricted strictly to the elected Vault Leader. Non-leaders are blocked from running analyses until an election is completed.
* Computes the **Coleman-Liau Readability Index** on text from `data/sample.txt`:
  $$\text{Grade} = 0.0588 \times L - 0.296 \times S - 15.8$$
  *(where $L$ is the average number of letters per 100 words, and $S$ is the average number of sentences per 100 words).*

### 5. Games Arcade (`games`)
* **File:** `src/room_games.c`
* A collection of four classic terminal games:
  1. **Guess the Number:** Random number guessing game between 1 and 100 with directional hints.
  2. **Rock, Paper, Scissors:** Fast-paced game against the computer with win/loss/draw records.
  3. **Hangman:** Interactive word-guessing game with ASCII gallows and programming-themed vocabulary.
  4. **Dino Jump:** Endless obstacle runner with jump mechanics (real-time with `conio.h` on Windows; turn-based timing fallback on Linux/macOS).

### 6. Shared Ledger (`ledger`)
* **File:** `src/room_ledger.c`
* Demonstrates **binary file serialization** (`data/ledger.dat`) and pointer manipulation.
* Features:
  * **List Entries:** View all outstanding meal/shared debts (`<ower> owes <payer> <amount>`).
  * **Add Entry:** Append a new record directly into binary storage.
  * **Settle All:** Uses pointer arithmetic to traverse active entries and calculate net balances.

---

## Built-in Shell Commands

From the `vault> ` prompt:

| Command | Description |
|---|---|
| `help` | Displays command overview and available rooms. |
| `help <room>` | Shows detailed instructions for a specific room (e.g., `help election`). |
| `ls` | Lists all accessible rooms. |
| `status` | Displays system info, data path, and current leader. |
| `whoami` | Identifies the user against the database and checks leader status. |
| `leader` | Shows the currently elected leader. |
| `clear` | Clears the terminal screen (`cls` on Windows, `clear` on POSIX). |
| `<room_name>` | Enters the specified room (e.g., `attendance`, `election`, `games`). |
| `quit` / `exit` | Exits the Vault shell. |

---

## Project Structure

```
Project-CSE-1102-1/
├── Makefile                # Build automation for Unix systems
├── .gitignore              # Ignores build artifacts and runtime data
├── README.md               # Project documentation
├── include/                # Header files
│   ├── common.h            # Global macros, data paths, and structs
│   ├── shell.h             # Shell lifecycle and dispatcher prototypes
│   ├── room_attendance.h   # Attendance room definitions
│   ├── room_election.h     # Election room definitions
│   ├── room_games.h        # Games arcade definitions
│   ├── room_ledger.h       # Binary ledger definitions
│   ├── room_readability.h  # Readability analyzer definitions
│   └── room_records.h      # Records search definitions
├── src/                    # Source code
│   ├── main.c              # Application entry point
│   ├── shell.c             # Shell loop, banner, and command dispatching
│   ├── room_attendance.c   # Attendance processing logic
│   ├── room_election.c     # Runoff voting logic
│   ├── room_games.c        # Mini-games implementation
│   ├── room_ledger.c       # Binary file I/O & pointer ledger
│   ├── room_readability.c  # Coleman-Liau readability calculation
│   └── room_records.c      # String matching and user lookup
└── data/                   # Seed files and runtime storage
    ├── attendance.txt      # Class roster with attendance flags
    ├── candidates.txt      # Registered election candidates
    ├── sample.txt          # Sample text for readability testing
    ├── users.txt           # Registered user database
    ├── votes.txt           # Cast ballots for runoff voting
    ├── leader.txt          # (Generated) Current elected leader
    └── ledger.dat          # (Generated) Binary ledger data
```

---

## Build & Run Instructions

### Prerequisites
* A C compiler supporting **C99** (`gcc` or `clang`).
* `make` (standard on macOS and Linux).

### Option 1: macOS / Linux (using Makefile)

1. **Compile the program:**
   ```bash
   make
   ```
2. **Launch the shell:**
   ```bash
   ./vault
   ```
3. **Compile and run in one step:**
   ```bash
   make run
   ```
4. **Clean up compiled object files and binaries:**
   ```bash
   make clean
   ```

---

### Option 2: Windows (using MinGW / GCC)

Compile all source files directly:
```cmd
gcc -Wall -Wextra -std=c99 -Iinclude -o vault.exe src/*.c
vault.exe
```

*Note: On Windows systems, run in a terminal supporting ANSI color codes and UTF-8 encoding (such as Windows Terminal or `chcp 65001`).*

---

## Typical Workflow Example

1. **Start the vault:**
   ```bash
   ./vault
   ```
2. **Hold an election to designate a leader:**
   ```
   vault> election
   ```
   *(Votes are tallied from `data/votes.txt` and the winner is saved as the system leader).*
3. **Authenticate:**
   ```
   vault> whoami
   identify yourself: <winner_name>
   ```
4. **Analyze text as the leader:**
   ```
   vault> readability
   identify yourself: <winner_name>
   ```
5. **Manage shared bills or play games:**
   ```
   vault> ledger
   vault> games
   ```
6. **Exit the session:**
   ```
   vault> quit
   ```

---

## License & Academic Context

Developed for academic purposes under the **CSE 1102 (Structured Programming Language)** curriculum.
