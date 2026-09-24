#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

/* ---------- App identity (Lab 11) ---------- */
#define APP_NAME     "VAULT"
#define APP_VERSION  "1.0"
#define PROMPT       "vault> "
#define MAX_LINE     256
#define MAX_STR      64
#define MAX_RECORDS  128

/* ---------- Data file paths ---------- */
#define DATA_DIR        "data/"
#define FILE_ATTEND     DATA_DIR "attendance.txt"
#define FILE_CANDID     DATA_DIR "candidates.txt"
#define FILE_VOTES      DATA_DIR "votes.txt"
#define FILE_USERS      DATA_DIR "users.txt"
#define FILE_SAMPLE     DATA_DIR "sample.txt"
#define FILE_LEDGER     DATA_DIR "ledger.dat"
#define FILE_LEADER     DATA_DIR "leader.txt"

/* ---------- Function-like macros (Lab 4 + 11) ---------- */
#define MAX(a,b)        ((a) > (b) ? (a) : (b))
#define MIN(a,b)        ((a) < (b) ? (a) : (b))
#define ARRAY_LEN(a)    (sizeof(a) / sizeof((a)[0]))

/* ---------- Debug (Lab 11 conditional compilation) ---------- */
#define DEBUG 1
#if DEBUG
    #define LOG(fmt, ...) fprintf(stderr, "[vault] " fmt "\n", ##__VA_ARGS__)
#else
    #define LOG(fmt, ...) ((void)0)
#endif

/* ---------- Cross-platform clear (Lab 11) ---------- */
#ifdef _WIN32
    #define CLEAR "cls"
#else
    #define CLEAR "clear"
#endif

/* ---------- Structs (Lab 3) ---------- */
typedef struct {
    char name[MAX_STR];
    int  present;
} AttendanceRecord;

typedef struct {
    char name[MAX_STR];
    int  votes;
    int  eliminated;
} Candidate;

typedef struct {
    char payer[MAX_STR];
    char ower[MAX_STR];
    float amount;
} LedgerEntry;

#endif /* COMMON_H */
