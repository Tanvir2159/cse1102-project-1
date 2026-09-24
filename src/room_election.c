#include "common.h"
#include "room_election.h"
#include "shell.h"

/* Loads candidates from data/candidates.txt (one per line).
   Loads votes from data/votes.txt (one candidate name per line).
   Runoff: eliminate lowest until someone has > half. */
void room_election_run(void) {
    Candidate cands[MAX_RECORDS];
    int n = 0;
    char line[MAX_LINE];

    FILE *cf = fopen(FILE_CANDID, "r");
    if (!cf) { printf("  no candidates file.\n"); return; }
    while (fgets(line, MAX_LINE, cf) && n < MAX_RECORDS) {
        line[strcspn(line, "\n")] = '\0';
        if (line[0] == '\0') continue;
        strncpy(cands[n].name, line, MAX_STR - 1);
        cands[n].name[MAX_STR - 1] = '\0';
        cands[n].votes = 0;
        cands[n].eliminated = 0;
        n++;
    }
    fclose(cf);
    if (n == 0) { printf("  no candidates.\n"); return; }

    FILE *vf = fopen(FILE_VOTES, "r");
    if (!vf) { printf("  no votes file.\n"); return; }
    int total = 0;
    while (fgets(line, MAX_LINE, vf)) {
        line[strcspn(line, "\n")] = '\0';
        for (int i = 0; i < n; i++) {
            if (strcmp(cands[i].name, line) == 0) {
                cands[i].votes++; total++; break;
            }
        }
    }
    fclose(vf);
    if (total == 0) { printf("  no votes cast.\n"); return; }

    printf("\n  --- election room ---\n");
    while (1) {
        int active = 0, best = -1;
        for (int i = 0; i < n; i++) {
            if (cands[i].eliminated) continue;
            active++;
            if (cands[i].votes > best) best = cands[i].votes;
        }
        if (active == 1 || best * 2 > total) break;

        int low = -1;
        for (int i = 0; i < n; i++) {
            if (cands[i].eliminated) continue;
            if (low < 0 || cands[i].votes < cands[low].votes) low = i;
        }
        printf("  eliminating %s (%d votes)\n",
               cands[low].name, cands[low].votes);
        cands[low].eliminated = 1;
    }

    int winner = -1, best = -1;
    for (int i = 0; i < n; i++) {
        if (cands[i].eliminated) continue;
        if (cands[i].votes > best) { best = cands[i].votes; winner = i; }
    }

    if (winner >= 0) {
        printf("  \033[1;33mwinner: %s (%d / %d votes)\033[0m\n\n",
               cands[winner].name, cands[winner].votes, total);
        shell_set_leader(cands[winner].name);
        printf("  %s is now the vault leader.\n\n", cands[winner].name);
    }
}
