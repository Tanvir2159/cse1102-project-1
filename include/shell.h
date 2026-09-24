#ifndef SHELL_H
#define SHELL_H

void shell_boot(void);
void shell_run(void);
int  shell_dispatch(char *line);

const char *shell_get_leader(void);
void        shell_set_leader(const char *name);
int         shell_is_leader(const char *name);

#endif
