#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "writefiles.h"
#include "utils.h"
#include "first_pass.h"
#include "second_pass.h"

/**
 * Processes a single assembly source file, and returns the result status.
 * @param filename The filename, without it's extension
 * @return Whether succeeded
 */
static bool process_file(char *filename);

int main(int argc, char *argv[]) {
	int i;
	/* To break line if needed */
	bool succeeded = TRUE;
	/* Process each file by arguments */
	for (i = 1; i < argc; ++i) {
		/* if last process failed and there's another file, break line: */
		if (!succeeded) puts("");
		/* foreach argument (file name), send it for full processing. */
		succeeded = process_file(argv[i]);
		/* Line break if failed */
	}
	return 0;
}

static bool process_file(char *filename) {
	/* Memory address counters */
	int temp_c;
	long ic = IC_INIT_VALUE, dc = 0, icf, dcf, codeIndex = 0, dataIndex = 0;
	bool is_success = TRUE; /* current success status */
	char *input_filename;
	char temp_line[MAX_LINE_LENGTH + 2]; /* a line from source file */
	FILE *file_des; 
	data_word data_img[CODE_ARR_IMG_LENGTH]; /* image of the data words */
	code_word *code_img[CODE_ARR_IMG_LENGTH];/* image of the code words */
	
	/* symbol table */
	table symbol_table = NULL;
	line_info curr_line_info;

	/* add .as extension */
	input_filename = strallocat(filename, ".as");

	/* Open file, skip on failure */
	file_des = fopen(input_filename, "r");
	if (file_des == NULL) {
		/* if file couldn't be opened, write to stderr. */
		printf("Error: file \"%s.as\" is inaccessible for reading. skipping file process.\n", filename);
		free(input_filename); /* free file name */
		return FALSE;
	}

/* ############################# first pass ################################# */
	curr_line_info.file_name = input_filename;
	curr_line_info.content = temp_line; /* the line content string */
	
	/* Read line - stop if read failed (when NULL returned) - usually when EOF. increase line counter for error printing. */
	for (curr_line_info.line_number = 1;
	     fgets(temp_line, MAX_LINE_LENGTH + 2, file_des) != NULL; curr_line_info.line_number++) {
		/* if line too long, the buffer doesn't include the '\n' char OR the file isn't on end. */
		if (!is_valid_length(temp_line) && !feof(file_des)) {
			/* Print message and prevent further line processing, as well as second pass.  */
			printf_line_error(curr_line_info, "Line too long to process. Maximum line length should be %d.",
			                  MAX_LINE_LENGTH);
			is_success = FALSE;
			/* skip leftovers */
			do {
				temp_c = fgetc(file_des);
			} while (temp_c != '\n' && temp_c != EOF);
		}
		else {

			if (!process_line_fpass(curr_line_info, &ic, &dc, &codeIndex, &dataIndex, code_img, data_img, &symbol_table)) {
				if (is_success) {
					icf = -1;
					is_success = FALSE;
				}
			}		
		}
	}
	
	/* Save ICF & DCF  */
	icf = ic;
	dcf = dc;
 
/* ######################### SECOND PASS ################################ */

	/* if first pass didn't fail, start the second pass */
	if (is_success) {
		ic = IC_INIT_VALUE;
		/* add IC to the DC of data symbols and data_img */
		add_value_to_type(symbol_table, icf, DATA_SYMBOL);
		add_value_to_data(data_img, icf, dcf, dataIndex);
		
		rewind(file_des); /* Start from beginning of file again */
		codeIndex = 0;
		for (curr_line_info.line_number = 1;  !feof(file_des); curr_line_info.line_number++) {
			int i = 0;
			fgets(temp_line, MAX_LINE_LENGTH, file_des); /* get line */
			/*
			if(!temp_line[0] || temp_line[0] == '\n' || temp_line[0] == ' ' ||temp_line[0] == '	')
			continue;
			*/
			MOVE_TO_NOT_WHITE(temp_line, i)
			/* process line second pass */
			if (code_img[codeIndex] != NULL || temp_line[i] == '.')		{
				is_success &= process_line_spass(curr_line_info, &codeIndex, code_img, &symbol_table);
			}
		}

		/* Write files if second pass succeeded */
		if (is_success)
		/* Lets build .ob .ext .ent files */
			is_success = write_output_files(codeIndex, dataIndex, code_img, data_img, icf, dcf, filename, symbol_table);
	}

	/* free pointers */
	/* free current file name */
	free(input_filename);
	/* Free symbol table */
	free_table(symbol_table);
	/* Free code image and data image */
	free_code_image(code_img, codeIndex);

	/* return success status of file processing */
	return is_success;
}
