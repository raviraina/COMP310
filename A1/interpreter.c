#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h> // standard header in UNIX for directory traversal

#include "shellmemory.h"
#include "shell.h"

int MAX_ARGS_SIZE = 7;

int help();
int quit();
int badcommand();
int badcommandTooFewTokens();
int badcommandTooManyTokens();
// int set(char* var, char* value);
int set(char **, int);
int echo(char* var);
int myls();
int print(char* var);
int run(char* script);
int badcommandFileDoesNotExist();

// Interpret commands and their arguments
int interpreter(char* command_args[], int args_size){
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
		return set(command_args+1, args_size-1); // pointer to input #2 and beyond (depends on args_size)
	
	} else if (strcmp(command_args[0], "print")==0) {
		if (args_size < 2) return badcommandTooFewTokens();
		if (args_size > 2) return badcommandTooManyTokens();
		return print(command_args[1]);
	
	} else if (strcmp(command_args[0], "run")==0) {
		if (args_size < 2) return badcommandTooFewTokens();
		if (args_size > 2) return badcommandTooManyTokens();
		return run(command_args[1]);
	
	} else if (strcmp(command_args[0], "echo")==0) {
		// TODO: may need to modify this once enhanced set implemented
		if (args_size < 2) return badcommandTooFewTokens();
		if (args_size > 2) return badcommandTooManyTokens();
		return echo(command_args[1]);
	} else if (strcmp(command_args[0], "my_ls")==0) {
		if (args_size != 1) return badcommandTooManyTokens();
		return myls();
	} else return badcommand();
}

int help(){

	char help_string[] = "COMMAND			DESCRIPTION\n \
help			Displays all the commands\n \
quit			Exits / terminates the shell with ???Bye!???\n \
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

// For run command only
int badcommandFileDoesNotExist(){
	printf("%s\n", "Bad Command: File not found");
	return 3;
}

// param args: array of strings (args[0] := variable name; args[1: args_size] := tokens)
// param ars_size: length of args
int set(char* args[], int args_size){
	char *var = args[0];
	char buffer[1000];
	
	strcpy(buffer, args[1]); // copy first token to the buffer
	
	for(int i = 2; i < args_size; i++){ //copy remaining tokens to the buffer, if any
		strcat(buffer, " ");
		strcat(buffer, args[i]);
	}

	mem_set_value(var, buffer);

	return 0;
}

int echo(char* var) {

	if (var[0] == '$') {	// check if input is from memory
		char *varFromMem = var + 1;

		if (check_mem_value_exists(varFromMem)) {	// check for existance
			print(varFromMem);

		} else {
			printf("%s\n","");
		}

	} else {
		printf("%s\n", var);	// normal echo if not referencing memory
	}
	return 0;
}

int print(char* var){
	printf("%s\n", mem_get_value(var)); 
	return 0;
}

int run(char* script){
	int errCode = 0;
	char line[1000];
	FILE *p = fopen(script,"rt");  // the program is in a file

	if(p == NULL){
		return badcommandFileDoesNotExist();
	}

	fgets(line,999,p);
	while(1){
		errCode = parseInput(line);	// which calls interpreter()
		memset(line, 0, sizeof(line));

		if(feof(p)){
			break;
		}
		fgets(line,999,p);
	}

    fclose(p);

	return errCode;
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