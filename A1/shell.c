#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "interpreter.h"
#include "shellmemory.h"

const int MAX_USER_INPUT = 1000;
const int MAX_NUM_COMMANDS = 10;

int parseInput(char ui[]);

// Start of everything
int main(int argc, char *argv[])
{

	printf("%s\n", "Shell version 1.1 Created January 2022");
	help();

	char prompt = '$';				// Shell prompt
	char userInput[MAX_USER_INPUT]; // user's input stored here
	int errorCode = 0;				// zero means no error, default

	// init user input
	for (int i = 0; i < MAX_USER_INPUT; i++)
		userInput[i] = '\0';

	// init shell memory
	mem_init();

	while (1)
	{
		if (feof(stdin))
		{
			freopen("/dev/tty", "r", stdin);
		}
		printf("%c ", prompt);
		fgets(userInput, MAX_USER_INPUT - 1, stdin);

		errorCode = parseInput(userInput);
		if (errorCode == -1)
			exit(99); // ignore all other errors
		memset(userInput, 0, sizeof(userInput));
	}

	return 0;
}

// Extract words from the input then call interpreter
int parseInput(char ui[])
{

	char tmp[200];
	char *words[100];
	int a, b;
	int w = 0; // wordID -- number of words in the user input (in each command)
	int c = 0; // commandID -- number of commands in the user input
	int return_val = 0;
	const char delim[2] = ";";
	char *token;
	char *ptr = ui;

	token = strtok(ui, delim);

	while (token != NULL)
	{
		if(strcmp(token, "\0") == 0) continue;
        printf("token : %s\n", token);
		for (a = 0, w = 0; token[a] == ' ' && a < MAX_USER_INPUT; a++); // skip leading white spaces
		while (token[a] != '\0' && a < MAX_USER_INPUT)
		{

			for (b = 0; token[a] != '\0' && token[a] != ' ' && a < MAX_USER_INPUT; a++, b++)
				tmp[b] = token[a]; // extract a word

			tmp[b] = '\0';

			words[w] = strdup(tmp);

			a++;
			w++;
		}

		// for(int i = 0; i < w; i++) {
		// 	printf("%s\n", words[i]);
		// }

		return_val = interpreter(words, w);
		if (return_val)
			return return_val;

		token = strtok(NULL, delim);
		// ptr = NULL;
	}
	return return_val;

	// for(a=0; ui[a]==' ' && a<MAX_USER_INPUT; a++);		// skip leading white spaces

	// while(ui[a] != '\0' && a<MAX_USER_INPUT) {

	// 	for(b=0; ui[a]!='\0' && ui[a]!=' ' && a<MAX_USER_INPUT; a++, b++)
	// 		tmp[b] = ui[a];						// extract a word

	// 	tmp[b] = '\0';

	// 	words[w] = strdup(tmp);

	// 	a++;
	// 	w++;
	// }

	// while(ui[a] != '\0' && a<MAX_USER_INPUT) {

	// 	for(b=0; ui[a]!='\0' && ui[a]!=' ' && ui[a] != ';' && a<MAX_USER_INPUT; a++, b++)
	// 		tmp[b] = ui[a];						// extract a word

	// 	tmp[b] = '\0';

	// 	words[w] = strdup(tmp);

	// 	w++;

	// 	if(ui[a] == ';') {

	// 		printf("w is : %d\n", w);
	// 		for (int i = w; i < 100; i++) words[i] = strdup("\0");

	// 		for(int i=0; i < w; i++) printf("%s -- ", words[i]);
	// 		return_val = interpreter(words, w);
	// 		if(return_val) return return_val;
	// 		w = 0;
	// 	}

	// 	a++;
	// }
	// return return_val;

	// return interpreter(words, w);
}
