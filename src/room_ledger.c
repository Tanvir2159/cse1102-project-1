#include "common.h"
#include "room_ledger.h"

void room_ledger_run(void) {
    LedgerEntry entries[MAX_RECORDS];
    int count = 0;

    FILE *fp = fopen(FILE_LEDGER, "rb");
    if (fp) {
        fread(&count, sizeof(int), 1, fp);
        if (count > MAX_RECORDS) count = MAX_RECORDS;
        fread(entries, sizeof(LedgerEntry), count, fp);
        fclose(fp);
    }

    printf("\n  --- ledger room ---\n");
    printf("  1. list entries\n");
    printf("  2. add entry\n");
    printf("  3. settle all\n");
    printf("  choice: ");
    int c;
    if (scanf("%d", &c) != 1) { while (getchar() != '\n'); return; }
    while (getchar() != '\n');

    if (c == 1) {
        if (count == 0) { printf("  (empty)\n"); }
        for (int i = 0; i < count; i++) {
            printf("  %-12s owes %-12s %.2f\n",
                   entries[i].ower, entries[i].payer, entries[i].amount);
        }
    } else if (c == 2 && count < MAX_RECORDS) {
        /* Lab 9: pointer to the slot we will write into */
        LedgerEntry *e = &entries[count];
        printf("  payer: ");  fgets(e->payer, MAX_STR, stdin);
        e->payer[strcspn(e->payer, "\n")] = '\0';
        printf("  ower : ");  fgets(e->ower, MAX_STR, stdin);
        e->ower[strcspn(e->ower, "\n")] = '\0';
        printf("  amount: "); scanf("%f", &e->amount);
        while (getchar() != '\n');
        count++;

        FILE *out = fopen(FILE_LEDGER, "wb");
        if (out) {
            fwrite(&count, sizeof(int), 1, out);
            fwrite(entries, sizeof(LedgerEntry), count, out);
            fclose(out);
        }
        printf("  saved.\n");
    } else if (c == 3) {
        /* Lab 9: pointer arithmetic over the array */
        LedgerEntry *p   = entries;
        LedgerEntry *end = entries + count;
        printf("  net balances:\n");
        for (; p < end; p++) {
            printf("  %-12s paid %.2f\n", p->payer, p->amount);
        }
    }
    printf("\n");
}
