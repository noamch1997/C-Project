#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "utils.h"
#include "code.h"

#define ERR_OUTPUT_FILE stderr


/*
	add value to dc value for each data img index
*/
void add_value_to_data(data_word *data_img, long icf, long dcf, long dataIndex) {
	int i=0;
	while(i<dataIndex){
		data_img[i].dc += icf;
		i++;
	}
}



/* Returns whether an error occurred during the try of parsing the symbol. puts the symbol into the second buffer. */
bool find_label(line_info line, char *symbol_dest) {
	int j, i;
	i = j = 0;

	/* Skip white chars at the beginning anyway */
	MOVE_TO_NOT_WHITE(line.content, i)

	/* Let's allocate some memory to the string needed to be returned */
	for (; line.content[i] && line.content[i] != ':' && line.content[i] != EOF && i <= MAX_LINE_LENGTH; i++, j++) {
		symbol_dest[j] = line.content[i];
	}
	symbol_dest[j] = '\0'; /* End of string */
	

	/* if it was a try to define label, print errors if needed. */
	if (line.content[i] == ':') {
		if (!is_valid_label_name(symbol_dest)) {
			printf_line_error(line,
			                  "Invalid label name - cannot be longer than 31 chars, may only start with letter be alphanumeric.");
			symbol_dest[0] = '\0';
			return TRUE; /* No valid symbol, and no try to define one */
		}
		return FALSE;
	}
	symbol_dest[0] = '\0';
	return FALSE; /* There was no error */
}


char *strallocat(char *s0, char* s1) {
	char *str = (char *)malloc_with_check(strlen(s0) + strlen(s1) + 1);
	strcpy(str, s0);
	strcat(str, s1);
	return str;
}

/*
 * check the string length and returns true if length < MAX_LINE_LENGTH
 * otherwise false
*/
bool is_valid_length(char *str){
	int i, cnt;
	i = cnt = 0;
	while(str[i] && str[i] != EOF && str[i] != '\n' && cnt <= MAX_LINE_LENGTH){
		if(str[i] != ' ' && str[i] != '\t')
			cnt++;
		i++;
	}
	if(cnt <= MAX_LINE_LENGTH)
		return TRUE;
	return FALSE;
}

struct instruction_lookup_item {
	char *name;
	instruction value;
};

static struct instruction_lookup_item
		instructions_lookup_table[] = {
		{"asciz", STRING_INST},
		{"db",   DB_INST},
		{"dh",   DH_INST},
		{"dw",   DW_INST},
		{"entry",  ENTRY_INST},
		{"extern", EXTERN_INST},
		{NULL, NONE_INST}
};

instruction find_instruction_by_name(char *name) {
	struct instruction_lookup_item *curr_item;
	for (curr_item = instructions_lookup_table; curr_item->name != NULL; curr_item++) {
		if (strcmp(curr_item->name, name) == 0) {
			return curr_item->value;
		}
	}

	return NONE_INST;
}



char* get_addressing_str(addressing_type address){
	switch(address)
	{
		case IMMEDIATE_ADDR:
			return "IMMEDIATE_ADDR";
			break;
		case DIRECT_ADDR:
			return "DIRECT_ADDR";
		break;
		case REGISTER_ADDR:
			return "REGISTER_ADDR";
			break;
		default:
			return "NONE_ADDR";
			break;
	}
	
}

/* is equals max int */
bool is_max(char *num){
	int i=0,j=0;
	char *max_int = "2147483647";
	if(num[0] == '+')
		i++;
	while(num[i] != '\0'){
		if(num[i] != max_int[j])
			return FALSE;
		i++;
		j++;
	}
	return TRUE;
}

/* is equals min int */
bool is_min(char *num){
	int i=0,j=0;
	char *max_int = "-2147483648";
	while(num[i] != '\0'){
		if(num[i] != max_int[j])
			return FALSE;
		i++;
		j++;
	}
	return TRUE;
}

bool is_int(char *string) {
	int i = 0;
	if (string[0] == '-' || string[0] == '+') string++; /* if string starts with +/-, it's OK */
	for (; string[i]; i++) { /* check that all chars are digits */
		if (!isdigit(string[i])) {
			return FALSE;
		}
	}
	return i > 0; /* if i==0 then it was an empty string! */
}

void *malloc_with_check(long size) {
	void *ptr = malloc(size);
	if (ptr == NULL) {
		printf("Error: Memory allocation failed. EXIT.....");
		exit(1);
	}
	return ptr;
}

cmd_type get_type(opcode op){
	if(op == 0 || op == 1)
		return R_CMD;
	if(op >= 10 && op <= 24)
		return I_CMD;
	if((op >= 30 && op <= 32) || op <= 63)
		return J_CMD;
	return NONE_CMD;
	
}

bool is_valid_label_name(char *name) {
	/* Check length, first char is alpha and all the others are alphanumeric, and not saved word */
	return name[0] && strlen(name) <=31 && isalpha(name[0]) && is_alphanumeric_str(name + 1) &&
	       !is_reserved_word(name);
}

bool is_alphanumeric_str(char *string) {
	int i;
	/*check for every char in string if it is non alphanumeric char if it is function returns true*/
	for (i = 0; string[i]; i++) {
		if (!isalpha(string[i]) && !isdigit(string[i])) return FALSE;
	}
	return TRUE;
}

bool is_reserved_word(char *name) {
	int _funct, _opc;
	/* check if register or command */
	get_opcode_func(name, &_opc, (funct *) &_funct);
	if (_opc != NONE_OP || get_register_by_name(name) != NONE_REG || find_instruction_by_name(name) != NONE_INST) return TRUE;
	
	return FALSE;
}

/* Prints the warnings into a file, defined above as macro */

int printf_line_warning(line_info line, char *message, ...) { 
	int result;
	va_list args; /* for formatting */
	/* Print file+line */
	fprintf(ERR_OUTPUT_FILE,ANSI_COLOR_GRAY"[Warning] " ANSI_COLOR_RESET "In %s:%ld: ", line.file_name, line.line_number);

	/* use vprintf to call printf from variable argument function (from stdio.h) with message + format */
	va_start(args, message);
	result = vfprintf(ERR_OUTPUT_FILE, message, args);
	va_end(args);

	fprintf(ERR_OUTPUT_FILE, "\n");
	return result;
}


/* Prints the errors into a file, defined above as macro */

int printf_line_error(line_info line, char *message, ...) { 
	int result;
	va_list args; /* for formatting */
	/* Print file+line */
	fprintf(ERR_OUTPUT_FILE,ANSI_COLOR_RED "[Error] " ANSI_COLOR_RESET "In %s:%ld: ", line.file_name, line.line_number);

	/* use vprintf to call printf from variable argument function (from stdio.h) with message + format */
	va_start(args, message);
	result = vfprintf(ERR_OUTPUT_FILE, message, args);
	va_end(args);

	fprintf(ERR_OUTPUT_FILE, "\n");
	return result;
}

void free_code_image(code_word **code_image, long icf) {
	long i=0;
	while(code_image[i] != NULL){
		free(code_image[i]);
		i++;
	}
}
