/* some functions to process and analyze code */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdlib.h>
#include "code.h"
#include "utils.h"




bool analyze_operands(line_info line, int i, char **destination, int *operand_count, char *c) {
	int j;
	*operand_count = 0;
	destination[0] = destination[1] = destination[2] = NULL;
	MOVE_TO_NOT_WHITE(line.content, i)
	if (line.content[i] == ',') {
		printf_line_error(line, "Unexpected comma after command.");
		return FALSE; /* an error occurred */
	}

	/* Until not too many operands (max of 3) and it's not the end of the line */
	for (*operand_count = 0; line.content[i] != EOF && line.content[i] != '\n' && line.content[i];) {
		if (*operand_count == 3) /* =We already got 3 operands in, We're going ot get the 4th! */ {
			printf_line_error(line, "Too much operands got: <%d> max possible operands is 3.", *operand_count);
			free(destination[0]);
			free(destination[1]);
			free(destination[2]);
			return FALSE; /* an error occurred */
		}

		/* Allocate memory to save the operand */
		destination[*operand_count] = malloc_with_check(MAX_LINE_LENGTH);
		/* as long we're still on same operand */
		for (j = 0; line.content[i] && line.content[i] != '\t' && line.content[i] != ' ' && line.content[i] != '\n' && line.content[i] != EOF &&
		            line.content[i] != ','; i++, j++) {
			destination[*operand_count][j] = line.content[i];
		}
		destination[*operand_count][j] = '\0';
		(*operand_count)++; /* We've just saved another operand! */
		MOVE_TO_NOT_WHITE(line.content, i)
		
		
		if (line.content[i] == '\n' || line.content[i] == EOF || !line.content[i]) break;
		else if (line.content[i] != ',') {
			/* After operand & after white chars there's something that isn't ',' or end of line.. */
			printf_line_error(line, "Expecting ',' between operands");
			/* RELEASE_OPERANDS */
			j=0;
			while(j < (*operand_count)){ /* free maximum 3 operands */
				free(destination[j]);
				j++;
			}
			
			return FALSE;
		}
		i++;
		MOVE_TO_NOT_WHITE(line.content, i)
		/* if there was just a comma, then (optionally) white char(s) and then end of line */
		
		if (line.content[i] == '\n' || line.content[i] == EOF || !line.content[i])
			printf_line_error(line, "Missing operand after comma.");
		
		else if (line.content[i] == ',') printf_line_error(line, "Multiple consecutive commas.");
		
		else continue; /* No errors, continue */
		/* Error found! (didn't continue) */
		/* RELEASE_OPERANDS */
		j=0;
		while(j < (*operand_count)){ /* free maximum 3 operands */
			free(destination[j]);
			j++;
		}
		
		return FALSE;
		
	}
	return TRUE;
}


/**
 * A single lookup table element
 */
struct cmd_lookup_element {
	char *cmd;
	opcode op;
	funct fun;
};


/**
 * A lookup table for opcode & funct by command name
 */
static struct cmd_lookup_element lookup_table[] = {
		/* TYPE OF R */
		{"add",ADD_OP, ADD_FUNCT},
		{"sub",SUB_OP, SUB_FUNCT},
		{"and",AND_OP, AND_FUNCT},
		{"or",OR_OP, OR_FUNCT},
		{"nor",NOR_OP, NOR_FUNCT},
		{"move",MOVE_OP, MOVE_FUNCT},
		{"mvhi",MVHI_OP, MVHI_FUNCT},
		{"mvlo",MVLO_OP, MVLO_FUNCT},
		/* TYPE OF I */
		{"addi",ADDI_OP, NONE_FUNCT},
		{"subi",SUBI_OP, NONE_FUNCT},
		{"andi",ANDI_OP, NONE_FUNCT},
		{"ori",ORI_OP, NONE_FUNCT},
		{"nori",NORI_OP, NONE_FUNCT},
		{"bne",BNE_OP, NONE_FUNCT},
		{"beq",BEQ_OP, NONE_FUNCT},
		{"blt",BLT_OP, NONE_FUNCT},
		{"bgt",BGT_OP, NONE_FUNCT},
		{"lb",LB_OP, NONE_FUNCT},
		{"sb",SB_OP, NONE_FUNCT},
		{"lw",LW_OP, NONE_FUNCT},
		{"sw",SW_OP, NONE_FUNCT},
		{"lh",LH_OP, NONE_FUNCT},
		{"sh",SH_OP, NONE_FUNCT},
		/* TYPE OF J */
		{"jmp",JMP_OP, NONE_FUNCT},
		{"la",LA_OP, NONE_FUNCT},
		{"call",CALL_OP, NONE_FUNCT},
		{"stop",STOP_OP, NONE_FUNCT},
		
		{NULL, NONE_OP, NONE_FUNCT}
};



void get_opcode_func(char *cmd, opcode *opcode_out, funct *funct_out) {
	struct cmd_lookup_element *e;
	*opcode_out = NONE_OP;
	*funct_out = NONE_FUNCT;
	/* iterate through the lookup table, if commands are same update opcode and funct. */

	for (e = lookup_table; e->cmd != NULL; e++) {
		if (strcmp(e->cmd, cmd) == 0) {
			*opcode_out = e->op;
			*funct_out = e->fun;
			return;
		}
	}
}


addressing_type get_addressing_type(char *operand) {
	/* if nothing, just return none */
	if (operand[0] == '\0') return NONE_ADDR;
	/* if it's a register */
	if(get_register_by_name(operand) != NONE_REG) return REGISTER_ADDR;
	/* if it's immediately addressed */
	else if (is_int(operand)) return IMMEDIATE_ADDR;
	/* if operand is a valid label name, it's directly addressed */
	else if (is_valid_label_name(operand)) return DIRECT_ADDR;
	else return NONE_ADDR;
}




/**
 * Validates the operands addressing types by the opcode of the instruction
 * also prints an error message if needed.
 * @param line The current source line info
 * @param curr_opcode The opcode of the current instruction
 * @param op_count The operand count of the current instruction
 * @param operands An array includes the operands string.
 * @return Whether valid addressing
 */
bool validate_operand_by_opcode(line_info line, opcode curr_opcode, int op_count, char *operands[3]) {
                                
                              
	/* Get <op_count> addressing types and validate them: */
	addressing_type first_addressing = op_count >= 1 ? get_addressing_type(operands[0]) : NONE_ADDR;
	addressing_type second_addressing = op_count >= 2 ? get_addressing_type(operands[1]) : NONE_ADDR;
	addressing_type third_addressing = op_count == 3 ? get_addressing_type(operands[2]) : NONE_ADDR;
    



	if (curr_opcode == 0 || (curr_opcode >= 10 && curr_opcode <= 24)) {
		/* 3 operands required */
		if (op_count != 3) {
			printf_line_error(line, "Operation requires 3 operands. got <%d> operands", op_count);
			return FALSE;
		}

		/* validate operands addressing */
		
		/* add,sub,and,or,nor */
		if (curr_opcode == 0){
			if(first_addressing != REGISTER_ADDR){
				printf_line_error(line, "Invalid addressing mode for first operand. expected for a valid REGISTER $0-$31 name. got: <%s>", operands[0]);
				return FALSE;
			}
			if(second_addressing != REGISTER_ADDR){
				printf_line_error(line, "Invalid addressing mode for second operand. expected for a valid REGISTER $0-$31 name. got: <%s>", operands[1]);
				return FALSE;
			}
			if(third_addressing != REGISTER_ADDR){
				printf_line_error(line, "Invalid addressing mode for third operand. expected for a valid REGISTER $0-$31 name. got: <%s>", operands[2]);
				return FALSE;
			}
			return TRUE;
		}
		
		/* addi,subi,andi,ori,nori OR lb,sb,lw,sw,lh,sh */
		if((curr_opcode >= 10 && curr_opcode <= 14) || (curr_opcode >= 19 && curr_opcode <= 24)){
			if(first_addressing != REGISTER_ADDR){
				printf_line_error(line, "Invalid addressing mode for first operand. expected for a valid REGISTER $0-$31 name. got: <%s>", operands[0]);
				return FALSE;
			}
			if(second_addressing != IMMEDIATE_ADDR){
				printf_line_error(line, "Invalid addressing mode for second operand. expected for an IMMEDIATE number. got: <%s>", operands[1]);
				return FALSE;
			}
			if(third_addressing != REGISTER_ADDR){
				printf_line_error(line, "Invalid addressing mode for third operand. expected for a valid REGISTER $0-$31 name. got: <%s>", operands[2]);
				return FALSE;
			}
			return TRUE;
		}
			
		/* bne,beq,blt,bgt */
		if (curr_opcode >= 15 && curr_opcode <= 18){
			if(first_addressing != REGISTER_ADDR){
				printf_line_error(line, "Invalid addressing mode for first operand. expected for a valid REGISTER $0-$31 name. got: <%s>", operands[0]);
				return FALSE;
			}
			if(second_addressing != REGISTER_ADDR){
				printf_line_error(line, "Invalid addressing mode for second operand. expected for a valid REGISTER $0-$31 name. got: <%s>", operands[1]);
				return FALSE;
			}
			if(third_addressing != DIRECT_ADDR){
				printf_line_error(line, "Invalid addressing mode for first operand. expected for a string of LABEL name. got: <%s>", operands[2]);
				return FALSE;
			}
			return TRUE;
		}
	}
	
	/* move,mvhi,mvlo */
	else if (curr_opcode == 1) {
		/* 2 operands required */
		if (op_count != 2) {
			printf_line_error(line, "Operation requires 2 operands. got: <%d> operands", op_count);
			return FALSE;
		}
		/* validate operand addressing */
		if (curr_opcode == 1) {
		
			if(first_addressing != REGISTER_ADDR){
				printf_line_error(line, "Invalid addressing mode for first operand. expected for a valid REGISTER $0-$31 <got: %s>", operands[0]);
				return FALSE;
			}
			if(second_addressing != REGISTER_ADDR){
				printf_line_error(line, "Invalid addressing mode for second operand. expected for a valid REGISTER $0-$31 <got: %s>", operands[1]);
				return FALSE;
			}
			return TRUE;
		}
		
	}
	
	/* jmp,la,call */
	else if (curr_opcode >= 30 && curr_opcode <= 32) {
		/* 1 operand required */
		if (op_count != 1) {
			printf_line_error(line, "Operation requires 1 operand (got %d)", op_count);
			return FALSE;
		}
		/* validate operand addressing */
		if (curr_opcode == JMP_OP){ 
			if(first_addressing != REGISTER_ADDR && first_addressing != DIRECT_ADDR){
				printf_line_error(line, "Invalid addressing mode for first operand. expected for a valid REGISTER $0-$31 or a string of LABEL name. got: <%s>", operands[0]);
				return FALSE;
			}
			return TRUE;
		}
		
		if (curr_opcode == LA_OP || curr_opcode == CALL_OP){ 
			if(first_addressing != DIRECT_ADDR){
				printf_line_error(line, "Invalid addressing mode for first operand. expected for a string of LABEL name. got: <%s>", operands[0]);
				return FALSE;
			}
			return TRUE;
		}
	}
	/* stop */		
	else if (curr_opcode == STOP_OP && op_count > 0) {
		/* 0 operands required */
			printf_line_error(line, "Operation STOP requires no operands. got: <%d> operands.", op_count);
			return FALSE;
	}
	
	return TRUE;
}


r_word get_r_code(opcode curr_opcode, funct curr_funct, int op_count, char *operands[3])
{
	r_word codeword;
	
	/*initiallize values*/
	
	if (curr_opcode == 0) { /* 3 registers */
		codeword.funct = curr_funct;
		codeword.unused = 0;
		codeword.RS = get_register_by_name(operands[0]);
		codeword.RT = get_register_by_name(operands[1]);
		codeword.RD = get_register_by_name(operands[2]);
	}
	else if (curr_opcode == 1) { /* 2 operands both registers */
		codeword.funct = curr_funct;
		codeword.RT = 0;		
		codeword.unused = 0;
		codeword.RS = get_register_by_name(operands[0]);
		codeword.RD = get_register_by_name(operands[1]);
	}
					
	return codeword;
}


i_word get_i_code(opcode curr_opcode, int op_count, char *operands[3])
{
	i_word codeword;
	
	/*initiallize values*/
	
	if ((curr_opcode >= 10 && curr_opcode <= 14) || 
	(curr_opcode >= 19 && curr_opcode <= 24)) { /* 3 operands registers assebly cmd: cmd rs,imm,rd */
		char *ptr; 
		codeword.RS = get_register_by_name(operands[0]);
		codeword.RT = get_register_by_name(operands[2]);
		codeword.immed = strtol(operands[1] , &ptr, 10);
		
	}
	else if (curr_opcode >= 15 && curr_opcode <= 18){ /* 3 operands registers assebly cmd: cmd rs,imm,rd */
		codeword.RS = get_register_by_name(operands[0]);
		codeword.RT = get_register_by_name(operands[1]);
	}
	return codeword;
}

j_word get_j_code(opcode curr_opcode, int op_count, char *operands[3])
{
	j_word codeword;
	
	
	/*initiallize values*/
	
	if (curr_opcode == JMP_OP) { /* 1 operands register/label */
		addressing_type operand_addressing = get_addressing_type(operands[0]);
		if(operand_addressing == REGISTER_ADDR){
			codeword.reg = 1;
			codeword.addressL = get_register_by_name(operands[0]);
			codeword.addressH = 0;
		}
		else{
			codeword.reg = 0;
			codeword.addressL = 0;
		}
		codeword.addressH = 0;
	}
	
	else if (curr_opcode == LA_OP || CALL_OP) { /* 0 operands */
		codeword.reg = 0;
		codeword.addressL = codeword.addressH = 0;
	}

	else if (curr_opcode == STOP_OP) { /* 1 operands register/label */
		codeword.reg = 0;
		codeword.addressL = codeword.addressH = 0;
	}
	return codeword;
}


reg get_register_by_name(char *name) {
	if ( name[0] == '$' && isdigit(name[1]) ){
		int flag=-1;
		int regValue=-1;
		if( name[2] == '\0') /* one digit register*/
			flag=1;			
		else if( isdigit(name[2]) && name[3] == '\0' ) /* two digit register*/
			flag=2;
		if(flag == 2) {
			regValue = (name[1] - '0')*10; /* to get the digit at the 10^1 place we need to multiply the converted digit by 10. convert ascii digit a natural number (10/20/30) */
			regValue += (name[2] - '0'); /* convert ascii digit to a natural number (0-9) */
		}
		else if(flag == 1)
			regValue = (name[1] - '0');
		if (regValue >= 0 && regValue <= 31) return regValue;
	}
	return NONE_REG; /* Not valid register*/
}

