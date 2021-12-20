/* Output files related functions */
#ifndef _WRITEFILES_H
#define _WRITEFILES_H
#include "globals.h"
#include "table.h"

/**
 * Writes the output files (.ob , .ent , .ext) of a single file
 * @param codeIndex The current code img index
 * @param dataIndex The current data img index
 * @param code_img The code image
 * @param data_img The data image
 * @param icf The final instruction counter
 * @param dcf The final data counter
 * @param filename The filename (without the extension)
 * @param symbol_table The symbol table
 * @return Whether succeeded
 */
int write_output_files(long codeIndex, long dataIndex, 
code_word **code_img, data_word *data_img, long icf, long dcf,
char *filename, table symbol_table);

#endif
