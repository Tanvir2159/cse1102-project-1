# VAULT (v1.0)

> A modular, dual-mode CLI environment and utility suite built in C (C99) for **CSE 1102: Structured Programming Language**.

---

## Overview

**VAULT** is an interactive, multi-room terminal application and subcommand CLI engine. It is designed to showcase fundamental and advanced structured programming concepts in C—including custom data structures, function-pointer dispatch tables, pointers and pointer arithmetic, binary and text file serialization, runoff voting algorithms, cross-platform terminal raw mode, dynamic memory allocation (`malloc`/`free`), and modular system architecture.

```
  ██╗   ██╗ █████╗ ██╗   ██╗██╗  ████████╗
  ██║   ██║██╔══██╗██║   ██║██║  ╚══██╔══╝
  ██║   ██║███████║██║   ██║██║     ██║   
  ╚██╗ ██╔╝██╔══██║██║   ██║██║     ██║   
   ╚████╔╝ ██║  ██║╚██████╔╝███████╗██║   
    ╚═══╝  ╚═╝  ╚═╝ ╚═════╝ ╚══════╝╚═╝   
```

---

## Dual-Mode Operation

VAULT operates seamlessly in **two modes**:

1. **Direct One-Shot CLI Mode:** Run subcommands directly from your system terminal (bash, zsh, Windows PowerShell):
   ```bash
   ./vault records search Rahim
   ./vault ledger add Tanvir Rahim 120
   ./vault attendance summary
   ./vault election run
   ```
2. **Interactive REPL Shell:** Start the interactive shell with live Up/Down arrow history navigation, command shortcuts, and prompt interface:
   ```bash
   ./vault
   vault> attendance mark Rahim P
   vault> ledger settle
   vault> history
   vault> !!
   ```

---

## Features & Modules ("Rooms")

Each module supports **direct subcommands** for fast terminal execution and a **guided interactive menu fallback** when invoked without arguments:

### 1. Attendance Room (`attendance`)
* **File:** `src/room_attendance.c`
* Reads and updates attendance rosters from `data/attendance.txt`.
* Subcommands:
  * `attendance list` – Displays color-coded roster with percentage metrics.
  * `attendance mark <name> <P|A>` – Marks student present (`P`) or absent (`A`), appending new students automatically.
  * `attendance summary` – Displays statistical summary card (total enrolled, present, absent, rate).
  * `attendance` – Opens the interactive menu.

### 2. User Records Room (`records`)
* **File:** `src/room_records.c`
* Searches and registers users in `data/users.txt`.
* Subcommands:
  * `records search <query>` (or `records find <query>`) – Substring query matching.
  * `records list` – Enumerates all registered users.
  * `records add <name>` – Appends a new student to the database.
  * `records` – Opens the interactive search/list/add menu.

### 3. Runoff Election Room (`election`)
* **File:** `src/room_election.c`
* Implements an **Instant-Runoff Voting (IRV)** algorithm with elimination rounds.
* Subcommands:
  * `election run` (or `election tally`) – Tallies votes from `data/votes.txt`, computes multi-round eliminations until majority, and designates the Vault Leader in `data/leader.txt`.
  * `election vote <candidate>` – Casts a new ballot for a registered candidate.
  * `election candidates` – Lists all candidates in `data/candidates.txt`.
  * `election add <candidate>` – Registers a new candidate.
  * `election reset` – Clears all ballots and the current leader.
  * `election` – Opens the interactive voting menu.

### 4. Readability Analyzer (`readability`)
* **File:** `src/room_readability.c`
* **Role-Gated Security:** Restricted strictly to the elected Vault Leader.
* Computes the **Coleman-Liau Readability Index**:
  $$\text{Grade} = 0.0588 \times L - 0.296 \times S - 15.8$$
* Subcommands:
  * `readability file [path]` – Analyzes a file (defaults to `data/sample.txt`).
  * `readability text "<string>"` – Analyzes custom text passed in double quotes.
  * `readability` – Prompts for leader authentication and analyzes default sample text.

### 5. Games Arcade (`games`)
* **File:** `src/room_games.c`
* Subcommands:
  * `games guess` – Number guessing game (1–100) with higher/lower hints.
  * `games rps [rock|paper|scissors]` – Rock-Paper-Scissors against CPU.
  * `games hangman` – Hangman with ASCII gallows and CS-themed words.
  * `games dino` – Endless obstacle runner (real-time on Windows with `conio.h`, turn-based timing fallback on macOS/Linux).
  * `games` – Opens the arcade game selector.

### 6. Shared Ledger (`ledger`)
* **File:** `src/room_ledger.c`
* Demonstrates **binary file serialization** (`data/ledger.dat`) and pointer manipulation.
* Subcommands:
  * `ledger list` – Displays all recorded debt transactions.
  * `ledger add <payer> <ower> <amount>` – Saves a new bill directly to binary storage.
  * `ledger settle` – Performs net settlement calculations using pointer arithmetic.
  * `ledger clear` – Clears binary ledger records.
  * `ledger` – Opens the interactive ledger menu.

---

## Built-in Shell & Engine Commands

From the `vault> ` prompt:

| Command | Usage | Description |
|---|---|---|
| `help` | `help [room]` | Displays command table or room-specific manual. |
| `ls` | `ls` | Lists all accessible rooms. |
| `status` | `status` | Displays system info, data path, and current leader. |
| `whoami` | `whoami [username]` | Authenticates against database and checks leader status. |
| `leader` | `leader` | Shows the currently elected leader. |
| `history` | `history` | Displays recent command history with numerical indices. |
| `!!` | `!!` | Re-executes the immediate last command. |
| `!<n>` | `!<n>` | Re-executes command number `<n>` from history. |
| `clear` | `clear` | Clears the terminal screen (`cls` on Windows, `clear` on POSIX). |
| `quit` / `exit` | `quit` | Exits the Vault shell. |

---

## Architecture Highlights

1. **Function-Pointer Dispatch Table:** Centralized `Command` struct table in `src/shell.c` eliminates deep `if-else` blocks and routes commands via clean function pointer callbacks (`cmd_handler_t`).
2. **Quoted String Tokenizer:** Dynamically splits command strings into `argc` and `argv` tokens, preserving arguments enclosed in double quotes (e.g. `readability text "CSE 1102"`).
3. **Terminal Raw Mode & Live History:** Custom non-canonical POSIX `termios` line reader allows scrolling through previous commands using Up/Down arrow keys directly in the terminal without external library dependencies like ncurses or GNU readline.
4. **Binary & Text File I/O:** Demonstrates dual serialization schemes—formatted plain text files (`attendance.txt`, `candidates.txt`, `votes.txt`, `users.txt`) alongside raw binary structures (`ledger.dat`).

---

## Build & Run Instructions

### Prerequisites
* A C compiler supporting **C99** (`gcc` or `clang`).
* `make` (standard on macOS and Linux).

### Option 1: macOS / Linux (using Makefile)

```bash
# Compile the project
make

# Run in interactive REPL mode
./vault

# Or execute one-shot CLI commands
./vault status
./vault records search Rahim
./vault ledger list

# Compile and run immediately
make run

# Clean build artifacts
make clean
```

### Option 2: Windows (using MinGW / GCC)

```cmd
gcc -Wall -Wextra -std=c99 -Iinclude -o vault.exe src/*.c
vault.exe
```

---

## License & Academic Context

Developed for academic evaluation under the **CSE 1102 (Structured Programming Language)** curriculum at **Khulna University of Engineering & Technology (KUET)**.
