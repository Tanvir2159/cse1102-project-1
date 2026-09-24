#include "common.h"
#include "shell.h"
#include "room_attendance.h"
#include "room_records.h"
#include "room_election.h"
#include "room_readability.h"
#include "room_games.h"
#include "room_ledger.h"

/* ---------- ASCII banner (Unicode – works with chcp 65001 / Windows Terminal) ---------- */
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
    ts[strlen(ts) - 1] = '\0';

    printf("\n");
    printf("  \033[1;33m%s\033[0m \033[0;37mv%s\033[0m\n", APP_NAME, APP_VERSION);
    printf("  \033[0;37m--------------------------------\033[0m\n");
    printf("  \033[0;37mhost     \033[0m campus terminal\n");
    printf("  \033[0;37mkernel   \033[0m structured programming (C99)\n");
    printf("  \033[0;37mdate     \033[0m %s\n", ts);
    printf("  \033[0;37mrooms    \033[0m attendance, records, election,\n");
    printf("  \033[0;37m         \033[0m readability, games, ledger\n");
    printf("  \033[0;37mleader   \033[0m %s\n",
           shell_get_leader()[0] ? shell_get_leader() : "(none elected)");
    printf("\n");
}

/* ---------- install.sh style boot lines ---------- */
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
    printf("  type \033[1;33mhelp\033[0m for commands, "
           "\033[1;33mquit\033[0m to exit\n\n");
}

/* ---------- leader key ---------- */
const char *shell_get_leader(void) {
    static char leader[MAX_STR] = {0};
    FILE *fp = fopen(FILE_LEADER, "r");
    if (!fp) return "";
    if (fgets(leader, MAX_STR, fp)) {
        leader[strcspn(leader, "\n")] = '\0';
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
    return (l[0] != '\0' && strcmp(l, name) == 0);
}

/* ---------- status ---------- */
static void cmd_status(void) {
    const char *l = shell_get_leader();
    printf("\n  \033[1;37m┌─ vault status ─────────────────\033[0m\n");
    printf("  │ version    %s\n", APP_VERSION);
    printf("  │ leader     %s\n", l[0] ? l : "(none)");
    printf("  │ rooms      attendance, records, election,\n");
    printf("  │            readability, games, ledger\n");
    printf("  │ data dir   %s\n", DATA_DIR);
    printf("  \033[1;37m└────────────────────────────────\033[0m\n\n");
}

/* ---------- help ---------- */
static void cmd_help(const char *room) {
    if (room == NULL) {
        printf("\n  \033[1;37mcommands\033[0m\n");
        printf("    \033[1;33mls\033[0m                 list all rooms\n");
        printf("    \033[1;33mstatus\033[0m             show vault state\n");
        printf("    \033[1;33mwhoami\033[0m             identify yourself\n");
        printf("    \033[1;33mleader\033[0m             show elected leader\n");
        printf("    \033[1;33mhelp\033[0m               this message\n");
        printf("    \033[1;33mhelp attendance\033[0m    help for a specific room\n");
        printf("    \033[1;33mclear\033[0m              clear screen\n");
        printf("    \033[1;33mquit\033[0m               exit vault\n");
        printf("\n  \033[1;37mrooms\033[0m  (type the name to enter)\n");
        printf("    \033[1;33mattendance\033[0m         view attendance from file\n");
        printf("    \033[1;33mrecords\033[0m            search names in user index\n");
        printf("    \033[1;33melection\033[0m           runoff election for leader\n");
        printf("    \033[1;33mreadability\033[0m        judge text (leader only)\n");
        printf("    \033[1;33mgames\033[0m              guess / rps / hangman / dino\n");
        printf("    \033[1;33mledger\033[0m             lunch bill tracker (binary)\n\n");
        return;
    }
    if (strcmp(room, "attendance") == 0)
        printf("  attendance: reads %s and shows who is present/absent.\n", FILE_ATTEND);
    else if (strcmp(room, "records") == 0)
        printf("  records: type a name fragment; prints every matching user.\n");
    else if (strcmp(room, "election") == 0) {
        printf("  election: runoff voting. eliminates lowest until majority.\n");
        printf("  winner becomes the vault leader (needed for readability).\n");
    }
    else if (strcmp(room, "readability") == 0)
        printf("  readability: Coleman-Liau grade. only the elected leader may use it.\n");
    else if (strcmp(room, "games") == 0)
        printf("  games: 1=guess number  2=rock-paper-scissors  3=hangman  4=dino jump\n");
    else if (strcmp(room, "ledger") == 0)
        printf("  ledger: add/list/settle lunch bills stored in binary file.\n");
    else
        printf("  unknown room: \"%s\"  (try: help attendance)\n", room);
}

/* ---------- dispatch ---------- */
int shell_dispatch(char *line) {
    line[strcspn(line, "\n")] = '\0';
    if (line[0] == '\0') return 0;

    char *cmd = strtok(line, " ");
    char *arg = strtok(NULL, " ");
    if (!cmd) return 0;

    if (strcmp(cmd, "help") == 0) { cmd_help(arg); return 0; }
    if (strcmp(cmd, "ls") == 0) {
        printf("\n  rooms/\n");
        printf("    attendance/   records/    election/\n");
        printf("    readability/  games/      ledger/\n\n");
        return 0;
    }
    if (strcmp(cmd, "clear") == 0) { system(CLEAR); return 0; }
    if (strcmp(cmd, "quit") == 0 || strcmp(cmd, "exit") == 0) return 1;
    if (strcmp(cmd, "status") == 0) { cmd_status(); return 0; }

    if (strcmp(cmd, "leader") == 0) {
        const char *l = shell_get_leader();
        printf("  elected leader: \033[1;33m%s\033[0m\n", l[0] ? l : "(none yet)");
        return 0;
    }

    if (strcmp(cmd, "whoami") == 0) {
        char who[MAX_STR];
        printf("  identify yourself: "); fflush(stdout);
        if (!fgets(who, MAX_STR, stdin)) return 0;
        who[strcspn(who, "\n")] = '\0';

        FILE *fp = fopen(FILE_USERS, "r");
        int known = 0;
        char line2[MAX_STR];
        if (fp) {
            while (fgets(line2, MAX_STR, fp)) {
                line2[strcspn(line2, "\n")] = '\0';
                if (strcmp(line2, who) == 0) { known = 1; break; }
            }
            fclose(fp);
        }
        printf("  hello, \033[1;33m%s\033[0m. known user: %s. leader: %s\n",
               who, known ? "yes" : "no",
               shell_is_leader(who) ? "yes" : "no");
        return 0;
    }

    if (strcmp(cmd, "attendance")  == 0) { room_attendance_run();  return 0; }
    if (strcmp(cmd, "records")     == 0) { room_records_run();     return 0; }
    if (strcmp(cmd, "election")    == 0) { room_election_run();    return 0; }
    if (strcmp(cmd, "readability") == 0) { room_readability_run(); return 0; }
    if (strcmp(cmd, "games")       == 0) { room_games_run();       return 0; }
    if (strcmp(cmd, "ledger")      == 0) { room_ledger_run();      return 0; }

    printf("  unknown command: %s  (try '\033[1;33mhelp\033[0m')\n", cmd);
    return 0;
}

void shell_run(void) {
    char line[MAX_LINE];
    while (1) {
        printf("\033[1;32m%s\033[0m", PROMPT);
        fflush(stdout);
        if (!fgets(line, MAX_LINE, stdin)) break;
        if (shell_dispatch(line)) break;
    }
    printf("  closing vault.\n");
}
