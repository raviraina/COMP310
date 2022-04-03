#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include <sys/stat.h>
#include <errno.h>
#include "interpreter.h"
#include "shellmemory.h"
#include "pcb.h"
#include "readyqueue.h"

// max vars
const int MAX_USER_INPUT = 1000;
const int MAX_NUM_COMMANDS = 10;

int parseInput(char ui[], pcb_t *pcb, rq_t *rq);

// Start of everything
int main(int argc, char *argv[])
{

	printf("%s\n", "Shell version 1.2 Created March 2022");
	printf("Frame Store Size = %d; Variable Store Size = %d\n",FRAME_SIZE, VAR_MEM_SIZE);
	help();

	// TODO: Remove verbose error checking
	// create backing store
	int backingUsable = 0;
	const char *name = "backingstore";
	while(backingUsable == 0) {
		errno = 0;
		char cmd[40];
		int result = 0;
		int ret = mkdir(name, S_IRWXU);
		if (ret == -1) {
			switch (errno) {
				case EACCES :
					printf("ERROR: Current directory does not allow write access.\n");
					exit(1);
				
				case EEXIST:
					printf("pathname already exists\n");
					sprintf(cmd, "rm -r %s", name);
					result = system(cmd);
					
					if (result == 0) {
        				printf("It is deleted\n");
					}
					continue;
				
				case ENAMETOOLONG:
					printf("ERROR: Specified folder name is too long.\n");
					exit(1);
				
				default:
					perror("mkdir");
					exit(EXIT_FAILURE);
			}
		} else {
			printf("back store created\n");
			backingUsable = 1;
		}
	}

	char prompt = '$';				// Shell prompt
	char userInput[MAX_USER_INPUT]; // user's input stored here
	int errorCode = 0;				// zero means no error, default

	// init user input
	for (int i = 0; i < MAX_USER_INPUT; i++)
		userInput[i] = '\0';

	// init shell memory
	mem_init();

	//init ready queue
	rq_t *rq = init_rq();

	while (1)
	{
		if (feof(stdin))	// check if we're at the end of the input file
		{
			freopen("/dev/tty", "r", stdin);	// release control back to user
		}
		printf("%c ", prompt);
		fgets(userInput, MAX_USER_INPUT - 1, stdin);
		errorCode = parseInput(userInput, NULL, rq); // direct commands from shell are not executed as a process
		if (errorCode == -1)
			exit(99); // ignore all other errors
		memset(userInput, 0, sizeof(userInput));
	}

	return 0;
}

// Extract words from the input then call interpreter
int parseInput(char ui[], pcb_t *pcb, rq_t *rq)
{
	char *words[100]; // holds parsed commands and arguments until they are sent to the interpreter
	int w = 0; // wordID -- number of words in the user input (in each command)

	// tokens and checkpoints for strtok_r
	char* cmd_token;
	char* cmd_token_chkpt = ui;
	char* word_token;
	char* word_token_chkpt;

	// delimeters for parsing multiple commands
	const char* delim = strdup(";");
	const char* word_sep = strdup(" ");

	int return_val = 0; // return value of the command(s)

	// parse user input (ui) into separate commands separated by semi-colons (;)
	while ((cmd_token = strtok_r(cmd_token_chkpt, delim, &cmd_token_chkpt)) != NULL)
	{
		if(strcmp(cmd_token, "\0") == 0 || strcmp(cmd_token, " ") == 0 || strcmp(cmd_token, "") == 0) { 
			strtok(NULL, delim);
			continue;
		}

		// parse command name and arguments
		word_token_chkpt = cmd_token;
		while ((word_token = strtok_r(word_token_chkpt, word_sep, &word_token_chkpt)) != NULL)
		{
			words[w++] = strdup(word_token);
		}

		// call the interpreter
		return_val = interpreter(words, w, pcb, rq);
		
		// reset wordID for the following command
		w = 0;
	}
	return return_val;
}
