#ifndef IR_H
#define IR_H

#include "parser.h"

typedef enum OperandType {
    OPERAND_CONST,
    OPERAND_VAR,
    OPERAND_TEMP
} OperandType;

typedef enum IROp {
    IR_COPY,
    IR_ADD,
    IR_SUB,
    IR_MUL,
    IR_DIV,
    IR_MOD,
    IR_NEG,
    IR_COMPL,
    IR_NOT,
    IR_EQ,
    IR_NEQ,
    IR_LT,
    IR_LTE,
    IR_GT,
    IR_GTE,
    IR_AND,
    IR_OR,
    IR_RETURN,
    IR_CALL,
    IR_LABEL,
    IR_JUMP,
    IR_JUMP_ZERO
} IROp;

typedef struct Operand {
    OperandType type;
    union {
        int constant;
        int offset;
        int temp_id;
    } value;
} Operand;

typedef struct IRArg {
    Operand *operand;
    struct IRArg *next;
} IRArg;

typedef struct IRInstruction {
    IROp operation;
    Operand *dest;
    Operand *src1;
    Operand *src2;
    IRArg *args;
    int arg_count;
    char name[64];
    struct IRInstruction *next;
} IRInstruction;

typedef struct IRFunction {
    IRInstruction *head;
    IRInstruction *tail;
    char name[64];
    struct IRFunction *next;
} IRFunction;

typedef struct IRProgram {
    IRFunction *head;
    IRFunction *tail;
} IRProgram;

void lower_code(Program *prog);

#endif