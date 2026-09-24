#include "common.h"
#include "shell.h"

int main(void) {
    shell_boot();
    shell_run();
    return 0;
}
