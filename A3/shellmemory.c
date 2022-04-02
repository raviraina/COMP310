#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <regex.h>
#include <assert.h>
#include "shellmemory.h"
#include "pcb.h"


const int FRAME_SIZE = 3; // size of each frame in the shellmemory
const int VAR_MEM_SIZE = 100; // part of shellmemory to store variables
const int FREE_LIST_SIZE = (int) (1000 - VAR_MEM_SIZE) / FRAME_SIZE; // size of the free list

/*
* first 100 places in shell memory are reserved for variables
* the remaining memory is used for loading scripts
*/
struct memory_struct shellmemory[1000];

/* 
* free_list to keep track of holes in shell memory
* index i of free_list corresponds to VAR_MEM_SIZE + (i * FRAME_SIZE) in shell memory
*/
int free_list[FREE_LIST_SIZE];


// initialize shell memory with all variables and respective values as "none"
void mem_init(){
	int i;
	// mark each spot in shellmemory as empty
	for (i=0; i<1000; i++){		
		shellmemory[i].var = "none";
		shellmemory[i].value = "none";
	}
	// mark each frame as free
	for (i = 0; i < FREE_LIST_SIZE; i ++) {
		free_list[i] = 1;
	}
}


// get the shellmemory struct for a given frame and offset
struct memory_struct *mem_get_entry(int frame_number, int offset){
	assert(offset >= 0 && offset < FRAME_SIZE);
	return &shellmemory[VAR_MEM_SIZE + (frame_number * FRAME_SIZE)] + offset;
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

	for (i=0; i<VAR_MEM_SIZE; i++){
		if (strcmp(shellmemory[i].var, var) == 0){
			shellmemory[i].value = strdup(value_in);
			return;
		} 
	}

	//Value does not exist, need to find a free spot.
	for (i=0; i<VAR_MEM_SIZE; i++){
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
	free(script_line);

	return 0;
}


// Loads a frame in shellmemory; returns -1 if error, 0 if success
int mem_load_frame(int pid, int line_number, char **script_lines, int frame_num) {
	int err = 0;
	struct memory_struct *mem = NULL;

	for (int i = 0; i < FRAME_SIZE; i++) {
		mem = mem_get_entry(frame_num, i);
		err = mem_load_script_line(pid, line_number++, script_lines[i], mem);
		if (err == -1) return err;
	}
	return err;
}


// Loads a script into memory
// returns -1 if error, 0 if success
int mem_load_script(FILE *script, pcb_t *pcb) {
	char line[1000]; // single line read from the script
	char *page[FRAME_SIZE]; // a set of lines to be loaded into shellmemory
	char c;
	int i, j, k=0, line_num=1;

	// calculate the size of script - number of lines
	pcb->size = 0;
	while(fgets(line, 1000, script) != NULL)
		pcb->size++;

	// initialize the initial job length score to script size
	pcb->jls = pcb->size;

	// allocate memory for the pcb page table
	pcb->num_pages = pcb->size%FRAME_SIZE == 0 ? pcb->size/FRAME_SIZE : ((int) pcb->size/FRAME_SIZE) + 1;
	pcb->page_table = calloc(pcb->num_pages, sizeof(int));

	// get back to the beginning of script
	rewind(script); 

	// find empty frames in shellmemory and load script pages into them
	for (i = 0; i < FREE_LIST_SIZE && k < pcb->num_pages; i++) {
		j = 0;

		// find an empty spot
		if (free_list[i] == 1) {
			// mark the spot as taken
			free_list[i] = 0;

			// load one page from the script
			while (j < FRAME_SIZE) {
				if(fgets(line, 1000, script) != NULL) {
					page[j++] = strdup(line);
				} else {
					page[j++] = strdup("\0");
				}
			}
			
			// load the page into shellmemory
			if (mem_load_frame(pcb->pid, line_num, page, i) != 0) return -1;

			// increment the line number
			line_num += FRAME_SIZE;

			// update pcb->page_table to include the frame
			pcb->page_table[k++] = i;
		}
	}

	// initialize the pcb->pc to the first entry of the first frame
	pcb->pc = mem_get_entry(pcb->page_table[0], 0);
	pcb->curr_page = 0;

	return 0;
}


// cleans up a frame in shellmemory
int mem_cleanup_frame(int frame_num) {
	struct memory_struct *mem = mem_get_entry(frame_num, 0);

	for (int i=0; i<FRAME_SIZE; i++, mem++){
		mem->var = "none";
		mem->value = "none";
	}

	// mark the frame as free
	free_list[frame_num] = 1;

	return 0;
}


// cleans up a script stored in shellmemory
int mem_cleanup_script(pcb_t *pcb) {
	char *var;
	int pid;

	// clean up script from shell memory
	for (int i = 0; i < pcb->num_pages; i++) {
		mem_cleanup_frame(pcb->page_table[i]);
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