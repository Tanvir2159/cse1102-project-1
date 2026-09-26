#include "common.h"
#include "room_attendance.h"

static int load_attendance(AttendanceRecord *recs, int max_recs) {
    FILE *fp = fopen(FILE_ATTEND, "r");
    if (!fp) return 0;

    int count = 0;
    char line[MAX_LINE];
    while (fgets(line, MAX_LINE, fp) && count < max_recs) {
        line[strcspn(line, "\r\n")] = '\0';
        if (line[0] == '\0') continue;

        char *space = strrchr(line, ' ');
        if (!space) continue;
        *space = '\0';
        char status = space[1];

        strncpy(recs[count].name, line, MAX_STR - 1);
        recs[count].name[MAX_STR - 1] = '\0';
        recs[count].present = (status == 'P' || status == 'p') ? 1 : 0;
        count++;
    }
    fclose(fp);
    return count;
}

static int save_attendance(const AttendanceRecord *recs, int count) {
    FILE *fp = fopen(FILE_ATTEND, "w");
    if (!fp) return -1;
    for (int i = 0; i < count; i++) {
        fprintf(fp, "%s %c\n", recs[i].name, recs[i].present ? 'P' : 'A');
    }
    fclose(fp);
    return 0;
}

static void display_attendance(void) {
    AttendanceRecord recs[MAX_RECORDS];
    int count = load_attendance(recs, MAX_RECORDS);
    if (count == 0) {
        printf("  no records found in %s\n", FILE_ATTEND);
        return;
    }

    int present = 0;
    printf("\n  \033[1;36m--- Attendance Roster ---\033[0m\n");
    for (int i = 0; i < count; i++) {
        printf("  %-20s %s\n", recs[i].name,
               recs[i].present ? "\033[0;32mpresent\033[0m"
                               : "\033[0;31mabsent \033[0m");
        present += recs[i].present;
    }
    float pct = count > 0 ? ((float)present / (float)count) * 100.0f : 0.0f;
    printf("  \033[1;37mtotal:\033[0m %d / %d present (%.1f%%)\n\n", present, count, pct);
}

static void mark_attendance(const char *name, char status) {
    AttendanceRecord recs[MAX_RECORDS];
    int count = load_attendance(recs, MAX_RECORDS);
    int is_present = (status == 'P' || status == 'p') ? 1 : 0;
    int found = 0;

    for (int i = 0; i < count; i++) {
        if (strcasecmp(recs[i].name, name) == 0) {
            recs[i].present = is_present;
            found = 1;
            break;
        }
    }

    if (!found) {
        if (count >= MAX_RECORDS) {
            printf("  \033[0;31merror:\033[0m attendance registry is full.\n");
            return;
        }
        strncpy(recs[count].name, name, MAX_STR - 1);
        recs[count].name[MAX_STR - 1] = '\0';
        recs[count].present = is_present;
        count++;
    }

    if (save_attendance(recs, count) == 0) {
        printf("  marked \033[1;33m%s\033[0m as %s.\n",
               name, is_present ? "\033[0;32mpresent\033[0m" : "\033[0;31mabsent\033[0m");
    } else {
        printf("  \033[0;31merror:\033[0m failed to save attendance.\n");
    }
}

static void show_summary(void) {
    AttendanceRecord recs[MAX_RECORDS];
    int count = load_attendance(recs, MAX_RECORDS);
    int present = 0;
    for (int i = 0; i < count; i++) present += recs[i].present;
    int absent = count - present;
    float pct = count > 0 ? ((float)present / (float)count) * 100.0f : 0.0f;

    printf("\n  \033[1;36m┌─ Attendance Summary ─────────┐\033[0m\n");
    printf("  │ Total Enrolled : %-12d│\n", count);
    printf("  │ Present        : %-12d│\n", present);
    printf("  │ Absent         : %-12d│\n", absent);
    printf("  │ Rate           : %-10.1f%% │\n", pct);
    printf("  \033[1;36m└──────────────────────────────┘\033[0m\n\n");
}

void room_attendance_run(void) {
    while (1) {
        printf("\n  \033[1;36m┌─ Attendance Room ────────────┐\033[0m\n");
        printf("  │ 1. View attendance roster    │\n");
        printf("  │ 2. Mark student status       │\n");
        printf("  │ 3. View summary metrics      │\n");
        printf("  │ 0. Return to Vault           │\n");
        printf("  \033[1;36m└──────────────────────────────┘\033[0m\n");
        printf("  choice: "); fflush(stdout);

        char buf[64];
        if (!fgets(buf, sizeof(buf), stdin)) break;
        int choice = atoi(buf);
        if (choice == 0) break;

        if (choice == 1) {
            display_attendance();
        } else if (choice == 2) {
            char name[MAX_STR];
            char st_buf[16];
            printf("  student name: "); fflush(stdout);
            if (!fgets(name, sizeof(name), stdin)) continue;
            name[strcspn(name, "\r\n")] = '\0';

            printf("  status (P for present, A for absent): "); fflush(stdout);
            if (!fgets(st_buf, sizeof(st_buf), stdin)) continue;
            mark_attendance(name, st_buf[0]);
        } else if (choice == 3) {
            show_summary();
        } else {
            printf("  invalid option.\n");
        }
    }
}

int room_attendance_cmd(int argc, char **argv) {
    if (argc < 2) {
        room_attendance_run();
        return 0;
    }

    const char *sub = argv[1];
    if (strcmp(sub, "list") == 0) {
        display_attendance();
        return 0;
    }
    if (strcmp(sub, "summary") == 0) {
        show_summary();
        return 0;
    }
    if (strcmp(sub, "mark") == 0) {
        if (argc < 4) {
            printf("  usage: attendance mark <student_name> <P|A>\n");
            return 1;
        }
        mark_attendance(argv[2], argv[3][0]);
        return 0;
    }

    printf("  unknown subcommand '%s'. Try: list, mark, summary\n", sub);
    return 1;
}
