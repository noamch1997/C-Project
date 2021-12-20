/*
	the first pass main function as described in booklet. and related functions.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "globals.h"
#include "code.h"
#include "utils.h"
#include "instructions.h"
#include "first_pass.h"


/**
 * Processes a single code line in the first pass.
 * Adds the code build binary structure to the code_img,
 * encodes immediately-addresses operands and leaves required data word that use labels NULL.
 * @param line The code line to process
 * @param i the starting index to process
 * @param ic A pointer to the current instruction counter
 * @param codeIndex The current code_word index
 * @param code_img The code image array
 * @return Whether succeeded
 */
static bool process_code(line_info line, int i, long *ic, long *codeIndex, code_word **code_img);

/**
 * Processes a single line in the first pass
 * @param line The process line
 * @param ic A pointer to the current instruction counter
 * @param dc A pointer to the current data counter
 * @param codeIndex A pointer to the current code index
 * @param dataIndex A pointer to the current data index
 * @param code_img The code image array
 * @param data_img The data image array
 * @param symbol_table The symbol table
 * @return Whether succeeded.
 */
bool process_line_fpass(line_info line, long *ic, long *dc, long *codeIndex, long *dataIndex, code_word **code_img,
data_word *data_img, table *symbol_table) {
	int i, j;
	char symbol[MAX_LINE_LENGTH];
	instruction instruction;

	i = 0;
	MOVE_TO_NOT_WHITE(line.content, i) /* Move to next non-white char */
	if (!line.content[i] || line.content[i] == '\n' || line.content[i] == EOF || line.content[i] == ';')
		return TRUE; /* skip empty/comment line */

	/* Check if symbol */
	/* if tried to define label, but it's invalid, return that an error occurred. */
	if (find_label(line, symbol)) {
		return FALSE;
	}

	/* if illegal name */
	if (symbol[0] && !is_valid_label_name(symbol)) {
		printf_line_error(line, "Illegal label name: %s", symbol);
		return FALSE;
	}

	if (symbol[0] != '\0') {
		for (; line.content[i] != ':'; i++); /* if symbol detected, start analyzing from deceleration ending */
		i++;
	}

	MOVE_TO_NOT_WHITE(line.content, i) /* Move to next not-white char */

	if (line.content[i] == '\n') return TRUE; /* skip label only line */

	/* if already defined as data/external/code and not empty line */
	if (find_by_types(*symbol_table, symbol, 3, EXTERNAL_SYMBOL, DATA_SYMBOL, CODE_SYMBOL)) {
		printf_line_error(line, "Symbol %s is already defined.", symbol);
		return FALSE;
	}

	/* Check if it's an instruction (starting with '.') */
	instruction = find_instruction_from_index(line, &i);

	if (instruction == ERROR_INST) { /* Syntax error found */
		printf_line_error(line, "Unrecognized command! - < %s >", symbol);
		return FALSE;
	}
	
	MOVE_TO_NOT_WHITE(line.content, i)

	/* if it's an instruction */
	if (instruction != NONE_INST) {
		/* if .asciz or .data, and symbol defined, put it into the symbol table */
		if ((instruction == DB_INST || instruction == DH_INST || instruction == DW_INST || instruction == STRING_INST) && symbol[0] != '\0')
			/* is data or string, add DC with the symbol to the table as data */
			add_table_item(symbol_table, symbol, *dc, DATA_SYMBOL);

		/* if string, encode into data image buffer and increase dc. */
		if (instruction == STRING_INST)
			return process_string_instruction(line, i, data_img, dataIndex, dc);
			/* if .data, encode into data image buffer and increase dc. */
		else if (instruction == DB_INST || instruction == DH_INST || instruction == DW_INST){
			return process_data_instruction(line, i, data_img, dataIndex, dc, instruction);
		}	
		
			/* if .extern, add to externals symbol table */
		else if (instruction == EXTERN_INST) {
			if(symbol[0] != '\0') 
				printf_line_warning(line, "Can't define a label to an extern instruction. ignoring.");
		
			MOVE_TO_NOT_WHITE(line.content, i)
			/* if external symbol detected, start analyzing from it's deceleration end */
			for (j = 0; line.content[i] && line.content[i] != '\n' && line.content[i] != '\t' && line.content[i] != ' ' && line.content[i] != EOF; i++, j++) {
				symbol[j] = line.content[i];
			}
			symbol[j] = 0;
			/* If invalid external label name, it's an error */
			if(symbol[0] == '\0'){
				printf_line_error(line, "You have to specify a label name for .extern instruction.");
				return FALSE;
			}	
			if (!is_valid_label_name(symbol)) {
				printf_line_error(line, "Invalid external label name: <%s>", symbol);
				return FALSE;
			}
			add_table_item(symbol_table, symbol, 0, EXTERNAL_SYMBOL); /* Extern value is defaulted to 0 */
		}
			/* if entry and symbol defined, print error */
		else if (instruction == ENTRY_INST && symbol[0] != '\0') {
				printf_line_warning(line, "Can't define a label to an entry instruction. ignoring.");
		}
		/* .entry is handled in second pass! */
	}
		/* its not an instruction so its a command */
	else {
		/* if symbol defined, add it to the symbol table */
		if (symbol[0] != '\0')
			add_table_item(symbol_table, symbol, *ic, CODE_SYMBOL);
		/* Analyze code */
		return process_code(line, i, ic, codeIndex, code_img);
	}
	return TRUE;
}


/**
 * Processes a single code line in the first pass.
 * Adds the code build binary structure to the code_img,
 * encodes immediately-addresses operands and leaves required data word that use labels NULL.
 * @param line The code line to process
 * @param i Where to start processing the line from
 * @param ic A pointer to the current instruction counter
 * @param codeInedex the current code index of code_img
 * @param code_img The code image array
 * @return Whether succeeded
 */
static bool process_code(line_info line, int i, long *ic, long *codeIndex, code_word **code_img) {
	cmd_type curr_type;
	code_word *code_to_write;
	opcode curr_opcode; /* the current opcode and funct values */
	funct curr_funct;
	int j, operand_count;
	char operation[MAX_LINE_LENGTH+1]; /* stores the string of the current code instruction */
	char *operands[3]; /* 3 strings, each for operand */

	
	/* Skip white chars */
	MOVE_TO_NOT_WHITE(line.content, i)

	/* Until white char, end of line, or too big instruction, copy it: */
	for (j = 0; line.content[i] && line.content[i] != '\t' && line.content[i] != ' ' && line.content[i] != '\n' && line.content[i] != EOF && j < MAX_LINE_LENGTH; i++, j++) {
		operation[j] = line.content[i];
	}
	operation[j] = '\0'; /* End of string */
	
	/* Get opcode & funct by command name into curr_opcode/curr_funct */
	get_opcode_func(operation, &curr_opcode, &curr_funct);
	
	/* If invalid operation (opcode is NONE_OP=-1), print ERROR and skip the line. */
	if (curr_opcode == NONE_OP) {
		printf_line_error(line, "Unrecognized instruction: <%s>.", operation);
		return FALSE;
	}

	/* Separate operands+get their count+check operands*/
	if (!analyze_operands(line, i, operands, &operand_count, operation))  {
		return FALSE;
	}
	
	/* validate operands by opcode */
	if (!validate_operand_by_opcode(line, curr_opcode, operand_count, operands)) {
		/* Release allocated memory for operands */
		j=0;
		while(j < operand_count){
			free(operands[j]);
			j++;
		}
		return FALSE;
	}	
	
	curr_type = get_type(curr_opcode);
	
	/* Build the code word and store in the code image array */	
	code_to_write = (code_word *) malloc_with_check(sizeof(code_word));
	(code_to_write->opcode) = curr_opcode;
	(code_to_write->ic) = (*ic);
	(code_to_write->type) = curr_type;	
	
	if(curr_type == R_CMD){
		r_word codeword = get_r_code(curr_opcode, curr_funct, operand_count, operands);
		(code_to_write->typeword).rword = codeword;
	}
	else if(curr_type == I_CMD){
		i_word codeword = get_i_code(curr_opcode, operand_count, operands);
		(code_to_write->typeword).iword = codeword;
	}
	else if(curr_type == J_CMD){
		j_word codeword = get_j_code(curr_opcode, operand_count, operands);
		(code_to_write->typeword).jword = codeword;
	}
	
	code_img[(*codeIndex)] = code_to_write;
	(*codeIndex)++;
	(*ic)+=4; /* increase ic to point the next address */

	return TRUE; /* No errors */
}
