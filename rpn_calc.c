/*
 * Simple reverse polish notation calculator.
 */
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#define STACK_SIZE 512

typedef enum Error_type {
	INVALID_STACK_TYPE,
	INVALID_OPERATOR,
	INVALID_EXPRESSION,
	DIVIDE_BY_ZERO,
	PARENTHESES_NOT_MATCHED,
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

typedef struct Token {
	int token;
	int isnum; // 0: non-number, otherwise: an integer
	pri_t pri; // priority of token
} token_t;

void error_handler(err_t err);
void print_stack(stack_t stack);
void push_num(int a);
void push_op(char c);
token_t pop(token_t **p);

void traverse(char *p);
char *substr_char(char *str, char *substr, char *c);
void parse(char *p);

int calculate();

// global stacks (`head` (first element) in each stack is null)
token_t num_stack[STACK_SIZE], *num_p = num_stack;
token_t op_stack[STACK_SIZE], *op_p = op_stack;
token_t calc_stack[STACK_SIZE], *calc_p = calc_stack;

int depth = 0;
int dbg = 0;

int main(int argc, char **argv)
{
	token_t *token;
	char input[STACK_SIZE], *p = input;
	int ans;
	int i;
	
	if (argc > 1) {
		if (!strcmp(argv[1], "-debug")) {
			printf("debug info enabled.\n");
			dbg = 1;
		}
		if (!strcmp(argv[1], "-help")) {
			printf("use '-debug' for debug info.\n");
			return 0;
		}
	}
	
	printf("Input: ");
	scanf("%s", input);
	
	// traverse the string and do necessary modifications
	// e.g. unary '-a' to '(0-a)', '(a)(b)' to '(a)*(b)'
	// currently no specific priority is set for parenthesis multiplication (just treat as a normal '*')
	if (dbg) printf("\nTraversing input: ");
	traverse(input);
	if (dbg) printf("%s\n\n", input);
	
	// parse the input and convert it to RPN onto `num_stack`
	if (dbg) printf("Parsing the input...\n\n");
	parse(input);
	
	// print `num_stack` for debugging
	if (dbg) {
		printf("\nConverted input into RPN\n");
		print_stack(NUM_STACK);
		putchar('\n');
	}
	
	if (depth != 0) {
		error_handler(PARENTHESES_NOT_MATCHED);
	}
	
	// (unnecessary) reset `op_stack`
	// memset(op_stack, 0, sizeof(op_stack));
	// op_p = op_stack;
	// print_stack(OP_STACK);
	
	if (dbg) printf("Calculating RPN...\n");
	ans = calculate();
	
	printf("Answer: %d\n", ans);
	
	return 0;
}

void error_handler(err_t err)
{
	switch(err) {
	case INVALID_STACK_TYPE:
		fprintf(stderr, "Error: invalid stack type\n");
		break;
	case INVALID_OPERATOR:
		fprintf(stderr, "Error: invalid operator\n");
		break;
	case INVALID_EXPRESSION:
		fprintf(stderr, "Error: invalid expression\n");
		break;
	case DIVIDE_BY_ZERO:
		fprintf(stderr, "Error: divide by zero\n");
		break;
	case PARENTHESES_NOT_MATCHED:
		fprintf(stderr, "Error: parentheses not matched\n");
		break;
	}
	
	exit(EXIT_FAILURE);
}

void print_stack(stack_t stuck)
{
	token_t *token;
	int i, diff;
	
	switch (stuck) {
	case NUM_STACK:
		printf("num_stack: [");
		token = num_stack;
		diff = num_p - num_stack;
		break;
	case OP_STACK:
		printf("op_stack: [");
		token = op_stack;
		diff = op_p - op_stack;
		break;
	case CALC_STACK:
		printf("calc_stack: [");
		token = calc_stack;
		diff = calc_p - calc_stack;
		break;
	default:
		error_handler(INVALID_STACK_TYPE);
	}
	
	for (i = 0; i <= diff; i++, token++) {
		if (token->isnum) {
			printf("%d,", token->token);
		} else {
			printf("%c,", token->token);
		}
	}
	printf("]\n");
	
	return;
}

// scan the input string and modify it
void traverse(char *p)
{
	char *found;
	found = p;
	while (found != NULL) {
		found = substr_char(found, ")(", "*");
	}
	
	found = p;
	while (found != NULL) {
		found = substr_char(found, "-", "(0");
	}
	return;
}

// returns a pointer to the character following substitution, or NULL
char *substr_char(char *st, char *subst, char *c)
{
	int offset = 0;
	char *loc = strstr(st, subst);
	char *prev = NULL;
	int nbytes = strlen(c);
	
	// kind of unsafe?; no strict boundary checking
	if (loc - 1 != NULL) {
		prev = loc - 1;
	}

	switch (*subst) {
	case '-':
		if (loc) {
			// when the condition is met, treat '-' as unary and do the replacement
			if (*prev !=  ')' && !isdigit(*prev)) {
				memmove(loc + nbytes, loc, strlen(loc) + 1);
				strncpy(loc, c, nbytes);
				loc += nbytes + 1;
				offset = check_int(loc);
				loc += offset;
				memmove(loc + 1, loc, strlen(loc) + 1);
				*loc = ')';
				loc++;
			} else {
				loc++;
			}
		}
		break;
	case ')':
		if (loc) {
			// put '*' between a pair of brackets
			loc++;
			memmove(loc + 1, loc, strlen(loc) + 1);
			*loc = *c;
			loc++;
		}
		break;
	}
	
	return loc;
}

void parse(char *p)
{
	int c = 0;
	while(*p != '\0') {
		if (c = check_int(p)) {
			push_num(atoi(p));
			p += c;
		} else {
			push_op(*p);
			p++; 
		}
	}
	
	// pop all the remaining operators onto `num_stack`
	while (op_p > op_stack) {
		*(++num_p) = pop(&op_p);
	}
	
	return;
}

int check_int(char *number)
{
	int count = 0;
	for (; *number; number++) {
		if (isdigit(*number)) {
			count++;
		} else {
			return count;
		}
	}
	
	return count;
}

void push_num(int a)
{
	(++num_p)->token = a;
	num_p->isnum = 1;
	num_p->pri = _num;
	
	return;
}

void push_op(char c)
{
	token_t *tmp;
	pri_t pri_arg;
	switch (c) {
	case '+':
		pri_arg = _add;
		break;
	case '-':
		pri_arg = _sub;
		break;
	case '*':
		pri_arg = _mul;
		break;
	case '/':
		pri_arg = _div;
		break;
	case '%':
		pri_arg = _mod;
	case '(':
		pri_arg = _lbr;
		break;
	case ')':
		pri_arg = _rbr;
		break;
	default:
		error_handler(INVALID_OPERATOR);
	}
	
	if (c != '(' || c != ')') {
		// if the stack isn't empty and the scanned operator has higher priority except for parentheses ('*', '/', or '%')
		if (op_p > op_stack && pri_arg <= op_p->pri && op_p->token != '(' && op_p->token != ')') {
			// pop all the elements in `op_stack` onto `num_stack`
			while (op_p > op_stack && op_p->token != '(' && op_p->token != ')') {
				*(++num_p) = pop(&op_p);
			}
			goto L_push_op;
		}
	}
	
	// if the scanned character is a parenthesis
	if (c == '(') {
		if (dbg) printf("A left bracket detected.\n");
		depth++;
		goto L_push_op;
	}
	if (c == ')') {
		if (dbg) printf("Falling down the op_stack...\n");
		// pop all the elements in `op_stack` onto `num_stack`
		while(op_p > op_stack) {
			if (op_p->token == '(') {
				// discard a left parenthesis
				pop(&op_p);
				break;
			} else {
				if (dbg) printf("A right braket detected, poping...\n");
				*(++num_p) = pop(&op_p);
			}
		}
		depth--;
		return;
	}
	
L_push_op:
	(++op_p)->token = c;
	op_p->isnum = 0;
	op_p->pri = pri_arg;
	
	return;
}

token_t pop(token_t **p)
{
	token_t ret = **p;
	(*p)--;
	return ret;
}

int calculate()
{
	token_t token;
	int a, b;
	int i;
	// pop every token in `num_stack` and push it into `calc_stack`
	while (num_p > num_stack) {
		*(++calc_p) = pop(&num_p);
	}
	
	// for debugging
	if (dbg) print_stack(CALC_STACK);
	
	// reset `num_stack`
	memset(num_stack, 0, sizeof(num_stack));
	num_p = num_stack;
	
	while (calc_p > calc_stack) {
		token = pop(&calc_p);
		if (dbg) {
			print_stack(NUM_STACK);
			putchar('\n');
			print_stack(CALC_STACK);
		}
		if (token.isnum) {
			push_num(token.token);
		} else {
			b = pop(&num_p).token;
			a = pop(&num_p).token;
			
			switch (token.token) {
			case '+':
				push_num(a + b);
				break;
			case '-':
				push_num(a - b);
				break;
			case '*':
				push_num(a * b);
				break;
			case '/':
				if (b) {
					push_num(a / b);
					break;
				} else {
					error_handler(DIVIDE_BY_ZERO);
				}
			case '%':
				if (b) {
					push_num(a % b);
					break;
				} else {
					error_handler(DIVIDE_BY_ZERO);
				}
			}
		}
	}
	
	// if `num_p` doesn't hit the bottom of the stack
	if (num_p != (num_stack + 1)) {
		error_handler(INVALID_EXPRESSION);
	}
	
	if (dbg) {
		print_stack(NUM_STACK);
		putchar('\n');
	}
	
	return num_p->token;
}
