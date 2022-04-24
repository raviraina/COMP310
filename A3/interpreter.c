#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h> // standard header in UNIX for directory traversal
#include <errno.h>
#include <libgen.h>

#include "shellmemory.h"
#include "shell.h"
#include "pcb.h"
#include "readyqueue.h"
#include "scheduler.h"

int MAX_ARGS_SIZE = 7;
int CURR_PID = 0;

int help();
int quit();
int badcommand();
int badcommandTooFewTokens();
int badcommandTooManyTokens();
int badCommandDuplicateArguments();
int badcommandFileDoesNotExist(char *);
int badCommandUnableToLoadScript(char *);
int set(char **, int, pcb_t* pcb);
int echo(char*, pcb_t *);
int myls();
int print(char*, pcb_t *);
int run(char*, rq_t *);
int exec(char **, int, char*, rq_t *);
int resetmem();


// Interpret commands and their arguments
int interpreter(char* command_args[], int args_size, pcb_t *pcb, rq_t *rq){
	int i;

	for ( i=0; i<args_size; i++){ //strip spaces new line etc
		command_args[i][strcspn(command_args[i], "\r\n")] = 0;
	}

	if (strcmp(command_args[0], "") == 0) return 0;

	if (strcmp(command_args[0], "help")==0){
	    //help
	    if (args_size != 1) return badcommandTooManyTokens();
	    return help();
	
	} else if (strcmp(command_args[0], "quit")==0) {
		//quit
		if (args_size != 1) return badcommandTooManyTokens();
		return quit();

	} else if (strcmp(command_args[0], "set")==0) {
		//set
		if (args_size < 3) return badcommandTooFewTokens();
		if (args_size > 7) return badcommandTooManyTokens();	
		return set(command_args+1, args_size-1, pcb); // pointer to input #2 and beyond (depends on args_size)
	
	} else if (strcmp(command_args[0], "print")==0) {
		if (args_size < 2) return badcommandTooFewTokens();
		if (args_size > 2) return badcommandTooManyTokens();
		return print(command_args[1], pcb);
	
	} else if (strcmp(command_args[0], "run")==0) {
		if (args_size < 2) return badcommandTooFewTokens();
		if (args_size > 2) return badcommandTooManyTokens();
		return run(command_args[1], rq);
	
	} else if (strcmp(command_args[0], "echo")==0) {
		if (args_size < 2) return badcommandTooFewTokens();
		if (args_size > 2) return badcommandTooManyTokens();
		return echo(command_args[1], pcb);
		
	} else if (strcmp(command_args[0], "my_ls")==0) {
		if (args_size != 1) return badcommandTooManyTokens();
		return myls();
		
	} else if (strcmp(command_args[0], "exec")==0) {
		if (args_size < 3) return badcommandTooFewTokens();
		if (args_size > 5) return badcommandTooManyTokens();
		return exec(command_args+1, args_size-2, command_args[args_size - 1], rq);
		
	} else if (strcmp(command_args[0], "resetmem")==0) {
		if (args_size != 1) return badcommandTooManyTokens();
		return resetmem();
	} else return badcommand();
}

int help(){

	char help_string[] = "COMMAND			DESCRIPTION\n \
help			Displays all the commands\n \
quit			Exits / terminates the shell with “Bye!”\n \
set VAR STRING		Assigns a value to shell memory\n \
print VAR		Displays the STRING assigned to VAR\n \
run SCRIPT.TXT		Executes the file SCRIPT.TXT\n ";
	printf("%s\n", help_string);
	return 0;
}

int quit(){
	const char *name = "backingstore";
	char cmd[40];
	DIR* dir = opendir(name);
	int result = 0;
	if (dir) {
		sprintf(cmd, "rm -r %s", name);
		result = system(cmd);
		
		if (result == 0) {
			printf("Backing store removed\n");
		}
	} else if (ENOENT == errno) {
		printf("Couldn't find backing store directory to delete\n");
	}
	printf("%s\n", "Bye!");
	exit(0);
}

int badcommand(){
	printf("%s\n", "Unknown Command");
	return 1;
}

int badcommandTooFewTokens(){
	printf("%s\n", "Bad Command: Too few tokens");
	return 2;
}

int badcommandTooManyTokens(){
	printf("%s\n", "Bad Command: Too many tokens");
	return 2;
}

int badCommandDuplicateArguments() {
	printf("%s\n", "Bad Command: Duplicate arguments encountered");
	return 2;
}


int badcommandFileDoesNotExist(char *file){
	printf("Bad command: file %s not found\n", file);
	return 3;
}

int badCommandUnableToLoadScript(char *file) {
	printf("Error: unexpected error occurred while loading script %s into shell memory\n", file);
	return 3;
}


// param args: array of strings (args[0] := variable name; args[1: args_size] := tokens)
// param ars_size: length of args
int set(char* args[], int args_size, pcb_t *pcb){
	char *var = args[0];
	char buffer[1000];
	
	strcpy(buffer, args[1]); // copy first token to the buffer
	
	for(int i = 2; i < args_size; i++){ //copy remaining tokens to the buffer, if any
		strcat(buffer, " ");
		strcat(buffer, args[i]);
	}

	mem_set_value(var, buffer, pcb);

	return 0;
}

int echo(char* var, pcb_t *pcb) {

	if (var[0] == '$') {	// check if input is from memory
		char *varFromMem = var + 1;

		if (check_mem_value_exists(varFromMem, pcb)) {	// check for existence
			print(varFromMem, pcb);

		} else {
			printf("%s\n","");
		}

	} else {
		printf("%s\n", var);	// normal echo if not referencing memory
	}
	return 0;
}

int print(char* var, pcb_t *pcb){
	printf("%s\n", mem_get_value(var, pcb)); 
	return 0;
}

int run(char* script, rq_t *rq){
	pcb_t *pcb = malloc(sizeof(pcb_t));

	// Preprocess: move to backing store
	char copy_command[50];
	sprintf(copy_command, "cp %s backingstore", script);
	if(system(copy_command) != 0) {
		return badCommandUnableToLoadScript(script);
	}

	// retreive filename from path
	char *filename = basename(script);
	
	// set command arg to backingstore path
	char backing_script[50];
	sprintf(backing_script, "backingstore/%s", filename);

	if (pcb == NULL) {
		return badCommandUnableToLoadScript(script);
	}

	FILE *fp = fopen(backing_script, "rt"); // the program is in a file

	if (fp == NULL) {
		return badcommandFileDoesNotExist(backing_script);
	}

	// initiate a pcb for the script
	pcb->pid = CURR_PID++;
	pcb->next = NULL;
	pcb->script_name = strdup(backing_script);

	// add pcb to the ready queue
	add_rq_tail(rq, pcb);

	// load script into memory
	if (mem_load_script(fp, pcb) != 0) {
		return badCommandUnableToLoadScript(backing_script);
	}

	// close the script
	fclose(fp);

	// let the scheduler execute the script(s) in the ready queue
	return FCFS_scheduler(rq);
}

int myls() {
	DIR *dir; // directory pointer
	struct dirent *ent; // directory entry pointer
	int total_dirs = 100;
	char **dirs = (char **) calloc(total_dirs, 100 * sizeof(char)); // array of file/directory names inside the current directory
	char tmp[100];
	int n = 0; // number of files/directories in the current directory

	if ((dir = opendir("./")) != NULL) { // open the current directory
		while ((ent = readdir (dir)) != NULL) {
			if(n >= total_dirs) {
				total_dirs += 100;
				dirs = (char **) realloc(dirs, total_dirs * 100 * sizeof(char));
			}
			dirs[n++] = strdup(ent->d_name);
		}
		closedir(dir);

		// sort the dir names using bubble sort (desc)
		for(int i=0; i<n; i++){
			for(int j=0; j<n-1-i; j++){
				if(strcasecmp(dirs[j], dirs[j+1]) < 0){ //< was originally >
					//swap array[j] and array[j+1]
					strcpy(tmp, dirs[j]);
					strcpy(dirs[j], dirs[j+1]);
					strcpy(dirs[j+1], tmp);
				}
			}
		}

		// print the sorted dir names
		while(n-->0){
			printf("%s\n", dirs[n]);
		}
		return 0;
	} else {
		// could not open directory
		perror("Error: an unexpected error occurred while reading directory contents\n");
		return 3;
	}
}

int exec(char* args[], int args_size, char* policy, rq_t *rq) {

	// case if only 1 prog (FCFS through run)
	if (args_size == 1) {
		return run(args[0], rq);
	}

	// load all PCBs into the ready queue
	for (int i = 0; i < args_size; i++) {

		// preprocess: move to backing store
		char copy_command[50];
		sprintf(copy_command, "cp %s backingstore", args[i]);
		if(system(copy_command) != 0) {
			return badCommandUnableToLoadScript(args[i]);
		}

		// retreive filename from path
		char *filename = basename(args[i]);
		
		// set command arg to backingstore path
		char backing_script[50];
		sprintf(backing_script, "backingstore/%s", filename);

		FILE *fp = fopen(backing_script, "rt");
		
		if (fp == NULL) {
			return badcommandFileDoesNotExist(args[i]);
		}

		pcb_t *pcb = malloc(sizeof(pcb_t));

		if (pcb == NULL) {
			return badCommandUnableToLoadScript(args[i]);
		}

		pcb->pid = CURR_PID++;
		pcb->next = NULL;
		pcb->script_name = strdup(backing_script);

		add_rq_tail(rq, pcb);
		
		if (mem_load_script(fp, pcb) != 0) {
			return badCommandUnableToLoadScript(backing_script);
		}

		fclose(fp);
	}

	if (strcmp(policy, "SJF") == 0) {
		return SJF_scheduler(rq);
	} else if (strcmp(policy, "RR") == 0) {
			return RR_scheduler(rq);
	} else if (strcmp(policy, "AGING") == 0) {
		return AGING_scheduler(rq);
	} else if (strcmp(policy, "FCFS") == 0) {
		return FCFS_scheduler(rq);
	} else {
		printf("%s", "Invalid policy type. Choose from 'FCFS', 'SJF', 'RR', or 'AGING'");
	}

	return 0;
}

int resetmem() {
	// reset memory as it was initially
	mem_init();

	return 0;
}
