#include "common.h"
#include "shell.h"
#include "room_attendance.h"
#include "room_records.h"
#include "room_election.h"
#include "room_readability.h"
#include "room_games.h"
#include "room_ledger.h"

#ifndef _WIN32
#include <termios.h>
#include <unistd.h>
#else
#include <conio.h>
#endif

/* ---------- Command Dispatcher Architecture ---------- */
typedef int (*cmd_handler_t)(int argc, char **argv);

typedef struct {
    const char   *name;
    const char   *desc;
    const char   *usage;
    cmd_handler_t handler;
} Command;

/* Forward declarations for built-in handlers */
static int cmd_help(int argc, char **argv);
static int cmd_ls(int argc, char **argv);
static int cmd_status(int argc, char **argv);
static int cmd_whoami(int argc, char **argv);
static int cmd_leader(int argc, char **argv);
static int cmd_history(int argc, char **argv);
static int cmd_clear(int argc, char **argv);
static int cmd_quit(int argc, char **argv);

static const Command COMMANDS[] = {
    {"help",        "Show help and usage guide",           "help [room]",                    cmd_help},
    {"ls",          "List all available rooms",            "ls",                             cmd_ls},
    {"status",      "Display system status and leader",    "status",                         cmd_status},
    {"whoami",      "Verify your identity and privileges", "whoami [username]",              cmd_whoami},
    {"leader",      "Show elected vault leader",           "leader",                         cmd_leader},
    {"history",     "View recent command history",         "history",                        cmd_history},
    {"clear",       "Clear the terminal screen",           "clear",                          cmd_clear},
    {"quit",        "Exit the Vault shell",                "quit",                           cmd_quit},
    {"exit",        "Exit the Vault shell",                "exit",                           cmd_quit},
    {"attendance",  "Manage class attendance records",     "attendance [list|mark|summary]", room_attendance_cmd},
    {"records",     "Search and manage user records",      "records [search|list|add]",      room_records_cmd},
    {"election",    "Runoff election and voting suite",    "election [run|vote|candidates]", room_election_cmd},
    {"readability", "Coleman-Liau readability analyzer",   "readability [file|text]",        room_readability_cmd},
    {"games",       "Terminal mini-games arcade",          "games [guess|rps|hangman|dino]", room_games_cmd},
    {"ledger",      "Shared expense tracker and balances", "ledger [list|add|settle|clear]", room_ledger_cmd}
};

#define NUM_COMMANDS (sizeof(COMMANDS) / sizeof(COMMANDS[0]))

/* ---------- Command History Engine ---------- */
#define MAX_HISTORY 64
static char *s_history[MAX_HISTORY];
static int   s_hist_count = 0;

void shell_add_history(const char *cmd) {
    if (!cmd || cmd[0] == '\0') return;
    /* Do not store duplicate of immediate last command */
    if (s_hist_count > 0 && strcmp(s_history[(s_hist_count - 1) % MAX_HISTORY], cmd) == 0) {
        return;
    }
    int idx = s_hist_count % MAX_HISTORY;
    if (s_history[idx]) {
        free(s_history[idx]);
    }
    s_history[idx] = strdup(cmd);
    s_hist_count++;
}

void shell_show_history(void) {
    printf("\n  \033[1;36m--- Command History ---\033[0m\n");
    int start = (s_hist_count > MAX_HISTORY) ? (s_hist_count - MAX_HISTORY) : 0;
    for (int i = start; i < s_hist_count; i++) {
        int slot = i % MAX_HISTORY;
        printf("  %3d  %s\n", i + 1, s_history[slot]);
    }
    printf("  \033[0;37m(use !<n> to rerun, or !! for last command)\033[0m\n\n");
}

const char *shell_get_history(int index) {
    if (index < 1 || index > s_hist_count) return NULL;
    int start = (s_hist_count > MAX_HISTORY) ? (s_hist_count - MAX_HISTORY) : 0;
    if (index - 1 < start) return NULL;
    return s_history[(index - 1) % MAX_HISTORY];
}

/* ---------- ASCII Banner & Boot Sequence ---------- */
static void print_banner(void) {
    printf("\n");
    printf("  \033[1;36m");
    printf("  ██╗   ██╗ █████╗ ██╗   ██╗██╗  ████████╗\n");
    printf("  ██║   ██║██╔══██╗██║   ██║██║  ╚══██╔══╝\n");
    printf("  ██║   ██║███████║██║   ██║██║     ██║   \n");
    printf("  ╚██╗ ██╔╝██╔══██║██║   ██║██║     ██║   \n");
    printf("   ╚████╔╝ ██║  ██║╚██████╔╝███████╗██║   \n");
    printf("    ╚═══╝  ╚═╝  ╚═╝ ╚═════╝ ╚══════╝╚═╝   \n");
    printf("\033[0m");

    time_t now = time(NULL);
    char *ts = ctime(&now);
    if (ts) ts[strlen(ts) - 1] = '\0';

    printf("\n");
    printf("  \033[1;33m%s\033[0m \033[0;37mv%s\033[0m\n", APP_NAME, APP_VERSION);
    printf("  \033[0;37m--------------------------------\033[0m\n");
    printf("  \033[0;37mhost     \033[0m campus terminal\n");
    printf("  \033[0;37mkernel   \033[0m structured programming (C99)\n");
    printf("  \033[0;37mdate     \033[0m %s\n", ts ? ts : "unknown");
    printf("  \033[0;37mrooms    \033[0m attendance, records, election,\n");
    printf("  \033[0;37m         \033[0m readability, games, ledger\n");
    printf("  \033[0;37mleader   \033[0m %s\n",
           shell_get_leader()[0] ? shell_get_leader() : "(none elected)");
    printf("\n");
}

static void boot_step(const char *msg) {
    printf("  \033[0;32m[ ok ]\033[0m %s\n", msg);
}

void shell_boot(void) {
    system(CLEAR);
    print_banner();
    printf("  \033[1;37mbooting %s...\033[0m\n\n", APP_NAME);
    boot_step("mounting data directory");
    boot_step("loading attendance ledger");
    boot_step("loading candidate registry");
    boot_step("loading user index");
    boot_step("preparing readability index");
    boot_step("starting shell\n");
    printf("  type \033[1;33mhelp\033[0m for commands, \033[1;33mquit\033[0m to exit\n\n");
}

/* ---------- Leader State Management ---------- */
const char *shell_get_leader(void) {
    static char leader[MAX_STR] = {0};
    FILE *fp = fopen(FILE_LEADER, "r");
    if (!fp) return "";
    if (fgets(leader, MAX_STR, fp)) {
        leader[strcspn(leader, "\r\n")] = '\0';
    } else {
        leader[0] = '\0';
    }
    fclose(fp);
    return leader;
}

void shell_set_leader(const char *name) {
    FILE *fp = fopen(FILE_LEADER, "w");
    if (!fp) return;
    fprintf(fp, "%s\n", name);
    fclose(fp);
}

int shell_is_leader(const char *name) {
    const char *l = shell_get_leader();
    return (l[0] != '\0' && strcasecmp(l, name) == 0);
}

/* ---------- Built-in Command Handlers ---------- */
static int cmd_status(int argc, char **argv) {
    (void)argc; (void)argv;
    const char *l = shell_get_leader();
    printf("\n  \033[1;37m┌─ vault status ─────────────────\033[0m\n");
    printf("  │ version    %s\n", APP_VERSION);
    printf("  │ leader     %s\n", l[0] ? l : "(none)");
    printf("  │ rooms      attendance, records, election,\n");
    printf("  │            readability, games, ledger\n");
    printf("  │ data dir   %s\n", DATA_DIR);
    printf("  \033[1;37m└────────────────────────────────\033[0m\n\n");
    return 0;
}

static int cmd_ls(int argc, char **argv) {
    (void)argc; (void)argv;
    printf("\n  rooms/\n");
    printf("    attendance/   records/    election/\n");
    printf("    readability/  games/      ledger/\n\n");
    return 0;
}

static int cmd_clear(int argc, char **argv) {
    (void)argc; (void)argv;
    system(CLEAR);
    return 0;
}

static int cmd_quit(int argc, char **argv) {
    (void)argc; (void)argv;
    return 999; /* special code to terminate shell loop */
}

static int cmd_leader(int argc, char **argv) {
    (void)argc; (void)argv;
    const char *l = shell_get_leader();
    printf("  elected leader: \033[1;33m%s\033[0m\n", l[0] ? l : "(none yet)");
    return 0;
}

static int cmd_history(int argc, char **argv) {
    (void)argc; (void)argv;
    shell_show_history();
    return 0;
}

static int cmd_whoami(int argc, char **argv) {
    char who[MAX_STR] = {0};
    if (argc >= 2) {
        strncpy(who, argv[1], MAX_STR - 1);
    } else {
        printf("  identify yourself: "); fflush(stdout);
        if (!fgets(who, MAX_STR, stdin)) return 0;
        who[strcspn(who, "\r\n")] = '\0';
    }

    FILE *fp = fopen(FILE_USERS, "r");
    int known = 0;
    char line[MAX_STR];
    if (fp) {
        while (fgets(line, MAX_STR, fp)) {
            line[strcspn(line, "\r\n")] = '\0';
            if (strcasecmp(line, who) == 0) { known = 1; break; }
        }
        fclose(fp);
    }
    printf("  hello, \033[1;33m%s\033[0m. known user: %s. leader: %s\n",
           who, known ? "\033[0;32myes\033[0m" : "\033[0;31mno\033[0m",
           shell_is_leader(who) ? "\033[1;33myes\033[0m" : "no");
    return 0;
}

static int cmd_help(int argc, char **argv) {
    if (argc < 2) {
        printf("\n  \033[1;37mVAULT Commands & Rooms\033[0m\n");
        for (size_t i = 0; i < NUM_COMMANDS; i++) {
            printf("    \033[1;33m%-14s\033[0m %-34s (\033[0;37m%s\033[0m)\n",
                   COMMANDS[i].name, COMMANDS[i].desc, COMMANDS[i].usage);
        }
        printf("\n  Tip: Run subcommands directly (e.g. \033[1;33mrecords search Rahim\033[0m, \033[1;33mledger add Alice Bob 50\033[0m)\n");
        printf("       Or type just the room name to launch its interactive menu.\n\n");
        return 0;
    }

    const char *target = argv[1];
    for (size_t i = 0; i < NUM_COMMANDS; i++) {
        if (strcmp(COMMANDS[i].name, target) == 0) {
            printf("\n  \033[1;33m%s\033[0m: %s\n", COMMANDS[i].name, COMMANDS[i].desc);
            printf("  usage: \033[1;37m%s\033[0m\n\n", COMMANDS[i].usage);
            return 0;
        }
    }
    printf("  unknown command: \"%s\". Type 'help' for available commands.\n", target);
    return 1;
}

/* ---------- Quoted Tokenizer (malloc / free) ---------- */
static int shell_tokenize(const char *input, int *argc_out, char ***argv_out) {
    int capacity = 8;
    int count = 0;
    char **tokens = malloc(capacity * sizeof(char *));
    if (!tokens) return -1;

    const char *p = input;
    while (*p) {
        while (*p && isspace((unsigned char)*p)) p++;
        if (*p == '\0') break;

        char buf[MAX_LINE];
        int buf_idx = 0;

        if (*p == '"') {
            p++; /* skip opening quote */
            while (*p && *p != '"' && buf_idx < MAX_LINE - 1) {
                buf[buf_idx++] = *p++;
            }
            if (*p == '"') p++; /* skip closing quote */
        } else {
            while (*p && !isspace((unsigned char)*p) && buf_idx < MAX_LINE - 1) {
                buf[buf_idx++] = *p++;
            }
        }
        buf[buf_idx] = '\0';

        if (count >= capacity) {
            capacity *= 2;
            char **new_tokens = realloc(tokens, capacity * sizeof(char *));
            if (!new_tokens) {
                for (int i = 0; i < count; i++) free(tokens[i]);
                free(tokens);
                return -1;
            }
            tokens = new_tokens;
        }

        tokens[count] = strdup(buf);
        count++;
    }

    *argc_out = count;
    *argv_out = tokens;
    return 0;
}

static void shell_free_tokens(int argc, char **argv) {
    if (!argv) return;
    for (int i = 0; i < argc; i++) free(argv[i]);
    free(argv);
}

/* ---------- Unified Command Executor ---------- */
int shell_execute(int argc, char **argv) {
    if (argc == 0 || !argv || !argv[0]) return 0;

    const char *cmd_name = argv[0];
    for (size_t i = 0; i < NUM_COMMANDS; i++) {
        if (strcmp(COMMANDS[i].name, cmd_name) == 0) {
            return COMMANDS[i].handler(argc, argv);
        }
    }

    printf("  unknown command: \033[0;31m%s\033[0m  (type '\033[1;33mhelp\033[0m' for list)\n", cmd_name);
    return 1;
}

/* ---------- Line Dispatcher with History Expansion ---------- */
int shell_dispatch(char *line) {
    line[strcspn(line, "\r\n")] = '\0';
    while (*line && isspace((unsigned char)*line)) line++;
    if (line[0] == '\0') return 0;

    char expanded[MAX_LINE];
    strncpy(expanded, line, MAX_LINE - 1);
    expanded[MAX_LINE - 1] = '\0';

    /* Handle !! (last command) */
    if (strcmp(line, "!!") == 0) {
        if (s_hist_count == 0) {
            printf("  no commands in history.\n");
            return 0;
        }
        const char *last = shell_get_history(s_hist_count);
        if (!last) return 0;
        printf("  %s\n", last);
        strncpy(expanded, last, MAX_LINE - 1);
        expanded[MAX_LINE - 1] = '\0';
    }
    /* Handle !<n> */
    else if (line[0] == '!' && isdigit((unsigned char)line[1])) {
        int num = atoi(line + 1);
        const char *rec = shell_get_history(num);
        if (!rec) {
            printf("  history entry !%d not found.\n", num);
            return 0;
        }
        printf("  %s\n", rec);
        strncpy(expanded, rec, MAX_LINE - 1);
        expanded[MAX_LINE - 1] = '\0';
    }

    /* Store command in history */
    shell_add_history(expanded);

    int argc = 0;
    char **argv = NULL;
    if (shell_tokenize(expanded, &argc, &argv) != 0 || argc == 0) {
        shell_free_tokens(argc, argv);
        return 0;
    }

    int res = shell_execute(argc, argv);
    shell_free_tokens(argc, argv);

    return (res == 999) ? 1 : 0;
}

/* ---------- Terminal Raw-Mode Line Reader with Live Up/Down History ---------- */
#ifndef _WIN32

static int shell_read_line_raw(char *buffer, int max_len) {
    struct termios orig_termios, raw;
    if (tcgetattr(STDIN_FILENO, &orig_termios) != 0) {
        /* Fallback to standard fgets */
        return fgets(buffer, max_len, stdin) ? 0 : -1;
    }

    raw = orig_termios;
    raw.c_lflag &= ~(ICANON | ECHO);
    raw.c_cc[VMIN] = 1;
    raw.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);

    int pos = 0;
    int hist_browse_idx = s_hist_count; /* pointing past end */
    buffer[0] = '\0';

    while (1) {
        char c;
        if (read(STDIN_FILENO, &c, 1) <= 0) break;

        if (c == '\n' || c == '\r') {
            buffer[pos] = '\0';
            printf("\n");
            break;
        } else if (c == 127 || c == '\b') { /* Backspace */
            if (pos > 0) {
                pos--;
                buffer[pos] = '\0';
                printf("\b \b");
                fflush(stdout);
            }
        } else if (c == 4) { /* Ctrl+D */
            if (pos == 0) {
                tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
                return -1;
            }
        } else if (c == 27) { /* Escape sequence */
            char seq[2];
            if (read(STDIN_FILENO, &seq[0], 1) <= 0) continue;
            if (read(STDIN_FILENO, &seq[1], 1) <= 0) continue;

            if (seq[0] == '[') {
                if (seq[1] == 'A') { /* UP arrow */
                    if (s_hist_count > 0 && hist_browse_idx > 0) {
                        int min_idx = (s_hist_count > MAX_HISTORY) ? (s_hist_count - MAX_HISTORY + 1) : 1;
                        if (hist_browse_idx > min_idx) {
                            hist_browse_idx--;
                        } else {
                            hist_browse_idx = min_idx;
                        }
                        const char *h = shell_get_history(hist_browse_idx);
                        if (h) {
                            /* clear line */
                            while (pos > 0) { printf("\b \b"); pos--; }
                            strncpy(buffer, h, max_len - 1);
                            buffer[max_len - 1] = '\0';
                            pos = strlen(buffer);
                            printf("%s", buffer);
                            fflush(stdout);
                        }
                    }
                } else if (seq[1] == 'B') { /* DOWN arrow */
                    if (hist_browse_idx < s_hist_count) {
                        hist_browse_idx++;
                        const char *h = shell_get_history(hist_browse_idx);
                        while (pos > 0) { printf("\b \b"); pos--; }
                        if (h) {
                            strncpy(buffer, h, max_len - 1);
                            buffer[max_len - 1] = '\0';
                            pos = strlen(buffer);
                            printf("%s", buffer);
                        } else {
                            buffer[0] = '\0';
                            pos = 0;
                        }
                        fflush(stdout);
                    } else {
                        while (pos > 0) { printf("\b \b"); pos--; }
                        buffer[0] = '\0';
                        pos = 0;
                        fflush(stdout);
                    }
                }
            }
        } else if ((unsigned char)c >= 32 && pos < max_len - 1) {
            buffer[pos++] = c;
            buffer[pos] = '\0';
            putchar(c);
            fflush(stdout);
        }
    }

    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
    return 0;
}

#else

static int shell_read_line_raw(char *buffer, int max_len) {
    return fgets(buffer, max_len, stdin) ? 0 : -1;
}

#endif

/* ---------- Interactive REPL Runner ---------- */
void shell_run(void) {
    char line[MAX_LINE];
    while (1) {
        printf("\033[1;32m%s\033[0m", PROMPT);
        fflush(stdout);
        if (shell_read_line_raw(line, MAX_LINE) != 0) break;
        if (shell_dispatch(line)) break;
    }
    printf("  closing vault.\n");
}
