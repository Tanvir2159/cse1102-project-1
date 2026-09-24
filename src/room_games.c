#include "common.h"
#include "room_games.h"

#ifdef _WIN32
#include <conio.h>
#include <windows.h>
#define SLEEP_MS(ms) Sleep(ms)
#define HAS_CONIO 1
#else
#include <unistd.h>
#define SLEEP_MS(ms) usleep((ms) * 1000)
#define HAS_CONIO 0
#endif

/* ============================================================
   1. Guess the number
   ============================================================ */
static void game_guess(void) {
    srand((unsigned)time(NULL));
    int secret = rand() % 100 + 1;
    int guess = 0, tries = 0;

    printf("\n  \033[1;36m=== Guess the Number ===\033[0m\n");
    printf("  I picked a number between 1 and 100.\n");
    while (guess != secret) {
        printf("  guess: "); fflush(stdout);
        if (scanf("%d", &guess) != 1) { while (getchar() != '\n'); continue; }
        tries++;
        if (guess < secret)      printf("  \033[0;33mhigher\033[0m\n");
        else if (guess > secret) printf("  \033[0;33mlower\033[0m\n");
    }
    printf("  \033[1;32mCorrect in %d tries!\033[0m\n\n", tries);
}

/* ============================================================
   2. Rock Paper Scissors
   ============================================================ */
static void game_rps(void) {
    const char *moves[] = {"rock", "paper", "scissors"};
    char input[MAX_STR];
    srand((unsigned)time(NULL));
    int wins = 0, losses = 0, draws = 0;

    printf("\n  \033[1;36m=== Rock Paper Scissors ===\033[0m\n");
    printf("  type rock / paper / scissors  (or q to quit)\n");
    while (1) {
        printf("  your move: "); fflush(stdout);
        if (!fgets(input, MAX_STR, stdin)) return;
        input[strcspn(input, "\n")] = '\0';
        if (strcmp(input, "q") == 0) break;

        int player = -1;
        for (int i = 0; i < 3; i++)
            if (strcmp(input, moves[i]) == 0) { player = i; break; }
        if (player < 0) { printf("  invalid. try rock, paper or scissors.\n"); continue; }

        int cpu = rand() % 3;
        printf("  cpu: \033[1;33m%s\033[0m\n", moves[cpu]);
        if (player == cpu) {
            printf("  draw.\n"); draws++;
        } else if ((player - cpu + 3) % 3 == 1) {
            printf("  \033[1;32myou win!\033[0m\n"); wins++;
        } else {
            printf("  \033[0;31myou lose.\033[0m\n"); losses++;
        }
    }
    printf("  score  W:%d  L:%d  D:%d\n\n", wins, losses, draws);
}

/* ============================================================
   3. Hangman
   ============================================================ */
static const char *HANGMAN_WORDS[] = {
    "computer", "program", "pointer", "struct", "function",
    "variable", "compile", "debug", "array", "string",
    "vault", "election", "leader", "campus", "terminal"
};
#define N_WORDS (sizeof(HANGMAN_WORDS) / sizeof(HANGMAN_WORDS[0]))

static void draw_hangman(int wrong) {
    const char *stages[] = {
        "\n"
        "      +---+\n"
        "      |   |\n"
        "          |\n"
        "          |\n"
        "          |\n"
        "          |\n"
        "    =========\n",
        "\n"
        "      +---+\n"
        "      |   |\n"
        "      O   |\n"
        "          |\n"
        "          |\n"
        "          |\n"
        "    =========\n",
        "\n"
        "      +---+\n"
        "      |   |\n"
        "      O   |\n"
        "      |   |\n"
        "          |\n"
        "          |\n"
        "    =========\n",
        "\n"
        "      +---+\n"
        "      |   |\n"
        "      O   |\n"
        "     /|   |\n"
        "          |\n"
        "          |\n"
        "    =========\n",
        "\n"
        "      +---+\n"
        "      |   |\n"
        "      O   |\n"
        "     /|\\  |\n"
        "          |\n"
        "          |\n"
        "    =========\n",
        "\n"
        "      +---+\n"
        "      |   |\n"
        "      O   |\n"
        "     /|\\  |\n"
        "     /    |\n"
        "          |\n"
        "    =========\n",
        "\n"
        "      +---+\n"
        "      |   |\n"
        "      O   |\n"
        "     /|\\  |\n"
        "     / \\  |\n"
        "          |\n"
        "    =========\n"
    };
    printf("%s", stages[wrong]);
}

static void game_hangman(void) {
    srand((unsigned)time(NULL));
    const char *word = HANGMAN_WORDS[rand() % N_WORDS];
    int len = (int)strlen(word);
    char revealed[MAX_STR];
    char guessed[32] = {0};
    int wrong = 0;
    int found;

    for (int i = 0; i < len; i++) revealed[i] = '_';
    revealed[len] = '\0';

    printf("\n  \033[1;36m=== Hangman ===\033[0m\n");
    printf("  CS50 / programming themed words. 6 wrong guesses max.\n");

    while (wrong < 6) {
        draw_hangman(wrong);
        printf("  word: ");
        for (int i = 0; i < len; i++) printf("%c ", revealed[i]);
        printf("\n  guessed: %s\n", guessed[0] ? guessed : "(none)");
        printf("  letter: "); fflush(stdout);

        char line[MAX_STR];
        if (!fgets(line, MAX_STR, stdin)) return;
        char ch = (char)tolower((unsigned char)line[0]);
        if (!isalpha(ch)) { printf("  enter a letter.\n"); continue; }

        /* already guessed? */
        if (strchr(guessed, ch)) {
            printf("  already tried '%c'.\n", ch);
            continue;
        }
        int glen = (int)strlen(guessed);
        guessed[glen] = ch;
        guessed[glen + 1] = '\0';

        found = 0;
        for (int i = 0; i < len; i++) {
            if (word[i] == ch) {
                revealed[i] = ch;
                found = 1;
            }
        }
        if (!found) {
            wrong++;
            printf("  \033[0;31mnope.\033[0m\n");
        } else {
            printf("  \033[0;32mhit!\033[0m\n");
        }

        if (strchr(revealed, '_') == NULL) {
            draw_hangman(wrong);
            printf("  word: %s\n", word);
            printf("  \033[1;32mYou won!\033[0m\n\n");
            return;
        }
    }
    draw_hangman(6);
    printf("  The word was: \033[1;33m%s\033[0m\n", word);
    printf("  \033[0;31mGame over.\033[0m\n\n");
}

/* ============================================================
   4. Dino Jump (ASCII endless runner)
   Windows: real-time with conio
   Other:   simplified turn-based fallback
   ============================================================ */
#if HAS_CONIO

static void game_dino(void) {
    const int WIDTH = 40;
    int dino_y = 0;          /* 0 = ground, >0 = in air */
    int dino_vel = 0;
    int cactus_x = WIDTH - 1;
    int score = 0;
    int speed = 80;          /* ms per frame */
    int running = 1;
    char ground[64];

    printf("\n  \033[1;36m=== Dino Jump ===\033[0m\n");
    printf("  Press SPACE to jump. Avoid the cactus (^).\n");
    printf("  Press Q to quit.\n");
    printf("  Starting in 2 seconds...\n");
    SLEEP_MS(2000);

    while (running) {
        /* input */
        if (_kbhit()) {
            int k = _getch();
            if (k == ' ' && dino_y == 0) dino_vel = 4;
            if (k == 'q' || k == 'Q') { running = 0; break; }
        }

        /* physics */
        if (dino_vel > 0 || dino_y > 0) {
            dino_y += dino_vel;
            dino_vel--;
            if (dino_y <= 0) { dino_y = 0; dino_vel = 0; }
        }

        /* move cactus */
        cactus_x--;
        if (cactus_x < 0) {
            cactus_x = WIDTH - 1 - (rand() % 10);
            score++;
            if (speed > 30) speed -= 2;
        }

        /* collision */
        if (cactus_x == 3 && dino_y == 0) {
            system("cls");
            printf("\n  \033[0;31mCRASH!\033[0m  Score: \033[1;33m%d\033[0m\n\n", score);
            return;
        }

        /* draw */
        system("cls");
        printf("  \033[1;36mDINO JUMP\033[0m   score: %d\n\n", score);

        /* sky / dino row */
        for (int row = 4; row >= 0; row--) {
            printf("  ");
            for (int x = 0; x < WIDTH; x++) {
                if (x == 3 && dino_y == row)
                    printf("\033[1;32m@\033[0m");
                else if (x == cactus_x && row == 0)
                    printf("\033[0;31m^\033[0m");
                else
                    printf(" ");
            }
            printf("\n");
        }
        /* ground */
        memset(ground, '=', WIDTH);
        ground[WIDTH] = '\0';
        printf("  %s\n", ground);
        printf("  SPACE = jump   Q = quit\n");

        SLEEP_MS(speed);
    }
    printf("\n  final score: %d\n\n", score);
}

#else  /* non-Windows simplified version */

static void game_dino(void) {
    int pos = 0;
    int score = 0;
    char line[MAX_STR];

    printf("\n  \033[1;36m=== Dino Jump (simple) ===\033[0m\n");
    printf("  Press Enter to jump when cactus is near.\n");
    printf("  Type q + Enter to quit.\n\n");

    while (1) {
        int cactus = 8 + (rand() % 6);
        printf("  track: ");
        for (int i = 0; i < 15; i++) {
            if (i == 2) printf("\033[1;32m@\033[0m");
            else if (i == cactus) printf("\033[0;31m^\033[0m");
            else printf("_");
        }
        printf("   cactus distance: %d\n", cactus - 2);
        printf("  jump? (enter / q): "); fflush(stdout);
        if (!fgets(line, MAX_STR, stdin)) return;
        if (line[0] == 'q' || line[0] == 'Q') break;

        if (cactus <= 4) {
            printf("  \033[1;32mjumped over!\033[0m\n");
            score++;
        } else {
            printf("  \033[0;31mtoo early / crash!\033[0m  Score: %d\n\n", score);
            return;
        }
        pos++;
    }
    printf("  final score: %d\n\n", score);
}

#endif

/* ============================================================
   Games menu
   ============================================================ */
void room_games_run(void) {
    printf("\n  \033[1;36m┌─ games room ──────────────────┐\033[0m\n");
    printf("  │  1. Guess the number          │\n");
    printf("  │  2. Rock Paper Scissors       │\n");
    printf("  │  3. Hangman                   │\n");
    printf("  │  4. Dino Jump                 │\n");
    printf("  \033[1;36m└────────────────────────────────┘\033[0m\n");
    printf("  choice: ");
    int c;
    if (scanf("%d", &c) != 1) { while (getchar() != '\n'); return; }
    while (getchar() != '\n');

    if (c == 1) game_guess();
    else if (c == 2) game_rps();
    else if (c == 3) game_hangman();
    else if (c == 4) game_dino();
    else printf("  invalid choice.\n");
}
