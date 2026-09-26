#include "common.h"
#include "room_ledger.h"

static int load_ledger(LedgerEntry *entries, int max_entries) {
    FILE *fp = fopen(FILE_LEDGER, "rb");
    if (!fp) return 0;

    int count = 0;
    if (fread(&count, sizeof(int), 1, fp) != 1) {
        fclose(fp);
        return 0;
    }
    if (count > max_entries) count = max_entries;
    int read = fread(entries, sizeof(LedgerEntry), count, fp);
    fclose(fp);
    return read;
}

static int save_ledger(const LedgerEntry *entries, int count) {
    FILE *out = fopen(FILE_LEDGER, "wb");
    if (!out) return -1;
    fwrite(&count, sizeof(int), 1, out);
    fwrite(entries, sizeof(LedgerEntry), count, out);
    fclose(out);
    return 0;
}

static void ledger_list(void) {
    LedgerEntry entries[MAX_RECORDS];
    int count = load_ledger(entries, MAX_RECORDS);

    printf("\n  \033[1;36m--- Active Ledger Entries ---\033[0m\n");
    if (count == 0) {
        printf("  (ledger is empty)\n\n");
        return;
    }

    float total_volume = 0.0f;
    for (int i = 0; i < count; i++) {
        printf("  %2d. \033[1;33m%-12s\033[0m owes \033[1;32m%-12s\033[0m $%.2f\n",
               i + 1, entries[i].ower, entries[i].payer, entries[i].amount);
        total_volume += entries[i].amount;
    }
    printf("  \033[1;37mtotal volume:\033[0m $%.2f across %d transaction(s)\n\n", total_volume, count);
}

static void ledger_add(const char *payer, const char *ower, float amount) {
    if (amount <= 0.0f) {
        printf("  \033[0;31merror:\033[0m amount must be positive.\n");
        return;
    }
    if (strcasecmp(payer, ower) == 0) {
        printf("  \033[0;31merror:\033[0m payer and ower cannot be the same person.\n");
        return;
    }

    LedgerEntry entries[MAX_RECORDS];
    int count = load_ledger(entries, MAX_RECORDS);
    if (count >= MAX_RECORDS) {
        printf("  \033[0;31merror:\033[0m ledger capacity full.\n");
        return;
    }

    /* Lab 9: pointer to slot */
    LedgerEntry *e = &entries[count];
    strncpy(e->payer, payer, MAX_STR - 1);
    e->payer[MAX_STR - 1] = '\0';
    strncpy(e->ower, ower, MAX_STR - 1);
    e->ower[MAX_STR - 1] = '\0';
    e->amount = amount;
    count++;

    if (save_ledger(entries, count) == 0) {
        printf("  saved entry: \033[1;33m%s\033[0m owes \033[1;32m%s\033[0m $%.2f\n",
               ower, payer, amount);
    } else {
        printf("  \033[0;31merror:\033[0m failed to write to binary ledger file.\n");
    }
}

static void ledger_settle(void) {
    LedgerEntry entries[MAX_RECORDS];
    int count = load_ledger(entries, MAX_RECORDS);
    if (count == 0) {
        printf("  ledger is empty, nothing to settle.\n");
        return;
    }

    /* Collect unique people and calculate net balance: paid - owed */
    char people[MAX_RECORDS][MAX_STR];
    float balances[MAX_RECORDS] = {0};
    int num_people = 0;

    for (int i = 0; i < count; i++) {
        /* check payer */
        int p_idx = -1;
        for (int j = 0; j < num_people; j++) {
            if (strcasecmp(people[j], entries[i].payer) == 0) { p_idx = j; break; }
        }
        if (p_idx == -1 && num_people < MAX_RECORDS) {
            strncpy(people[num_people], entries[i].payer, MAX_STR - 1);
            people[num_people][MAX_STR - 1] = '\0';
            p_idx = num_people++;
        }

        /* check ower */
        int o_idx = -1;
        for (int j = 0; j < num_people; j++) {
            if (strcasecmp(people[j], entries[i].ower) == 0) { o_idx = j; break; }
        }
        if (o_idx == -1 && num_people < MAX_RECORDS) {
            strncpy(people[num_people], entries[i].ower, MAX_STR - 1);
            people[num_people][MAX_STR - 1] = '\0';
            o_idx = num_people++;
        }

        if (p_idx >= 0) balances[p_idx] += entries[i].amount;
        if (o_idx >= 0) balances[o_idx] -= entries[i].amount;
    }

    printf("\n  \033[1;36m--- Net Settlement Balances ---\033[0m\n");
    for (int i = 0; i < num_people; i++) {
        if (balances[i] > 0.001f) {
            printf("  \033[1;32m%-16s\033[0m should receive \033[0;32m+$%.2f\033[0m\n",
                   people[i], balances[i]);
        } else if (balances[i] < -0.001f) {
            printf("  \033[1;31m%-16s\033[0m needs to pay   \033[0;31m-$%.2f\033[0m\n",
                   people[i], -balances[i]);
        } else {
            printf("  \033[0;37m%-16s\033[0m settled (balance $0.00)\n", people[i]);
        }
    }
    printf("\n");
}

static void ledger_clear(void) {
    int count = 0;
    FILE *out = fopen(FILE_LEDGER, "wb");
    if (out) {
        fwrite(&count, sizeof(int), 1, out);
        fclose(out);
        printf("  binary ledger cleared.\n");
    } else {
        printf("  failed to clear ledger.\n");
    }
}

void room_ledger_run(void) {
    while (1) {
        printf("\n  \033[1;36m┌─ Ledger Room ────────────────┐\033[0m\n");
        printf("  │ 1. List transactions         │\n");
        printf("  │ 2. Add transaction           │\n");
        printf("  │ 3. Settle net balances       │\n");
        printf("  │ 4. Clear all entries         │\n");
        printf("  │ 0. Return to Vault           │\n");
        printf("  \033[1;36m└──────────────────────────────┘\033[0m\n");
        printf("  choice: "); fflush(stdout);

        char buf[64];
        if (!fgets(buf, sizeof(buf), stdin)) break;
        int choice = atoi(buf);
        if (choice == 0) break;

        if (choice == 1) {
            ledger_list();
        } else if (choice == 2) {
            char payer[MAX_STR], ower[MAX_STR], amt_str[32];
            printf("  payer (who paid): "); fflush(stdout);
            if (!fgets(payer, sizeof(payer), stdin)) continue;
            payer[strcspn(payer, "\r\n")] = '\0';

            printf("  ower (who owes): "); fflush(stdout);
            if (!fgets(ower, sizeof(ower), stdin)) continue;
            ower[strcspn(ower, "\r\n")] = '\0';

            printf("  amount: "); fflush(stdout);
            if (!fgets(amt_str, sizeof(amt_str), stdin)) continue;
            float amount = atof(amt_str);

            ledger_add(payer, ower, amount);
        } else if (choice == 3) {
            ledger_settle();
        } else if (choice == 4) {
            ledger_clear();
        } else {
            printf("  invalid choice.\n");
        }
    }
}

int room_ledger_cmd(int argc, char **argv) {
    if (argc < 2) {
        room_ledger_run();
        return 0;
    }

    const char *sub = argv[1];
    if (strcmp(sub, "list") == 0) {
        ledger_list();
        return 0;
    }
    if (strcmp(sub, "settle") == 0) {
        ledger_settle();
        return 0;
    }
    if (strcmp(sub, "clear") == 0) {
        ledger_clear();
        return 0;
    }
    if (strcmp(sub, "add") == 0) {
        if (argc < 5) {
            printf("  usage: ledger add <payer> <ower> <amount>\n");
            return 1;
        }
        float amt = atof(argv[4]);
        ledger_add(argv[2], argv[3], amt);
        return 0;
    }

    printf("  unknown ledger subcommand '%s'. Try: list, add, settle, clear\n", sub);
    return 1;
}
