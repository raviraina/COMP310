#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "shellmemory.h"

/*
* first 100 places in shell memory are reserved for variables
* the remaining memory is used for loading scripts
*/
struct memory_struct shellmemory[1000];

// Helper functions

// // checks whether two strings are equal in content and size
// int match(char *model, char *var) {
// 	int i, len=strlen(var), matchCount=0;
// 	for(i=0;i<len;i++)
// 		if (*(model+i) == *(var+i)) matchCount++;
// 	if (matchCount == len)
// 		return 1;
// 	else
// 		return 0;
// }


// char *extract(char *model) {
// 	char token='=';    // look for this to find value
// 	char value[1000];  // stores the extract value
// 	int i,j, len=strlen(model);
// 	for(i=0;i<len && *(model+i)!=token;i++); // loop till we get there
// 	// extract the value
// 	for(i=i+1,j=0;i<len;i++,j++) value[j]=*(model+i);
// 	value[j]='\0';
// 	return strdup(value);
// }


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
void mem_set_value(char *var_in, char *value_in) {
	
	int i;

	for (i=0; i<100; i++){
		if (strcmp(shellmemory[i].var, var_in) == 0){
			shellmemory[i].value = strdup(value_in);
			return;
		} 
	}

	//Value does not exist, need to find a free spot.
	for (i=0; i<100; i++){
		if (strcmp(shellmemory[i].var, "none") == 0){
			shellmemory[i].var = strdup(var_in);
			shellmemory[i].value = strdup(value_in);
			return;
		} 
	}

	return;

}

//get value based on input key (variable)
char *mem_get_value(char *var_in) {
	int i;

	for (i=0; i<100; i++){
		if (strcmp(shellmemory[i].var, var_in) == 0){

			return strdup(shellmemory[i].value);
		} 
	}
	return "Variable does not exist";

}

// check if a variable exists in shell memory
int check_mem_value_exists(char *var_in) {
	int i;

	for (i=0; i<100; i++){
		if (strcmp(shellmemory[i].var, var_in) == 0){

			return 1;
		}
	}

	return 0;
}


// Shell memory functions for loading scripts
void mem_load_script_line(int pid, int line_number, char *script_line) {
	int i;
	char key[10];
	int er = snprintf(key, 10, "%d-%d", pid, line_number);
	for (i=100; i<1000; i++){
		if (strcmp(shellmemory[i].var, "none") == 0){
			shellmemory[i].var = strdup(key);
			shellmemory[i].value = strdup(script_line);
			return;
		} 
	}
}
