#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <regex.h>
#include <assert.h>

#include "shellmemory.h"
#include "frame.h"
#include "pcb.h"
#include "readyqueue.h"

const int FRAME_SIZE = 3; // size of each frame in shellmemory
const int VAR_MEM_SIZE = (int) VARMEMSIZE; // part of shellmemory to store variables
const int SHELL_MEM_SIZE = FRAMEMEMSIZE + VARMEMSIZE; // total size of the shellmemory
const int FREE_LIST_SIZE = FRAMEMEMSIZE % FRAME_SIZE == 0 ? FRAMEMEMSIZE / FRAME_SIZE : (int) (FRAMEMEMSIZE / FRAME_SIZE) + 1; // amount of shellmemory entries allocated for frames and also the size of free_list

/*
* first 100 places in shell memory are reserved for variables
* the remaining memory is used for loading scripts
*/
mem_entry_t shellmemory[SHELL_MEM_SIZE];

/* 
* free_list to keep track of holes in shell memory
* index i of free_list corresponds to VAR_MEM_SIZE + (i * FRAME_SIZE) in shell memory
*/
int free_list[FREE_LIST_SIZE];

// initialize shell memory with all variables and respective values as "none"
void mem_init(){
	int i;
	// mark each spot in shellmemory as empty
	for (i=0; i<SHELL_MEM_SIZE; i++){		
		shellmemory[i].var = "none";
		shellmemory[i].value = "none";
	}
	// mark each frame as free
	for (i = 0; i < FREE_LIST_SIZE; i ++) {
		free_list[i] = 1;
	}
}


// get the shellmemory struct for a given frame and offset
mem_entry_t *mem_get_entry(int frame_number, int offset){
	assert(frame_number < FREE_LIST_SIZE && offset >= 0 && offset < FRAME_SIZE);
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

	for (i=0; i<VAR_MEM_SIZE; i++){
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

	for (i=0; i<VAR_MEM_SIZE; i++){
		if (strcmp(shellmemory[i].var, var) == 0){

			return 1;
		}
	}

	return 0;
}



// Shell memory functions for loading scripts

// loads the next required page from the script into the given page
void load_page(pcb_t *pcb, int page_num, char **page) {
	assert(pcb->curr_page < pcb->num_pages && page_num < pcb->num_pages);

	char line[1000];
	int i = 0, line_num = 1;
	int line_num_to_store_from = (page_num * FRAME_SIZE) + 1;

	// open the file
	FILE *script = fopen(pcb->script_name, "r");

	// skip to the required line number
	while(line_num != line_num_to_store_from) {
		fgets(line, 1000, script);
		line_num++;
	}

	// load the required lines into the page
	while (i < FRAME_SIZE) {
		if(fgets(line, 1000, script) != NULL) {
			page[i++] = strdup(line);
		} else {
			page[i++] = strdup("none");
		}
	}

	// close the file
	fclose(script);
}

// returns -1 if error, 0 if success
int mem_load_script_line(int pid, int line_number, char *script_line, mem_entry_t *mem) {
	int i;
	char key[10];
	if (strcmp(script_line, "none") == 0){
		strcpy(key, "none");
	} else {
		if(snprintf(key, 10, "%d-%d", pid, line_number) < 0) { 
			printf("Error: unable to load line %d of process %d\n", line_number, pid);
			return -1;
		}
	}
	mem->var = strdup(key);
	mem->value = strdup(script_line);
	free(script_line);

	return 0;
}


// Loads a frame in shellmemory; returns -1 if error, 0 if success
int mem_load_frame1(int pid, int line_number, char **script_lines, int frame_num) {
	int err = 0;
	mem_entry_t *mem = NULL;

	for (int i = 0; i < FRAME_SIZE; i++) {
		mem = mem_get_entry(frame_num, i);
		err = mem_load_script_line(pid, line_number++, script_lines[i], mem);
		if (err == -1) return err;
	}
	return err;
}


// Loads a page into a frame in shellmemory and updates the page tables accordingly
int mem_load_frame(pcb_t *pcb, char **script_lines, int page_num, rq_t *rq) {
	int flag = 0, frame_num_to_evict, err = 0;
	pcb_t *rq_head = peek_rq_head(rq);

	for (int i = 0; i < FREE_LIST_SIZE; i++) {
		// find whether a frame is available
		if (free_list[i] == 1) {
			flag = 1;
			free_list[i] = 0;
			// load the frame into that spot and update the pcb->page_table
			err = mem_load_frame1(pcb->pid, (page_num * FRAME_SIZE) + 1, script_lines, i);
			pcb->page_table[page_num] = i;
			printf("Loading page into frame %d\n", i);
			break;
		}
	}

	// if no free frame found, evict one of the frames and load the new frame in that spot
	// update the required page tables...
	if (!flag) {
		flag = 0;
		// find a frame to evict
		frame_num_to_evict = rand() % FREE_LIST_SIZE;

		// find the correspponding PCB to which the frame belongs
		while (rq_head != NULL) {
			for (int i = 0; i < rq_head->num_pages; i++) {
				if (rq_head->page_table[i] == frame_num_to_evict) {
					// update the page table of the pcb to denote that the frame has been evicted
					rq_head->page_table[i] = -1;
					flag = 1;
					break;
				}
			}
			if (flag) break;
			rq_head = rq_head->next;
		}
		
		if (!flag) {
			// finally, check whether this frame belongs to the current PCB and if so, update the PCB
			for (int i = 0; i < pcb->num_pages; i++) {
				if (pcb->page_table[i] == frame_num_to_evict) {
					pcb->page_table[i] = -1;
					break;
				}
			}
		}

		// evict the frame
		mem_cleanup_frame(frame_num_to_evict);

		printf("Loading page into frame %d [evicted]\n", frame_num_to_evict);

		// load the new page into the opened up slot and update pcb->page_table
		err = mem_load_frame1(pcb->pid, (page_num * FRAME_SIZE) + 1, script_lines, frame_num_to_evict);
		pcb->page_table[page_num] = frame_num_to_evict;

		// mark frame as unavailable
		free_list[frame_num_to_evict] = 0;
	}

	for (int x = 0; x < FRAME_SIZE; x++) {
		printf("FRAME %d - PROCESS %d PAGE %d ----> <<COMMAND %s>> %s ", pcb->page_table[page_num], pcb->pid, page_num, mem_get_entry(pcb->page_table[page_num], x)->var, mem_get_entry(pcb->page_table[page_num], x)->value);
	}
	printf("\n");

	return err;
}


// Loads a script into memory
// returns -1 if error, 0 if success
int mem_load_script(FILE *script, pcb_t *pcb, rq_t *rq) {
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
	
	// specify that each of the pages in the page table are not loaded into a frame (yet)
	for (int x = 0; x < pcb->num_pages; x++) {
		pcb->page_table[x] = -1;
	}

	// initialize pcb->pc, pcb->curr_page and pcb->exec_init
	pcb->curr_page = 0;
	pcb->exec_init = 0;
	pcb->pc = NULL;

	// get back to the beginning of script
	rewind(script); 


	// load two pages of the script and load them into the shellmemory
	for (i = 0; (i < 2 && i < pcb->num_pages) ; i++) {
		// load next page
		load_page(pcb, i, page);

		// for (j =0; j < FRAME_SIZE; j++) {
		// 	printf("PAGE %d ----> %s\n", i, page[j]);
		// }

		// store the page into the frame
		if (mem_load_frame(pcb, page, i, rq) != 0) return -1;
	}

	return 0;
}


// cleans up a frame in shellmemory
// REMEMBER: update the corresponding PCB's page table when removing a frame. It's NOT done automatically in this function
int mem_cleanup_frame(int frame_num) {
	mem_entry_t *mem = mem_get_entry(frame_num, 0);

	for (int i=0; i<FRAME_SIZE; i++, mem++){
		mem->var = "none";
		mem->value = "none";
	}

	// mark the frame as free
	free_list[frame_num] = 1;

	return 0;
}


// cleans up a script stored in shellmemory
int mem_cleanup_script(pcb_t *pcb, rq_t *rq) {
	char *var;
	int pid;

	// remove the pcb from the ready queue
	remove_rq_pcb(rq, pcb);

	// clean up script from shell memory
	for (int i = 0; i < pcb->num_pages; i++) {
		mem_cleanup_frame(pcb->page_table[i]);
	}

	// update the page table to indicate that none of the pages is loaded into memory
	for (int i = 0; i < pcb->num_pages; i++) {
		pcb->page_table[i] = -1;
	}

	// create regex to identify variables associated with a process
	regex_t re;
	regcomp(&re, "^[0-9]+-[a-zA-Z0-9]+$", 0);

	// clean up script variables from shell memory
	for (int i = 0; i < VAR_MEM_SIZE; i++) {
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