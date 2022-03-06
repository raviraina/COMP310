#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <regex.h>
#include "shellmemory.h"
#include "pcb.h"

/*
* first 100 places in shell memory are reserved for variables
* the remaining memory is used for loading scripts
*/
struct memory_struct shellmemory[1000];


// initialize shell memory with all variables and respective values as "none"
void mem_init(){
	int i;
	for (i=0; i<1000; i++){		
		shellmemory[i].var = "none";
		shellmemory[i].value = "none";
	}
}



// Shell memory functions for variables

// assign a value to a variable
void mem_set_value(char *var_in, char *value_in, pcb_t *pcb) {
	int i;
	char var[100];

	if (pcb != NULL) {
		snprintf(var, 100, "%d-%s", pcb->pid, var_in);
	}
	else {
		strcpy(var, var_in);
	}

	for (i=0; i<100; i++){
		if (strcmp(shellmemory[i].var, var) == 0){
			shellmemory[i].value = strdup(value_in);
			return;
		} 
	}

	//Value does not exist, need to find a free spot.
	for (i=0; i<100; i++){
		if (strcmp(shellmemory[i].var, "none") == 0){
			shellmemory[i].var = strdup(var);
			shellmemory[i].value = strdup(value_in);
			return;
		} 
	}

	return;

}

//get value based on input key (variable)
char *mem_get_value(char *var_in, pcb_t *pcb) {
	int i;
	char var[100];

	if (pcb != NULL) {
		snprintf(var, 100, "%d-%s", pcb->pid, var_in);
	}
	else {
		strcpy(var, var_in);
	}

	for (i=0; i<100; i++){
		if (strcmp(shellmemory[i].var, var) == 0){

			return strdup(shellmemory[i].value);
		} 
	}
	return "Variable does not exist";

}

// check if a variable exists in shell memory
int check_mem_value_exists(char *var_in, pcb_t *pcb) {
	int i;
	char var[100];
	
	if (pcb != NULL) {
		snprintf(var, 100, "%d-%s", pcb->pid, var_in);
	}
	else {
		strcpy(var, var_in);
	}

	for (i=0; i<100; i++){
		if (strcmp(shellmemory[i].var, var) == 0){

			return 1;
		}
	}

	return 0;
}



// Shell memory functions for loading scripts

// returns -1 if error, 0 if success
int mem_load_script_line(int pid, int line_number, char *script_line, struct memory_struct *mem) {
	int i;
	char key[10];
	if(snprintf(key, 10, "%d-%d", pid, line_number) < 0) { 
		printf("Error: unable to load line %d of process %d\n", line_number, pid);
		return -1;
	}
	mem->var = strdup(key);
	mem->value = strdup(script_line);
	// printf("var: %s, value: %s\n", mem->var, mem->value);
	return 0;
}


// load script into memory
// returns -1 if error, 0 if success
int mem_load_script(FILE *script, pcb_t *pcb) {
	char line[1000];
	char c;
	int i, j, base, line_num=1;

	// calculate the size of script - number of lines
	pcb->size = 0;
	while(fgets(line, 1000, script) != NULL)
		pcb->size++;

	// get back to the beginning of script
	rewind(script); 

	// find a spot in shell memory suitable for loading script
	// if no spot is found, return -1
	for (i = 100; i < 1000; i++) {
		base = i;
		// find an empty spot
		if (strcmp(shellmemory[i].var, "none") == 0) {
			// now check whether the next script_size spots are also empty
			for (j = i+1; j < (i + pcb->size); j++) {
				if (strcmp(shellmemory[j].var, "none") != 0) {
					i = j+1;
					break;
				}
			}

			if (i < j) { // found a spot
				//update pcb base
				pcb->base = &shellmemory[base];
				pcb->pc = &shellmemory[base];

				// load each line of script into the designated shell memory
				while(fgets(line, 1000, script) != NULL && base < j) {
					if(mem_load_script_line(pcb->pid, line_num++, line, &shellmemory[base++]) != 0)
						return -1;
				}		
				return 0;
			}
		}
	}
	return -1;
}



int mem_cleanup_script(pcb_t *pcb) {
	char *var;
	int pid;
	
	// clean up script from shell memory
	for (int i = 0; i < pcb->size; i++) {
		(pcb->base + i)->var = "none";
		(pcb->base + i)->value = "none";
	}

	// create regex to identify variables associated with a process
	regex_t re;
	regcomp(&re, "^[0-9]+-[a-zA-Z0-9]+$", 0);

	// clean up script variables from shell memory
	for (int i = 0; i < 100; i++) {
		if (regexec(&re, shellmemory[i].var, 0, NULL, 0) == 0) {
			sscanf(shellmemory[i].var, "%d-%s", &pid, var);
			// check if the variable is associated with the given process
			if (pcb->pid == pid) {
				shellmemory[i].var = "none";
				shellmemory[i].value = "none";
			}
		}
	}

	// free the memory allocated to pcb
	free(pcb);

	return 0;
}