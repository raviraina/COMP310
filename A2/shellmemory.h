#ifndef SHELLMEMORY_H
#define SHELLMEMORY_H

void mem_init();
char *mem_get_value(char *var);
void mem_set_value(char *var, char *value);
int check_mem_value_exists(char *var_in);

struct memory_struct{
	char *var;
	char *value;
};

#endif /* SHELLMEMORY_H */
