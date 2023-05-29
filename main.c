/*
 * Personal project - A simple reverse polish notation calculator.
 *
 * File name: main.c
 * Desc.: declare main function and global variables.
 */
#include "calc.h"

// global stacks (`head` (first element) in each stack is a dummy and shouldn't be accessed)
// those stacks are of dual-linked lists
token_t num_stack = {
	NULL,	// *prev
	NULL,	// *next
	0,		// token
	0,		// isnum
	0,		// pri
};

token_t op_stack = {
	NULL,	// *prev
	NULL,	// *next
	0,		// token
	0,		// isnum
	0,		// pri
};

token_t calc_stack = {
	NULL,	// *prev
	NULL,	// *next
	0,		// token
	0,		// isnum
	0,		// pri
};

// pointers to the head of each stacks
token_t * const num_stack_head = &num_stack;
token_t * const op_stack_head = &op_stack;
token_t * const calc_stack_head = &calc_stack;

// depth of the nests
int depth = 0;

// debugging flag
int dbg = 0;


/*
 * main function
 *
 * Parameters: command line arguments
 * Return value: 0 on normal (program might end in other functions via `error_handler()`)
 */
int main(int argc, char **argv)
{
	token_t *token;
	char input[MAX_EQ_SIZE];
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
	
	// enter an input equation
	get_equation("Input=>", input, MAX_EQ_SIZE);
	
	// traverse the string and do necessary modifications
	// e.g. unary '-a' to '(0-a)', '(a)(b)' to '(a)*(b)', 'a(b)' to 'a * (b)'
	// currently no specific priority is set for parenthesis multiplication (just treat as a normal '*')
	if (dbg) printf("\nTraversing input: ");
	traverse(input);
	if (dbg) printf("%s\n\n", input);
	
	// parse the input and convert it to RPN and push onto `num_stack`
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
	
	if (dbg) printf("Calculating RPN...\n");
	ans = calculate();
	
	printf("Answer: %d\n", ans);
	
	// cleanup (other token objects are already freed)
	free(num_stack_head->next);
	
	return 0;
}

