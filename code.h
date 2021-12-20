#ifndef _CODE_H
#define _CODE_H
#include "table.h"
#include "globals.h"



/**
 * Detects the opcode and the funct of a command by it's name
 * @param cmd The command name (string)
 * @param opcode_out The opcode value destination
 * @param funct_out The funct value destination
 */
void get_opcode_func(char *cmd, opcode *opcode_out, funct *funct_out);

/**
 * Returns the addressing type of an operand
 * @param operand The operand's string
 * @return The addressing type of the operand
 */
addressing_type get_addressing_type(char *operand);

/**
 * Validates and builds a type of R code word by the opcode, funct, operand count and operands string
 * @param curr_opcode The current opcode
 * @param curr_funct The current funct
 * @param op_count The operands count
 * @param operands An array includes the operands string.
 * @return A pointer to code word struct, which represents the code. if validation fails, returns NULL.
 */
r_word get_r_code(opcode curr_opcode, funct curr_funct, int op_count, char *operands[3]);


/**
 * Validates and builds a type of I code word by the opcode, funct, operand count and operands string
 * @param curr_opcode The current opcode
 * @param op_count The operands count
 * @param operands An array includes the operands string.
 * @return A pointer to code word struct, which represents the code. if validation fails, returns NULL.
 */
i_word get_i_code(opcode curr_opcode, int op_count, char *operands[3]);


/**
 * Validates and builds a type of J code word by the opcode, operand count and operands string
 * @param curr_opcode The current opcode
 * @param op_count The operands count
 * @param operands An array includes the operands string.
 * @return A pointer to code word struct, which represents the code. if validation fails, returns NULL.
 */
j_word get_j_code(opcode curr_opcode, int op_count, char *operands[3]);



/**
 * Validates the operands addressing types by the opcode of the instruction
 * also prints an error message if needed.
 * @param line The current source line info
 * @param curr_opcode The opcode of the current instruction
 * @param op_count The operand count of the current instruction
 * @param operands An array includes the operands string.
 * @return Whether valid addressing
 */
bool validate_operand_by_opcode(line_info line, opcode curr_opcode, int op_count, char *operands[3]);

/**
 * Returns the register enum value by it's name
 * @param name The name of the register
 * @return The enum value of the register if found. otherwise, returns NONE_REG
 */
reg get_register_by_name(char *name);


/**
 * Separates the operands from a certain index, puts each operand into the destination array,
 * and puts the found operand count in operand count argument
 * @param line The command text
 * @param i The index to start analyzing from
 * @param destination At least a 2-cell buffer of strings for the extracted operand strings
 * @param operand_count The destination of the detected operands count
 * @param command The current command string
 * @return Whether analyzing succeeded
 */
bool analyze_operands(line_info line, int i, char **destination, int *operand_count, char *command);

#endif
