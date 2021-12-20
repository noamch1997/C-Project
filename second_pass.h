#ifndef _SECOND_PASS_H
#define _SECOND_PASS_H
#include "globals.h"
#include "table.h"


#define MAX_25BITS 33554431

/**
 * Processes a single line in the second pass
 * @param line The line string
 * @param codeIndex A pointer to current code index of code img
 * @param code_img Code image
 * @param symbol_table The symbol table
 * @return Whether operation succeeded
 */
bool process_line_spass(line_info line, long *codeIndex, code_word **code_img, table *symbol_table);

/**
 * Find the symbol that need replacment in a code line, and replacing them by the address in the symbol table.
 * @param line The current code line that is being processed
 * @param curr_opcode The current opcode
 * @param codeIndex A pointer to current code index of code img
 * @param code_img The code img array
 * @param symbol_table The symbol table
 * @return whether succeeded
 */
bool add_symbols_to_code(line_info line, opcode curr_opcode, long *codeIndex, code_word **code_img, table *symbol_table);

#endif
