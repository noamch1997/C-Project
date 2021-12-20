/*
	set of functions for instruction cmds analizing.
*/

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "utils.h"


/* Returns an instruction by specified index. if no such one, returns NONE */
instruction find_instruction_from_index(line_info line, int *index) {
	char temp[MAX_LINE_LENGTH];
	int j;
	instruction result;

	MOVE_TO_NOT_WHITE(line.content, *index) /* move index to first not white char */
	if (line.content[*index] != '.') return NONE_INST;
	for (j = 0; line.content[*index] && line.content[*index] != '\n' && line.content[*index] != '\t' && line.content[*index] != ' '; (*index)++, j++) {
		temp[j] = line.content[*index];
	}
	temp[j] = '\0'; /* End of string */
	
	/* if invalid instruction but starts with ., return error */
	if ((result = find_instruction_by_name(temp+1)) != NONE_INST) return result;
	printf_line_error(line, "Invalid instruction name: <%s>", temp);
	return ERROR_INST; /* starting with '.' but not a valid instruction */
}

/* process sting instruction */
bool process_string_instruction(line_info line, int index, data_word *data_img,long *dataIndex, long *dc) {
	int i,j;
	char temp_str[MAX_LINE_LENGTH];

	/*MOVE_TO_NOT_WHITE(line.content, index)*/
	if (line.content[index] != '"') {
		printf_line_error(line, "Missing opening quote of string");
		return FALSE;
	}
	
	/* check if the chars are printables & Copy the string including quotes & everything until end of line */
	for (i = 0;line.content[index] && line.content[index] != '\n' &&
	       line.content[index] != EOF; index++,i++) {
	       
	       if(!isprint(line.content[index]) && line.content[index] != '	' ){

	       		printf_line_error(line, "The i=%d char is'nt a valid ASCII char.",i);
	       		return FALSE;
	       }
			temp_str[i] = line.content[index];
	}
	/* Put string terminator instead of last quote: */
	temp_str[i] = '\0';
	
	if (i == 1) { /* if there is no data after opening quote print error */
		printf_line_error(line, "Missing data after opening quote of string.");
		return FALSE;
	}
	
	
	/* now lets get the last char and check if its a qoute or illegal char */
	j=i-1; /* last char index */
	while(temp_str[j] == ' ' || temp_str[j] == '\t') j--;
	if (temp_str[j] != '"') { /* if the last char isnt a closing quote char=" */
		printf_line_error(line, "Missing closing quote of string. the last char is: <%c>",temp_str[j]);

		return FALSE;
	}	

	for(i = 1;temp_str[i] && i<j; i++) {
		data_img[*dataIndex].value = temp_str[i];
		data_img[*dataIndex].dc = (*dc);
		data_img[*dataIndex].type = STRING_INST;
		(*dc)++;
		(*dataIndex)++;
	}
	/* Put string terminator */
	data_img[*dataIndex].value = '\0';
	data_img[*dataIndex].dc = (*dc);
	data_img[*dataIndex].type = STRING_INST;
	(*dc)++;
	(*dataIndex)++;
	
	return TRUE;
}

/*
 * Parses a .data instruction. copies each number value to data_img by dc position, and returns the amount of processed data.
 */
bool process_data_instruction(line_info line, int index, data_word *data_img,long *dataIndex, long *dc, instruction dataType) {
	char temp[MAX_LINE_LENGTH], *temp_ptr;
	long value;
	int i;
	char *data_str;
	dataType == DB_INST ? (data_str=".db"): dataType == DH_INST ? (data_str=".dh"): (data_str=".dw");
	
	MOVE_TO_NOT_WHITE(line.content, index)
	if (line.content[index] == ',') {
		printf_line_error(line, "Unexpected comma after <%s> instruction",data_str);
	}
	do {
		for (i = 0;
		     line.content[index] && line.content[index] != EOF && line.content[index] != '\t' &&
		     line.content[index] != ' ' && line.content[index] != ',' &&
		     line.content[index] != '\n'; index++, i++) {
			temp[i] = line.content[index];
		}
		temp[i] = '\0'; /* End of string */
		if (!is_int(temp)) {
			printf_line_error(line, "Expected integer for <%s> instruction. got: <%s>", data_str, temp);
			return FALSE;
		}
		
		/* Now let's write to data buffer */
		value = strtol(temp, &temp_ptr, 10);

		switch(dataType)
		{
			case DB_INST:
				if(!(value >= -128 && value <= 127)){
					printf_line_error(line, "operator exceeds byte range <-128 - 127>  got: <%s>", temp);
					return FALSE;
				}
			break;
			case DH_INST:
				
				if(!(value >= -32768 && value <= 32767)){
					printf_line_error(line, "operator exceeds halfword range <-32768 - 32767>. got: <%s>", temp);
					return FALSE;
				}
			break;
			case DW_INST:
				if(value == (MAX_INT32bit) && !is_max(temp)){
					printf_line_error(line, "operator exceeds word range <-2147483648 - 2147483647>.  got: <%s>", temp);
					return FALSE;
				}
				
				if(value == (MIN_INT32bit) && !is_min(temp)){
					printf_line_error(line, "operator exceeds word range <-2,147,483,648 - 2,147,483,647>  got: <%s>", temp);
					return FALSE;
				}
				
			break;
			default:
			break;
			
		}
		
		
		data_img[*dataIndex].type = dataType;
		data_img[*dataIndex].value = value;
		data_img[*dataIndex].dc = (*dc);

		(*dataIndex)++;
		(*dc)+=dataType; /* a word was written right now */
		
		MOVE_TO_NOT_WHITE(line.content, index)
		if (line.content[index] == ',') index++;
		else if (!line.content[index] || line.content[index] == '\n' || line.content[index] == EOF)
			break; /* End of line/file/string => nothing to process anymore */
		/* Got comma. Skip white chars and check if end of line (if so, there's extraneous comma!) */
		MOVE_TO_NOT_WHITE(line.content, index)
		if (line.content[index] == ',') {
			printf_line_error(line, "Multiple consecutive commas.");
			return FALSE;
		} else if (line.content[index] == EOF || line.content[index] == '\n' || !line.content[index]) {
			printf_line_error(line, "Missing data after comma");
			return FALSE;
		}
	} while (line.content[index] != '\n' && line.content[index] != EOF);
	return TRUE;
}
