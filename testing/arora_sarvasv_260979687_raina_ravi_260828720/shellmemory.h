#ifndef SHELLMEMORY_H
#define SHELLMEMORY_H

#include "pcb.h"

typedef struct memory_struct{
	char *var;
	char *value;
} mem_entry_t;

extern int FRAME_SIZE;
extern int VAR_MEM_SIZE;
extern int FREE_LIST_SIZE;
extern int SHELL_MEM_SIZE;

void mem_init();
mem_entry_t *mem_get_entry(int frame_number, int offset);

char *mem_get_value(char *var, pcb_t *pcb);
void mem_set_value(char *var, char *value, pcb_t *pcb);
int check_mem_value_exists(char *var_in, pcb_t *pcb);

int mem_load_frame(pcb_t *pcb, char **script_lines, int page_num);
void load_page(pcb_t *pcb, int page_num, char **page);

int mem_load_script_line(int pid, int line_number, char *script_line, mem_entry_t *mem);
int mem_load_script(FILE *script, pcb_t *pcb);
int mem_cleanup_script(pcb_t *pcb);
int mem_cleanup_frame(int frame_num);

#endif /* SHELLMEMORY_H */
