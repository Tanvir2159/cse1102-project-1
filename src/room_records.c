#include "common.h"
#include "room_records.h"

/* DNA-flavored: load a data file, ask for a query, print every match. */
void room_records_run(void) {
    FILE *fp = fopen(FILE_USERS, "r");
    if (!fp) { printf("  no user index at %s\n", FILE_USERS); return; }

    char query[MAX_STR];
    printf("  search name: "); fflush(stdout);
    if (!fgets(query, MAX_STR, stdin)) { fclose(fp); return; }
    query[strcspn(query, "\n")] = '\0';

    char line[MAX_STR];
    int matches = 0;
    printf("\n  --- records room ---\n");
    while (fgets(line, MAX_STR, fp)) {
        line[strcspn(line, "\n")] = '\0';
        if (strstr(line, query)) {
            printf("  match: %s\n", line);
            matches++;
        }
    }
    fclose(fp);
    printf("  %d match(es) for \"%s\"\n\n", matches, query);
}
