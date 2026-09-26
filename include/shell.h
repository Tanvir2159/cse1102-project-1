#ifndef SHELL_H
#define SHELL_H

void shell_boot(void);
void shell_run(void);
int  shell_execute(int argc, char **argv);
int  shell_dispatch(char *line);

const char *shell_get_leader(void);
void        shell_set_leader(const char *name);
int         shell_is_leader(const char *name);

/* History management */
void        shell_add_history(const char *cmd);
void        shell_show_history(void);
const char *shell_get_history(int index);

#endif
