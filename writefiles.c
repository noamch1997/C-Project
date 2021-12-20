#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "utils.h"
#include "table.h"


/**
 * Writes the code and data image into an .ob file and write the lengths of icf&dcf on top
 * @param codeIndex The ending index of the code image
 * @param dataIndex The ending index of the data image
 * @param code_img The code image
 * @param data_img The data image
 * @param icf The final instruction counter
 * @param dcf The final data counter
 * @param filename The filename, without the extension
 * @return Whether succeeded
 */
static bool write_ob(long codeIndex, long dataIndex, code_word **code_img, data_word *data_img, long icf, long dcf, char *filename);

/**
 * Writes a symbol table to a file. Each symbol and it's address in line, separated by a single space.
 * @param tab The symbol table to write
 * @param filename The filename without the extension
 * @param file_extension The extension of the file, including dot before
 * @return Whether succeeded
 */
static bool write_table_to_file(table tab, char *filename, char *file_extension);

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
char *filename, table symbol_table) {

	bool result;
	table externals = filter_table_by_type(symbol_table, EXTERNAL_REFERENCE);
	table entries = filter_table_by_type(symbol_table, ENTRY_SYMBOL);
	
	/* Write .ob file */
	result = write_ob(codeIndex, dataIndex, code_img, data_img, icf, dcf, filename) &&
	         /* Write *.ent and *.ext files , filtered by externals and entries*/
	         write_table_to_file(externals, filename, ".ext") &&
	         write_table_to_file(entries, filename, ".ent");
	         
	/* Release filtered tables */
	free_table(externals);
	free_table(entries);
	return result;
}

/**
 * Writes the .ob file
 * @param codeIndex The ending index of the code image
 * @param dataIndex The ending index of the data image
 * @param code_img The code image
 * @param data_img The data image
 * @param icf The final instruction counter
 * @param dcf The final data counter
 * @param filename The filename, without the extension
 * @return Whether succeeded
 */
static bool write_ob(long codeIndex, long dataIndex, code_word **code_img, data_word *data_img, long icf, long dcf, char *filename) {
	int bytes; /* bytes counter for the current command */
	instruction type;
	int j,addr=IC_INIT_VALUE;
	int temp;
	long curr_index;
	long mask = 0xFF;
	long value=0;
	FILE *file_desc;
	int cnt; /* bytes left for current address line */
	int curr_opcode;
	/* add extension of file to open */
	char *output_filename = strallocat(filename, ".ob");
	/* Try to open the file for writing */
	file_desc = fopen(output_filename, "w");
	free(output_filename);
	if (file_desc == NULL) {
		printf("Can't create or rewrite to file %s.", output_filename);
		return FALSE;
	}

	/* print data/code word count on top */
	fprintf(file_desc, "     %ld %ld", icf - IC_INIT_VALUE, dcf);

	for (curr_index = 0; curr_index < codeIndex ; curr_index++) {
		curr_opcode = code_img[curr_index]->opcode;		
		/* add,sub,and,or,nor */
		if (curr_opcode == 0)			
			value = (code_img[curr_index]->opcode << 26) |
				  (code_img[curr_index]->typeword.rword.RS << 21) |
			      (code_img[curr_index]->typeword.rword.RT << 16) |
			      (code_img[curr_index]->typeword.rword.RD << 11) |
			      (code_img[curr_index]->typeword.rword.funct << 6);
		/* move,mvhi,mvlo */
		if (curr_opcode == 1) 
			value = (code_img[curr_index]->opcode << 26) |
			(code_img[curr_index]->typeword.rword.RS << 21) | 
			(code_img[curr_index]->typeword.rword.RD << 16) |
			(code_img[curr_index]->typeword.rword.funct << 6);	
		/* addi,subi,andi,ori,nori */
		if (curr_opcode >= 10 || curr_opcode <= 14)
			value = (code_img[curr_index]->opcode << 26) |
				  (code_img[curr_index]->typeword.iword.RS << 21) |
			      (code_img[curr_index]->typeword.iword.RT << 16) |
			      (code_img[curr_index]->typeword.iword.immed);		      
		/* bne,beq,blt,bgt */
		if (curr_opcode >= 15 || curr_opcode <= 18)
			value = (code_img[curr_index]->opcode << 26) |
				  (code_img[curr_index]->typeword.iword.RS << 21) |
			      (code_img[curr_index]->typeword.iword.RT << 16) |
			      (code_img[curr_index]->typeword.iword.immed);
		/* lb,sb,lw,sw,lh,sh */
		if (curr_opcode >= 19 || curr_opcode <= 24)
			value = (code_img[curr_index]->opcode << 26) |
				  (code_img[curr_index]->typeword.iword.RS << 21) |
			      (code_img[curr_index]->typeword.iword.RT << 16) |
			      (code_img[curr_index]->typeword.iword.immed);
		/* jmp,la,call */
		if (curr_opcode == JMP_OP)
			value = (code_img[curr_index]->opcode << 26) |
				  (code_img[curr_index]->typeword.jword.reg << 25) |
			      (code_img[curr_index]->typeword.jword.addressH << 16) |
			      (code_img[curr_index]->typeword.jword.addressL);
		if (curr_opcode == LA_OP) 
			value = (code_img[curr_index]->opcode << 26) |
			      (code_img[curr_index]->typeword.jword.addressH << 16) |
			      (code_img[curr_index]->typeword.jword.addressL);
		if (curr_opcode == CALL_OP) 
			value = (code_img[curr_index]->opcode << 26) |
			      (code_img[curr_index]->typeword.jword.addressH << 16) |
			      (code_img[curr_index]->typeword.jword.addressL);
		/* stop */		
		if (curr_opcode == STOP_OP)
			value = (code_img[curr_index]->opcode << 26);
		
		/* print address */
		fprintf(file_desc, "\n%.4d", addr);
		addr +=4;

		for(j=0;j<4;j++){/* print the cmd code - 4 bytes */
			temp = value & mask;
			/* Write a byte value to .ob file */
			fprintf(file_desc, " %.2X", temp);
			value >>= 8;
		}	
	}

	/* Write data image */
	for (cnt=0, curr_index = 0; curr_index < dataIndex ; curr_index++) {
		type = data_img[curr_index].type;
		(type == STRING_INST) ? (bytes = DB_INST) : (bytes = type);
		value = data_img[curr_index].value;
		
		while(bytes ){
			if(!cnt) /* if cnt == 0 then we need to start a new line */
			{
				fprintf(file_desc, "\n%.4d", addr);/* print address */
				addr +=4;
				cnt = 4;
			}
			temp = value & mask;
			fprintf(file_desc, " %.2X", temp);/* print byte */
			value = value >> 8;/* get the next following byte */
			/* we just wrote a byte so lets demote the counters */
			bytes--;
			cnt--;
		}		
	}
	
/* fill last line with zeros if necessary*/
/*	while(cnt--) fprintf(file_desc, " 00");*/	

	/* Close the file */
	fclose(file_desc);
	return TRUE;
}

/**
 * Writes a sybol table to a file. 
 * @param tab A symbol table
 * @param filename The file name
 * @param file_extension The file extension
 */
static bool write_table_to_file(table tab, char *filename, char *file_extension) {
	FILE *file_desc;
	/* concatenate filename & extension, and open the file for writing: */
	char *full_filename = strallocat(filename, file_extension);
	file_desc = fopen(full_filename, "w");
	free(full_filename);
	/* if failed, print error and exit */
	if (file_desc == NULL) {
		printf("Can't create or rewrite to file %s.", full_filename);
		return FALSE;
	}
	/* if table is null, nothing to write */
	if (tab == NULL) return TRUE;

	/* Write first line without \n to avoid extraneous line breaks */
	fprintf(file_desc, "%s %.4ld", tab->key, tab->addr);
	while ((tab = tab->next) != NULL) {
		fprintf(file_desc, "\n%s %.4ld", tab->key, tab->addr);
	}
	fclose(file_desc);
	return TRUE;
}
