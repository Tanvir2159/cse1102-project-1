#include "common.h"
#include "room_readability.h"
#include "shell.h"

static void compute_grade(int letters, int words, int sentences, const char *source_name, const char *leader_name) {
    if (words == 0 || sentences == 0) {
        printf("  not enough text to analyze.\n");
        return;
    }

    float L = ((float)letters / words) * 100.0f;
    float S = ((float)sentences / words) * 100.0f;
    float grade = 0.0588f * L - 0.296f * S - 15.8f;

    printf("\n  \033[1;36m--- Readability Analysis (%s) ---\033[0m\n", source_name);
    printf("  Authorized Leader : \033[1;33m%s\033[0m\n", leader_name);
    printf("  Letters           : %d\n", letters);
    printf("  Words             : %d\n", words);
    printf("  Sentences         : %d\n", sentences);
    printf("  Letters / 100 w   : %.2f\n", L);
    printf("  Sentences / 100 w : %.2f\n", S);
    printf("  \033[1;32mColeman-Liau Grade: %.2f\033[0m\n", grade);
    if (grade >= 16.0f)
        printf("  Reading Level     : Graduate / Professional\n\n");
    else if (grade >= 12.0f)
        printf("  Reading Level     : Undergraduate / College\n\n");
    else if (grade >= 9.0f)
        printf("  Reading Level     : High School\n\n");
    else
        printf("  Reading Level     : Elementary / Middle School\n\n");
}

static int count_from_file(const char *path, int *letters, int *words, int *sentences) {
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

static void count_from_string(const char *text, int *letters, int *words, int *sentences) {
    int in_word = 0;
    *letters = *words = *sentences = 0;

    for (int i = 0; text[i] != '\0'; i++) {
        unsigned char ch = (unsigned char)text[i];
        if (isalpha(ch)) (*letters)++;
        if (isspace(ch)) {
            if (in_word) { (*words)++; in_word = 0; }
        } else {
            in_word = 1;
        }
        if (ch == '.' || ch == '!' || ch == '?') (*sentences)++;
    }
    if (in_word) (*words)++;
}

static int verify_leader_prompt(char *out_leader) {
    const char *current_leader = shell_get_leader();
    if (!current_leader || current_leader[0] == '\0') {
        printf("  \033[0;31merror:\033[0m no vault leader has been elected yet.\n");
        printf("  run 'election run' first.\n");
        return 0;
    }

    char who[MAX_STR];
    printf("  identify yourself (leader): "); fflush(stdout);
    if (!fgets(who, MAX_STR, stdin)) return 0;
    who[strcspn(who, "\r\n")] = '\0';

    if (!shell_is_leader(who)) {
        printf("  \033[0;31maccess denied:\033[0m only elected leader '%s' may access readability.\n", current_leader);
        return 0;
    }
    strncpy(out_leader, who, MAX_STR - 1);
    out_leader[MAX_STR - 1] = '\0';
    return 1;
}

void room_readability_run(void) {
    char leader[MAX_STR];
    if (!verify_leader_prompt(leader)) return;

    int l, w, s;
    if (count_from_file(FILE_SAMPLE, &l, &w, &s) != 0) {
        printf("  no sample text at %s\n", FILE_SAMPLE);
        return;
    }
    compute_grade(l, w, s, FILE_SAMPLE, leader);
}

int room_readability_cmd(int argc, char **argv) {
    if (argc < 2) {
        room_readability_run();
        return 0;
    }

    const char *sub = argv[1];
    char leader[MAX_STR];
    if (!verify_leader_prompt(leader)) return 1;

    if (strcmp(sub, "file") == 0) {
        const char *filepath = (argc >= 3) ? argv[2] : FILE_SAMPLE;
        int l, w, s;
        if (count_from_file(filepath, &l, &w, &s) != 0) {
            printf("  cannot open file: %s\n", filepath);
            return 1;
        }
        compute_grade(l, w, s, filepath, leader);
        return 0;
    }

    if (strcmp(sub, "text") == 0) {
        if (argc < 3) {
            printf("  usage: readability text \"<string of text>\"\n");
            return 1;
        }
        int l, w, s;
        count_from_string(argv[2], &l, &w, &s);
        compute_grade(l, w, s, "Custom Text Input", leader);
        return 0;
    }

    printf("  unknown readability subcommand '%s'. Try: file [path], text \"<str>\"\n", sub);
    return 1;
}
