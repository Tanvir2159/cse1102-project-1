#include "common.h"
#include "room_records.h"

static int records_search(const char *query) {
    FILE *fp = fopen(FILE_USERS, "r");
    if (!fp) {
        printf("  no user index at %s\n", FILE_USERS);
        return -1;
    }

    char line[MAX_STR];
    int matches = 0;
    printf("\n  \033[1;36m--- User Records (Query: \"%s\") ---\033[0m\n", query);
    while (fgets(line, MAX_STR, fp)) {
        line[strcspn(line, "\r\n")] = '\0';
        if (line[0] == '\0') continue;
        if (strcasestr(line, query)) {
            printf("  \033[0;32m[match]\033[0m %s\n", line);
            matches++;
        }
    }
    fclose(fp);
    printf("  found \033[1;33m%d\033[0m match(es).\n\n", matches);
    return matches;
}

static void records_list(void) {
    FILE *fp = fopen(FILE_USERS, "r");
    if (!fp) {
        printf("  no user index at %s\n", FILE_USERS);
        return;
    }

    char line[MAX_STR];
    int count = 0;
    printf("\n  \033[1;36m--- All Registered Users ---\033[0m\n");
    while (fgets(line, MAX_STR, fp)) {
        line[strcspn(line, "\r\n")] = '\0';
        if (line[0] == '\0') continue;
        count++;
        printf("  %3d. %s\n", count, line);
    }
    fclose(fp);
    printf("  total users: \033[1;33m%d\033[0m\n\n", count);
}

static void records_add(const char *name) {
    if (!name || name[0] == '\0') {
        printf("  invalid name.\n");
        return;
    }

    /* Check if already exists */
    FILE *fp = fopen(FILE_USERS, "r");
    if (fp) {
        char line[MAX_STR];
        while (fgets(line, MAX_STR, fp)) {
            line[strcspn(line, "\r\n")] = '\0';
            if (strcasecmp(line, name) == 0) {
                printf("  user '\033[1;33m%s\033[0m' already exists in index.\n", name);
                fclose(fp);
                return;
            }
        }
        fclose(fp);
    }

    fp = fopen(FILE_USERS, "a");
    if (!fp) {
        printf("  failed to write to %s\n", FILE_USERS);
        return;
    }
    fprintf(fp, "%s\n", name);
    fclose(fp);
    printf("  user '\033[1;32m%s\033[0m' added to index.\n", name);
}

void room_records_run(void) {
    while (1) {
        printf("\n  \033[1;36m┌─ Records Room ───────────────┐\033[0m\n");
        printf("  │ 1. Search users by fragment  │\n");
        printf("  │ 2. List all users            │\n");
        printf("  │ 3. Add new user              │\n");
        printf("  │ 0. Return to Vault           │\n");
        printf("  \033[1;36m└──────────────────────────────┘\033[0m\n");
        printf("  choice: "); fflush(stdout);

        char buf[64];
        if (!fgets(buf, sizeof(buf), stdin)) break;
        int choice = atoi(buf);
        if (choice == 0) break;

        if (choice == 1) {
            char query[MAX_STR];
            printf("  search query: "); fflush(stdout);
            if (!fgets(query, sizeof(query), stdin)) continue;
            query[strcspn(query, "\r\n")] = '\0';
            records_search(query);
        } else if (choice == 2) {
            records_list();
        } else if (choice == 3) {
            char name[MAX_STR];
            printf("  new user name: "); fflush(stdout);
            if (!fgets(name, sizeof(name), stdin)) continue;
            name[strcspn(name, "\r\n")] = '\0';
            records_add(name);
        } else {
            printf("  invalid option.\n");
        }
    }
}

int room_records_cmd(int argc, char **argv) {
    if (argc < 2) {
        room_records_run();
        return 0;
    }

    const char *sub = argv[1];
    if (strcmp(sub, "search") == 0 || strcmp(sub, "find") == 0) {
        if (argc < 3) {
            printf("  usage: records search <query>\n");
            return 1;
        }
        records_search(argv[2]);
        return 0;
    }
    if (strcmp(sub, "list") == 0) {
        records_list();
        return 0;
    }
    if (strcmp(sub, "add") == 0) {
        if (argc < 3) {
            printf("  usage: records add <name>\n");
            return 1;
        }
        records_add(argv[2]);
        return 0;
    }

    /* If user passed a single argument that isn't a known subcommand, treat as search query */
    records_search(sub);
    return 0;
}
