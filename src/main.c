#include "common.h"
#include "shell.h"

int main(int argc, char **argv) {
    if (argc > 1) {
        /* One-shot CLI mode: directly execute subcommands */
        return shell_execute(argc - 1, argv + 1);
    }

    /* Interactive REPL mode */
    shell_boot();
    shell_run();
    return 0;
}
