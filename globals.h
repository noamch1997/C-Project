/* global constants */

#ifndef _GLOBALS_H
#define _GLOBALS_H

#define MAX_INT32bit 2147483646+1 
#define MIN_INT32bit -2147483647-1

/** Boolean definition */
typedef enum booleans {
	FALSE = 0, TRUE = 1
} bool;

/** Maximum size of code image and data image */
#define CODE_ARR_IMG_LENGTH 1200

/** Maximum length of a single source line  */
#define MAX_LINE_LENGTH 80


/** Initial IC value */
#define IC_INIT_VALUE 100

#define ANSI_COLOR_RED     "\x1b[1;31m"
#define ANSI_COLOR_GRAY "\x1b[1;32m"
#define ANSI_COLOR_RESET   "\x1b[0;0m"


/* Commands opcode */
typedef enum opcodes {
	/* TYPE OF R */
	ADD_OP = 0,   /* 3 registers */
	SUB_OP = 0,   /* 3 */
	AND_OP = 0,   /* 3 */
	OR_OP  = 0,   /* 3 */
	NOR_OP = 0,   /* 3 */
	
	MOVE_OP = 1,	/* 2 operands both registers */
	MVHI_OP = 1,	/* 2 operands both registers */
	MVLO_OP = 1,	/* 2 operands both registers */

	/* TYPE OF I */
	ADDI_OP = 10,	/* 3 operands registers assebly cmd: cmd rs,imm,rd 														--> assembly code: cmd rs,rt,imm */
	SUBI_OP = 11,	/* 3 operands */
	ANDI_OP = 12,	/* 3 operands */
	ORI_OP  = 13,	/* 3 operands */
	NORI_OP = 14,	/* 3 operands */
	
	BNE_OP = 15,	/* 3 registers assebly cmd: cmd rs,rt,label --> 
													assembly code: cmd rs,rt,imm */
	BEQ_OP = 16,	/* 3 operands */
	BLT_OP = 17,	/* 3 operands */
	BGT_OP = 18,	/* 3 operands */
	
	LB_OP = 19,		/* 3 operands registers assebly cmd: cmd rs,imm,rd --> 
													assembly code: cmd rs,rt,(se)imm */
	SB_OP = 20,		/* 3 operands */
	LW_OP = 21,		/* 3 operands */
	SW_OP = 22,		/* 3 operands */
	LH_OP = 23,		/* 3 operands */
	SH_OP = 24,		/* 3 operands */

	/* others */
	JMP_OP = 30,	/* 1 operands register/label */
	LA_OP = 31,		/* 1 register-$0 assebly cmd: la label --> 
													assembly code: la [$0=label] */
	CALL_OP = 32,	/* 1 register-$0 assebly cmd: call label --> 
													assembly code: cmd [$0=label] */
	STOP_OP = 63,	/* 0 registers */

	/** Failed/Error */
	NONE_OP = -1
} opcode;

/* Operand addressing type */
typedef enum addressing_types {
	IMMEDIATE_ADDR = 0,
	DIRECT_ADDR = 1,
	REGISTER_ADDR = 3,
	/** Failed/Not detected addressing */
	NONE_ADDR = -1
} addressing_type;

/** Commands funct */
typedef enum funct {
	ADD_FUNCT  = 1,
	SUB_FUNCT  = 2,
	AND_FUNCT  = 3,
	OR_FUNCT  = 4,
	NOR_FUNCT  = 5,
	MOVE_FUNCT = 1,
	MVHI_FUNCT = 2,
	MVLO_FUNCT = 3,

	/** Default TYPE I/J OR ERROR */
	NONE_FUNCT = 0
} funct;

/** Registers - r0->r1 + not found */
typedef enum registers {
	R0 = 0,
	R1,
	R2,
	R3,
	R4,
	R5,
	R6,
	R7,
	R8,
	R9,
	R10,
	R11,
	R12,
	R13,
	R14,
	R15,
	R16,
	R17,
	R18,
	R19,
	R20,
	R21,
	R22,
	R23,
	R24,
	R25,
	R26,
	R27,
	R28,
	R29,
	R30,
	R31,
	NONE_REG = -1
} reg;




typedef struct r_word {
	unsigned int unused :6;
	unsigned int funct :5;
	unsigned int RD :5;
	unsigned int RT :5;
	unsigned int RS :5;

} r_word;

typedef struct i_word {
	unsigned int immed :16;
	unsigned int RT :5;
	unsigned int RS :5;

} i_word;

typedef struct j_word {
	/* aderress demends 25 bits so lets seperate it to 16,9 (16+9=25) */
	unsigned int addressL :16;
	unsigned int addressH :9;
	unsigned int reg :1;

} j_word;

/** data type (.db .dh .dw) */
typedef enum cmd_type {
	/** .db .dh .dw instruction */
	R_CMD = 1,
	/** .extern instruction */
	I_CMD = 2,
	/** .entry instruction */
	J_CMD = 4,
	NONE_CMD = -1
} cmd_type;


/** Represents a single code word */
typedef struct code_word {
	opcode opcode;
	unsigned long ic;
	cmd_type type;
	union typeword {
		r_word rword;
		i_word iword;
		j_word jword;
		
	} typeword;

} code_word;



/** Instruction type (.data, .entry, etc.) */
typedef enum instruction {
	/** data instruction: .db .dh .dw */
	DB_INST=1,
	DH_INST=2,
	DW_INST=4,
	/*DATA_INST,*/
	/** .extern instruction */
	EXTERN_INST,
	/** .entry instruction */
	ENTRY_INST,
	/** .asciz instruction */
	STRING_INST,
	/** Not found */
	NONE_INST,
	/** Parsing/syntax error */
	ERROR_INST
} instruction;


/** Represents a single data word. */
typedef struct data_word {
	unsigned long dc;
	instruction type;
	unsigned long value;
} data_word;


/**
 * Represents a single source line
 */
typedef struct line_info {
	/** Line number in file */
	long line_number;
	/** File name */
	char *file_name;
	/** Line content (source) */
	char *content;
} line_info;

#endif
