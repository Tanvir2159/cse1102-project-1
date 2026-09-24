#include "common.h"
#include "room_readability.h"
#include "shell.h"

static int count_all(const char *path,
                     int *letters, int *words, int *sentences) {
    FILE *fp = fopen(path, "r");
    if (!fp) return -1;

    int ch, in_word = 0;
    *letters = *words = *sentences = 0;

    while ((ch = fgetc(fp)) != EOF) {
        if (isalpha(ch)) (*letters)++;
        if (isspace(ch)) {
            if (in_word) { (*words)++; in_word = 0; }
        } else {
            in_word = 1;
        }
        if (ch == '.' || ch == '!' || ch == '?') (*sentences)++;
    }
    if (in_word) (*words)++;
    fclose(fp);
    return 0;
}

void room_readability_run(void) {
    char who[MAX_STR];
    printf("  identify yourself: "); fflush(stdout);
    if (!fgets(who, MAX_STR, stdin)) return;
    who[strcspn(who, "\n")] = '\0';

    if (!shell_is_leader(who)) {
        printf("  \033[0;31monly the elected leader may judge readability.\033[0m\n");
        printf("  run 'election' first.\n");
        return;
    }

    int l, w, s;
    if (count_all(FILE_SAMPLE, &l, &w, &s) != 0) {
        printf("  no sample text at %s\n", FILE_SAMPLE);
        return;
    }
    if (w == 0 || s == 0) { printf("  not enough text.\n"); return; }

    float L = (float)l / w * 100.0f;
    float S = (float)s / w * 100.0f;
    float grade = 0.0588f * L - 0.296f * S - 15.8f;

    printf("\n  --- readability room (leader: %s) ---\n", who);
    printf("  letters %d  words %d  sentences %d\n", l, w, s);
    printf("  L %.2f  S %.2f\n", L, S);
    printf("  \033[1;33mgrade index: %.2f\033[0m\n\n", grade);
}
