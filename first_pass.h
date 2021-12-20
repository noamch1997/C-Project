#ifndef _FIRST_PASS_H
#define _FIRST_PASS_H

#include "globals.h"

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
bool process_line_fpass(line_info line, long *ic, long *dc, long *codeIndex, long *dataIndex,
						code_word **code_img, data_word *data_img, table *symbol_table);

#endif
