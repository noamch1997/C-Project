/*
	the second pass main function as described in booklet.  and related functions.
*/


#include <stdio.h>
#include <stdlib.h>
#include "second_pass.h"
#include "code.h"
#include "utils.h"
#include "string.h"


int process_spass_operand(line_info line, char *operand, opcode curr_opcode, long *codeIndex, code_word **code_img, table *symbol_table);

/**
 * Processes a single line in the second pass
 * @param line The line string
 * @param codeIndex A pointer to current code index of code img
 * @param code_img Code image
 * @param symbol_table The symbol table
 * @return Whether operation succeeded
 */
bool process_line_spass(line_info line, long *codeIndex, code_word **code_img, table *symbol_table) {
	opcode curr_opcode;
	char *indexOfColon;
	char *token;
	long i = 0;
	
	MOVE_TO_NOT_WHITE(line.content, i)
	/* Empty/Comment line - no processing needed! */
	if (line.content[i] == ';' || line.content[i] == '\n') return TRUE;
	indexOfColon = strchr(line.content, ':');
	/*check for label */
	if (indexOfColon != NULL) {
		i = indexOfColon - line.content;
		i++;
	}
	
	MOVE_TO_NOT_WHITE(line.content, i)

	/* .instruction */
	if (line.content[i] == '.') {
	
		/*if it's entry we add it to the symbol table*/

		if (strncmp(".entry", line.content, 6) == 0) {
		
			i += 6;
			MOVE_TO_NOT_WHITE(line.content, i)
			token = strtok(line.content + i, " \n\t");
			
			/* if label is already marked as entry, ignore. */
			if (token == NULL) {
			
				printf_line_error(line, "You have to specify a label name for .entry instruction.");
				return FALSE;
			}
			
			if (find_by_types(*symbol_table, token, 1, ENTRY_SYMBOL) == NULL) {
				table_entry *entry;
				token = strtok(line.content + i, "\n"); /*get name of label*/
				if (token[0] == '&') token++;
				/* if symbol is not defined as data/code */
				if ((entry = find_by_types(*symbol_table, token, 2, DATA_SYMBOL, CODE_SYMBOL)) == NULL) {
					/* if defined as external print error */
					if ((entry = find_by_types(*symbol_table, token, 1, EXTERNAL_SYMBOL)) != NULL) {
						printf_line_error(line, "The symbol %s can be either external or entry, but not both.",
						                  entry->key);
						return FALSE;
					}
					printf_line_error(line, "The symbol %s for .entry is undefined.", token);
					return FALSE;
				}
				
				add_table_item(symbol_table, token, entry->addr, ENTRY_SYMBOL);
			}
		}
		return TRUE;
	}
	
	/* its a code command ! check if we need to "fix" the code */
	
	curr_opcode = (code_img[(*codeIndex)]->opcode);

	if ((curr_opcode >= 15 && curr_opcode <= 18) || (curr_opcode >= 30 && curr_opcode <= 32)){
		return add_symbols_to_code(line, curr_opcode, codeIndex, code_img, symbol_table);
	}
	else {
		(*codeIndex)++;
		return TRUE;
	}
}

/**
 * Find the symbol that need replacment in the code line and put the address of the label by searching in the symbol table.
 * @param line The current code line that is being processed
 * @param curr_opcode The current opcode
 * @param codeIndex A pointer to current code index of code img
 * @param code_img The code img array
 * @param symbol_table The symbol table
 * @return whether succeeded
 */
bool add_symbols_to_code(line_info line, opcode curr_opcode, long *codeIndex, code_word **code_img, table *symbol_table) {
	char temp[MAX_LINE_LENGTH];
	char *operands[3];
	int i,j, operand_count;
	bool isvalid = TRUE;
	i=j=0;

	/* skip command and get the operands */
	MOVE_TO_NOT_WHITE(line.content, i)
	find_label(line, temp);
	if (temp[0] != '\0') { /* if symbol is defined */
		for (; line.content[i] && line.content[i] != '\n' && line.content[i] != EOF && line.content[i] != ' ' && line.content[i] != '\t'; i++);
			i++;
	}
	MOVE_TO_NOT_WHITE(line.content, i)
	/* now skip command */
	for (; line.content[i] && line.content[i] != ' ' && line.content[i] != '\t' && line.content[i] != '\n' && line.content[i] != EOF; i++);
	MOVE_TO_NOT_WHITE(line.content, i)
	/* now analyze operands We send NULL as string of command because no error will be printed, and that's the only usage for it there. */
	analyze_operands(line, i, operands, &operand_count, NULL);
	/* Process operands */	
	/* we need to check only RS */
	if(curr_opcode == JMP_OP || curr_opcode == LA_OP || curr_opcode == CALL_OP){
		isvalid = process_spass_operand(line, operands[0], curr_opcode, codeIndex, code_img, symbol_table);
	}
	/* we need to check only RD */					
	else if(curr_opcode >= 15 && curr_opcode <= 18) {
		isvalid = process_spass_operand(line, operands[2], curr_opcode, codeIndex, code_img, symbol_table);
	}	
		
	while(j < 3 && operands[j] != NULL){ /* free maximum 3 operands */
			free(operands[j]);
			j++;
	}

	(*codeIndex)++;
	
	if (!isvalid) return FALSE;
	
	return TRUE;
}

/**
 * @param line The current code line that is being processed
 * @param curr_opcode The current opcode
 * @param codeIndex The current code index of code img
 * @param code_img The code image array
 * @param symbol_table The symbol table
 * @return Whether succeeded
 */
int process_spass_operand(line_info line, char *operand, opcode curr_opcode, long *codeIndex, code_word **code_img, table *symbol_table) {
	cmd_type type;
	addressing_type addr = get_addressing_type(operand);

	/* if [ call || jump && address operator || la ] cmd */
	/* or if BNE/BEQ/BLT/BGT */ 
	if((curr_opcode == JMP_OP && addr!=REGISTER_ADDR) || (curr_opcode >= 15 && curr_opcode <= 18) || 
							curr_opcode == LA_OP || curr_opcode == CALL_OP) {
		long data_to_add;
		table_entry *entry = find_by_types(*symbol_table, operand, 3, DATA_SYMBOL, CODE_SYMBOL, EXTERNAL_SYMBOL);
		if (entry == NULL) {
			printf_line_error(line, "The symbol %s not found", operand);
			return FALSE;
		}
		/*found symbol*/
		data_to_add = entry->addr;
		/* Calculate the distance to the label from ic if needed */
		if(curr_opcode >= 15 && curr_opcode <= 18) {
			/* if not code symbol it's impossible to calculate distance! */
			if (entry->type != CODE_SYMBOL) {
				printf_line_error(line, "The symbol %s cannot be addressed relatively because it's not a code symbol.", operand);
				return FALSE;
			}
			data_to_add = data_to_add - (code_img[(*codeIndex)]->ic);
			
			/* check if the label distance exceeds signed halfword 16 bits range <-32768 - 32767> */
			if(!(data_to_add >= -32768 && data_to_add <= 32767)){
				printf_line_error(line, "The symbol %s cannot be addressed relatively because the label distance exceeds halfword range <-32768 - 32767>. got: <%ld>.", operand, data_to_add);
			}
		}
		
	
		/* Add to externals reference table if it's an external. */
		if (entry->type == EXTERNAL_SYMBOL) {
			add_table_item(symbol_table, operand, (code_img[(*codeIndex)]->ic), EXTERNAL_REFERENCE);
		}
		
		type = (code_img[(*codeIndex)])->type;

		/*if i_cmd or j_cmd */
		if(type == I_CMD)
			(code_img[(*codeIndex)])->typeword.iword.immed = data_to_add;
		else{
			/* its a J_CMD so we need to check if the label's address exceeds 25 bits size <MAX_25BITS = 33,554,431> */
			if(data_to_add > MAX_25BITS){
				printf_line_error(line, "The symbol %s cannot be addressed directly because the label address exceeds 25 bits size <33,554,431>. got: <%ld>.", data_to_add);
				return FALSE;
			}
			(code_img[(*codeIndex)])->typeword.jword.addressL = (data_to_add & 0xFF);
			(code_img[(*codeIndex)])->typeword.jword.addressH = (data_to_add & 0xFF00);
		}			
	}
	return TRUE;
}
