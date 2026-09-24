#include "common.h"
#include "room_attendance.h"

void room_attendance_run(void) {
    FILE *fp = fopen(FILE_ATTEND, "r");
    if (!fp) { printf("  no attendance file at %s\n", FILE_ATTEND); return; }

    AttendanceRecord recs[MAX_RECORDS];
    int count = 0;
    char line[MAX_LINE];

    while (fgets(line, MAX_LINE, fp) && count < MAX_RECORDS) {
        line[strcspn(line, "\n")] = '\0';
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

    int present = 0;
    printf("\n  --- attendance room ---\n");
    for (int i = 0; i < count; i++) {
        printf("  %-20s %s\n", recs[i].name,
               recs[i].present ? "\033[0;32mpresent\033[0m"
                               : "\033[0;31mabsent \033[0m");
        present += recs[i].present;
    }
    printf("  total: %d / %d present\n\n", present, count);
}
