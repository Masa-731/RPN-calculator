/*
 * Personal project - A simple reverse polish notation calculator.
 *
 * File name: calc.h
 * Desc.: A header file for the calculator.
 */
#ifndef _CALC_H_
#define _CALC_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_EQ_SIZE 256

typedef enum Error_type {
	INVALID_STACK_TYPE,
	INVALID_OPERATOR,
	INVALID_EXPRESSION,
	DIVIDE_BY_ZERO,
	PARENTHESES_NOT_MATCHED,
	EMPTY_EQUATION,
	SYSTEM_ERROR,
} err_t;

typedef enum Stack_type {
	NUM_STACK,
	OP_STACK,
	CALC_STACK,
} stack_t;

typedef enum Op_priority {
	_num = 1,
	_add = 2,
	_sub = 2,
	_mul = 3,
	_div = 3,
	_mod = 3,
	_lbr = 4,
	_rbr = 4,
} pri_t;

struct Token {
	struct Token *prev;
	struct Token *next;
	int token;
	int isnum; // 0: non-number, otherwise: an integer
	pri_t pri; // priority of token
};

typedef struct Token token_t;

// functions in `util.c` (these don't modify the stacks)
void get_equation(char *prompt, char *input, int size);
int get_string(char *prompt, char *buffer, int size);
void trim(char *input);
void error_handler(err_t err);
int check_int(char *number);
void traverse(char *input);
char *substr_char(char *str, char *substr, char *c);

// functions in `calc.c` (these modify the stacks)
void print_stack(stack_t stack);
void parse(char *ch);
int calculate();
void push_token(token_t *dst_stack_head, token_t *token);
void push_num(int number);
void push_op(char ch);
token_t *pop(token_t *src_stack_head);
token_t *get_stack_top(token_t *src_stack_head);

// global stacks (`head` (first element) in each stack is a dummy)
extern token_t num_stack;
extern token_t op_stack;
extern token_t calc_stack;
extern token_t * const num_stack_head;
extern token_t * const op_stack_head;
extern token_t * const calc_stack_head;

extern int depth;
extern int dbg;

#endif