#include "common.h"
#include "room_election.h"
#include "shell.h"

static int load_candidates(Candidate *cands, int max_cands) {
    FILE *cf = fopen(FILE_CANDID, "r");
    if (!cf) return 0;

    int n = 0;
    char line[MAX_LINE];
    while (fgets(line, MAX_LINE, cf) && n < max_cands) {
        line[strcspn(line, "\r\n")] = '\0';
        if (line[0] == '\0') continue;
        strncpy(cands[n].name, line, MAX_STR - 1);
        cands[n].name[MAX_STR - 1] = '\0';
        cands[n].votes = 0;
        cands[n].eliminated = 0;
        n++;
    }
    fclose(cf);
    return n;
}

static void list_candidates(void) {
    Candidate cands[MAX_RECORDS];
    int n = load_candidates(cands, MAX_RECORDS);
    printf("\n  \033[1;36m--- Registered Election Candidates ---\033[0m\n");
    if (n == 0) {
        printf("  no candidates registered in %s\n\n", FILE_CANDID);
        return;
    }
    for (int i = 0; i < n; i++) {
        printf("  %2d. %s\n", i + 1, cands[i].name);
    }
    printf("\n");
}

static void add_candidate(const char *name) {
    Candidate cands[MAX_RECORDS];
    int n = load_candidates(cands, MAX_RECORDS);
    for (int i = 0; i < n; i++) {
        if (strcasecmp(cands[i].name, name) == 0) {
            printf("  candidate '\033[1;33m%s\033[0m' already registered.\n", name);
            return;
        }
    }
    FILE *fp = fopen(FILE_CANDID, "a");
    if (!fp) {
        printf("  failed to write to %s\n", FILE_CANDID);
        return;
    }
    fprintf(fp, "%s\n", name);
    fclose(fp);
    printf("  registered candidate: \033[1;32m%s\033[0m\n", name);
}

static void cast_vote(const char *candidate) {
    Candidate cands[MAX_RECORDS];
    int n = load_candidates(cands, MAX_RECORDS);
    int valid = 0;
    char proper_name[MAX_STR];

    for (int i = 0; i < n; i++) {
        if (strcasecmp(cands[i].name, candidate) == 0) {
            valid = 1;
            strncpy(proper_name, cands[i].name, MAX_STR - 1);
            proper_name[MAX_STR - 1] = '\0';
            break;
        }
    }

    if (!valid) {
        printf("  \033[0;31merror:\033[0m '%s' is not a registered candidate.\n", candidate);
        printf("  run 'election candidates' to see valid names.\n");
        return;
    }

    FILE *vf = fopen(FILE_VOTES, "a");
    if (!vf) {
        printf("  failed to open %s\n", FILE_VOTES);
        return;
    }
    fprintf(vf, "%s\n", proper_name);
    fclose(vf);
    printf("  ballot cast for \033[1;32m%s\033[0m.\n", proper_name);
}

static void election_reset(void) {
    FILE *vf = fopen(FILE_VOTES, "w");
    if (vf) fclose(vf);
    FILE *lf = fopen(FILE_LEADER, "w");
    if (lf) fclose(lf);
    printf("  election reset. All ballots and current leader cleared.\n");
}

static void election_tally(void) {
    Candidate cands[MAX_RECORDS];
    int n = load_candidates(cands, MAX_RECORDS);
    if (n == 0) {
        printf("  no candidates registered in %s\n", FILE_CANDID);
        return;
    }

    FILE *vf = fopen(FILE_VOTES, "r");
    if (!vf) {
        printf("  no votes file at %s\n", FILE_VOTES);
        return;
    }

    int total = 0;
    char line[MAX_LINE];
    while (fgets(line, MAX_LINE, vf)) {
        line[strcspn(line, "\r\n")] = '\0';
        if (line[0] == '\0') continue;
        for (int i = 0; i < n; i++) {
            if (strcasecmp(cands[i].name, line) == 0) {
                cands[i].votes++;
                total++;
                break;
            }
        }
    }
    fclose(vf);

    if (total == 0) {
        printf("  no votes cast yet. Cast votes using 'election vote <candidate>'.\n");
        return;
    }

    printf("\n  \033[1;36m--- Runoff Election Tally (%d Total Votes) ---\033[0m\n", total);
    int round = 1;

    while (1) {
        int active = 0, best = -1;
        printf("  \033[1;37mRound %d:\033[0m\n", round++);
        for (int i = 0; i < n; i++) {
            if (cands[i].eliminated) continue;
            active++;
            printf("    %-16s %3d votes (%.1f%%)\n",
                   cands[i].name, cands[i].votes, ((float)cands[i].votes / total) * 100.0f);
            if (cands[i].votes > best) best = cands[i].votes;
        }

        if (active == 1 || best * 2 > total) break;

        /* Find lowest active votes */
        int low = -1;
        for (int i = 0; i < n; i++) {
            if (cands[i].eliminated) continue;
            if (low < 0 || cands[i].votes < cands[low].votes) low = i;
        }

        printf("  --> eliminating \033[0;31m%s\033[0m (%d votes)\n\n",
               cands[low].name, cands[low].votes);
        cands[low].eliminated = 1;
    }

    int winner = -1, best = -1;
    for (int i = 0; i < n; i++) {
        if (cands[i].eliminated) continue;
        if (cands[i].votes > best) { best = cands[i].votes; winner = i; }
    }

    if (winner >= 0) {
        printf("\n  \033[1;32m★ WINNER: %s (%d / %d votes, %.1f%%)\033[0m\n",
               cands[winner].name, cands[winner].votes, total,
               ((float)cands[winner].votes / total) * 100.0f);
        shell_set_leader(cands[winner].name);
        printf("  \033[1;33m%s is now the elected Vault Leader.\033[0m\n\n", cands[winner].name);
    }
}

void room_election_run(void) {
    while (1) {
        printf("\n  \033[1;36m┌─ Election Room ──────────────┐\033[0m\n");
        printf("  │ 1. Run election runoff       │\n");
        printf("  │ 2. Cast a ballot             │\n");
        printf("  │ 3. View candidates           │\n");
        printf("  │ 4. Add candidate             │\n");
        printf("  │ 5. Reset election            │\n");
        printf("  │ 0. Return to Vault           │\n");
        printf("  \033[1;36m└──────────────────────────────┘\033[0m\n");
        printf("  choice: "); fflush(stdout);

        char buf[64];
        if (!fgets(buf, sizeof(buf), stdin)) break;
        int choice = atoi(buf);
        if (choice == 0) break;

        if (choice == 1) {
            election_tally();
        } else if (choice == 2) {
            char name[MAX_STR];
            printf("  candidate name: "); fflush(stdout);
            if (!fgets(name, sizeof(name), stdin)) continue;
            name[strcspn(name, "\r\n")] = '\0';
            cast_vote(name);
        } else if (choice == 3) {
            list_candidates();
        } else if (choice == 4) {
            char name[MAX_STR];
            printf("  new candidate name: "); fflush(stdout);
            if (!fgets(name, sizeof(name), stdin)) continue;
            name[strcspn(name, "\r\n")] = '\0';
            add_candidate(name);
        } else if (choice == 5) {
            election_reset();
        } else {
            printf("  invalid option.\n");
        }
    }
}

int room_election_cmd(int argc, char **argv) {
    if (argc < 2) {
        room_election_run();
        return 0;
    }

    const char *sub = argv[1];
    if (strcmp(sub, "run") == 0 || strcmp(sub, "tally") == 0) {
        election_tally();
        return 0;
    }
    if (strcmp(sub, "candidates") == 0 || strcmp(sub, "list") == 0) {
        list_candidates();
        return 0;
    }
    if (strcmp(sub, "vote") == 0) {
        if (argc < 3) {
            printf("  usage: election vote <candidate_name>\n");
            return 1;
        }
        cast_vote(argv[2]);
        return 0;
    }
    if (strcmp(sub, "add") == 0) {
        if (argc < 3) {
            printf("  usage: election add <candidate_name>\n");
            return 1;
        }
        add_candidate(argv[2]);
        return 0;
    }
    if (strcmp(sub, "reset") == 0) {
        election_reset();
        return 0;
    }

    printf("  unknown election subcommand '%s'. Try: run, vote, candidates, add, reset\n", sub);
    return 1;
}
