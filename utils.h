/* Contains general-purposed functions, for both passes and many usages */
#ifndef _UTILS_H
#define _UTILS_H

#include "globals.h"


/** moves the index to the next place in string where the char isn't white */
#define MOVE_TO_NOT_WHITE(string, index) \
        for (;string[(index)] && (string[(index)] == '\t' || string[(index)] == ' '); (++(index)))\
        ;


/*
 * check the string length and returns true if length < MAX_LINE_LENGTH
 * otherwise false
*/
bool is_valid_length(char *str);


/**
 * Concatenates both string to a new allocated memory
 * @param s0 The first string
 * @param s1 The second string
 * @return A pointer to the new, allocated string
 */
char *strallocat(char *s0, char* s1);

/**
 * Finds the defined label in the code if exists, and saves it into the buffer.
 * Returns whether syntax error found.
 * @param line The source line to find in
 * @param symbol_dest The buffer for copying the found label in
 * @return Whether syntax error found
 */
bool find_label(line_info line, char *symbol_dest);

/**
 * Returns the instruction enum by the instruction's name, without the opening '.'
 * @param name The instruction name, without the '.'
 * @return The instruction enum if found, NONE_INST if not found.
 */
instruction find_instruction_by_name(char *name);

/**
 * Returns the cmd type (R/I/J)
 * @param the cmd op code to test
 * @return the cmd type (R/I/J), NONE_CMD if not found.
 */
cmd_type get_type(opcode op);

/**
 * Returns whether the string is a valid 21-bit integer
 * @param string The number string
 * @return Whether a valid 21-bit signed integer.
 */
bool is_int(char* string);

/**
 * Allocates memory in the required size. Exits the program if failed.
 * @param size The size to allocate in bytes
 * @return A generic pointer to the allocated memory if succeeded
 */
void *malloc_with_check(long size);

/**
 * Returns whether a label can be defined with the specified name.
 * @param name The label name
 * @return Whether the specified name is valid,
 */
bool is_valid_label_name(char* name);

/**
 * Returns a string of address name.
 * @param address - the address type.
 * @return Returns a string of address name.
 */
char* get_addressing_str(addressing_type address);


/**
 * Adds the value to add into the value of each data word IC
 * @param data_img The img of data code, containing the IC entries
 * @param to_add The value to add
 * @param dataIndex The index of the current data word
 */
void add_value_to_data(data_word *data_img, long icf, long dcf, long dataIndex);

/* is equals max int */
bool is_max(char *num);

/* is equals min int */
bool is_min(char *num);

/**
 * Returns whether a string is alphanumeric.
 * @param string The string
 * @return Whether it's alphanumeric
 */
bool is_alphanumeric_str(char *string);

/*Returns TRUE if name is saved word*/
bool is_reserved_word(char *name);

/**
 * Prints a detailed error message, including file name and line number by the specified message,
 * formatted as specified in App. B of "The C Programming language" for printf.
 * @param message The error message
 * @param ... The arguments to format into the message
 * @return printf result of the message
 */
int printf_line_error(line_info line, char *message, ...);

/**
 * Prints a detailed error message, including file name and line number by the specified message,
 * formatted as specified in App. B of "The C Programming language" for printf.
 * @param message The error message
 * @param ... The arguments to format into the message
 * @return printf result of the message
 */
int printf_line_warning(line_info line, char *message, ...);

/**
 * Frees all the dynamically-allocated memory for the code image.
 * @param code_image A pointer to the code images buffer
 * @param fic The final instruction counter value
 */
void free_code_image(code_word **code_image, long fic);

#endif
