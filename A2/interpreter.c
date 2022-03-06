#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h> // standard header in UNIX for directory traversal

#include "shellmemory.h"
#include "shell.h"
#include "pcb.h"
#include "readyqueue.h"
#include "scheduler.h"

int MAX_ARGS_SIZE = 7;
int CURR_PID = 0;

int help();
int quit();
int badcommand(char* command_args[], int args_size);
int badcommandTooFewTokens();
int badcommandTooManyTokens();
// int set(char* var, char* value);
int set(char **, int, pcb_t* pcb);
int echo(char* var, pcb_t *pcb);
int myls();
int print(char* var, pcb_t *pcb);
int run(char* script, rq_t *rq);
int exec(char **, int args_size, char* policy, rq_t *rq);
int badcommandFileDoesNotExist();

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
		// TODO: may need to modify this once enhanced set implemented
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
	} else return badcommand(command_args, args_size);
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
	printf("%s\n", "Bye!");
	exit(0);
}

int badcommand(char* command_args[], int args_size){
	printf("%d", args_size);
	for (int i = 0; i < args_size; i++) {
		printf("%s", command_args[i]);
	}
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

// For run command only
int badcommandFileDoesNotExist(){
	printf("%s\n", "Bad Command: File not found");
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
	// int errCode = 0;
	// char line[1000];
	// FILE *p = fopen(script,"rt");  // the program is in a file

	// if(p == NULL){
	// 	return badcommandFileDoesNotExist();
	// }

	// fgets(line,999,p);
	// while(1){
	// 	errCode = parseInput(line, NULL);	// which calls interpreter()
	// 	memset(line, 0, sizeof(line));

	// 	if(feof(p)){
	// 		break;
	// 	}
	// 	fgets(line,999,p);
	// }

    // fclose(p);

	// return errCode;
	pcb_t *pcb = malloc(sizeof(pcb_t));

	if (pcb == NULL) {
		printf("%s\n", "Error: malloc failed");
		return -1;
	}

	FILE *fp = fopen(script, "rt"); // the program is in a file

	if (fp == NULL) {
		return badcommandFileDoesNotExist();
	}

	// initiate a pcb for the script
	pcb->pid = CURR_PID++;
	pcb->next = NULL;

	// add pcb to the ready queue
	add_rq_tail(rq, pcb);

	// load script into memory
	mem_load_script(fp, pcb);

	// close the script
	fclose(fp);

	// let the scheduler execute the script(s) in the ready queue
	return RR_scheduler(rq);
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
	/* could not open directory */
		perror("Could not open directory");
		return 3;
	}
}

int exec(char* args[], int args_size, char* policy, rq_t *rq) {
	
	// case if only 1 prog (FCFS through run)
	if (args_size == 1) {
			run(args[0], rq);
			return 0;
	}

	for (int i = 0; i < args_size; i++) {
		// printf("%s", args[i]);
		FILE *fp = fopen(args[i], "rt");
		
		if (fp == NULL) {
			return badcommandFileDoesNotExist();
		}

		pcb_t *pcb = malloc(sizeof(pcb_t));

		pcb->pid = CURR_PID++;
		pcb->next = NULL;

		add_rq_tail(rq, pcb);
		mem_load_script(fp, pcb);
		fclose(fp);
	}

	if (strcmp(policy, "SJF") == 0) {
			// SJF policy
		return SJF_scheduler(rq);
	} else if (strcmp(policy, "RR") == 0) {
			// RR policy 
			return RR_scheduler(rq);
	} else if (strcmp(policy, "AGING") == 0) {
			// AGING policy
			printf("%s", "AG");
	} else if (strcmp(policy, "FCFS") == 0) {
		return FCFS_scheduler(rq);
	} 
	else {
		printf("%s", "Invalid policy type. Choose from 'FCFS', 'SJF', 'RR', or 'AGING'");
	}

	return 0;
}