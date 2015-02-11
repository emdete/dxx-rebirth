/*
 * This file is part of the DXX-Rebirth project <http://www.dxx-rebirth.com/>.
 * It is copyright by its individual contributors, as recorded in the
 * project's Git history.  See COPYING.txt at the top level for license
 * terms and a link to the Git history.
 *
 */
/*
 *
 * Command parsing and processing
 *
 */

#ifndef _CMD_H_
#define _CMD_H_ 1


void cmd_init(void);

/* Maximum length for a single command */
#define CMD_MAX_LENGTH 2048
/* Maximum number of tokens per command */
#define CMD_MAX_TOKENS 64

/* Add some commands to the queue to be executed */
void cmd_enqueue(int insert, char *input);
void cmd_enqueuef(int insert, char *fmt, ...);
#define cmd_append(input) cmd_enqueue(0, (input))
#define cmd_appendf(...) cmd_enqueuef(0, __VA_ARGS__)
#define cmd_insert(input) cmd_enqueue(1, (input))
#define cmd_insertf(...) cmd_enqueuef(1, __VA_ARGS__)

/* Execute pending commands */
int cmd_queue_process(void);

/* execute until there are no commands left */
void cmd_queue_flush(void);

/* Attempt to autocomplete an input string */
char *cmd_complete(char *input);

typedef void (*cmd_handler_t)(int argc, char *argv[]);

void cmd_addcommand(char *cmd_name, cmd_handler_t cmd_func);


#endif /* _CMD_H_ */
