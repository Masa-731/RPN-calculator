/*
 * Personal project - A simple reverse polish notation calculator.
 *
 * File name: calc.c
 * Desc.: define functions which modify the stacks for calclation.
 */

#include "calc.h"


/*
 * `print_stack()` function
 *
 * Desc.: print the stack specified by `stack_type` (defined in `calc.h`).
 * Parameters: stack type
 * Return value: none
 */
void print_stack(stack_t stack_type)
{
	token_t *token_ptr;
	
	switch (stack_type) {
	case NUM_STACK:
		printf("num_stack: [");
		token_ptr = num_stack_head;
		break;
	case OP_STACK:
		printf("op_stack: [");
		token_ptr = op_stack_head;
		break;
	case CALC_STACK:
		printf("calc_stack: [");
		token_ptr = calc_stack_head;
		break;
	default:
		error_handler(INVALID_STACK_TYPE);
	}
	
	while (token_ptr->next != NULL) {
		token_ptr = token_ptr->next;
		if (token_ptr->isnum) {
			printf("%d,", token_ptr->token);
		} else {
			printf("%c,", token_ptr->token);
		}
	}
	printf("]\n");
	
	return;
}


/*
 * `parse()` function
 *
 * Desc.: parse the input string and create RPN tree.
 * Parameters: input string
 * Return value: none
 */
void parse(char *ch)
{
	int c = 0;
	token_t *op_stack_ptr;
	while(*ch != '\0') {
		if (c = check_int(ch)) {
			push_num(atoi(ch));
			ch += c;
		} else {
			push_op(*ch);
			ch++; 
		}
	}
	
	// pop all the remaining operators onto `num_stack`
	op_stack_ptr = get_stack_top(op_stack_head);
	while (op_stack_ptr != op_stack_head) {
		push_token(num_stack_head, pop(op_stack_head));
		op_stack_ptr = get_stack_top(op_stack_head);
	}
	
	return;
}


/*
 * `calculate()` function
 *
 * Desc.: calculate the equation in RPN notation.
 * Parameters: none
 * Return value: answer of the input equation
 */
int calculate()
{
	token_t *token;
	token_t *op_l, *op_r;
	token_t *stack_ptr;
	// pop every token in `num_stack` and push it into `calc_stack`
	stack_ptr = get_stack_top(num_stack_head);
	while (stack_ptr != num_stack_head) {
		push_token(calc_stack_head, pop(num_stack_head));
		stack_ptr = get_stack_top(num_stack_head);
	}
	
	// for debugging
	if (dbg) print_stack(CALC_STACK);
	
	// reset `num_stack`
	while (stack_ptr != num_stack_head) {
		free(pop(num_stack_head));
		stack_ptr = get_stack_top(num_stack_head);
	}
	
	stack_ptr = get_stack_top(calc_stack_head);
	while (stack_ptr != calc_stack_head) {
		token = pop(calc_stack_head);
		if (dbg) {
			print_stack(NUM_STACK);
			putchar('\n');
			print_stack(CALC_STACK);
		}
		if (token->isnum) {
			push_token(num_stack_head, token);
		} else {
			op_r = pop(num_stack_head);
			op_l = pop(num_stack_head);
			
			switch (token->token) {
			case '+':
				push_num(op_l->token + op_r->token);
				break;
			case '-':
				push_num(op_l->token - op_r->token);
				break;
			case '*':
				push_num(op_l->token * op_r->token);
				break;
			case '/':
				if (op_r->token) {
					push_num(op_l->token / op_r->token);
					break;
				} else {
					error_handler(DIVIDE_BY_ZERO);
				}
			case '%':
				if (op_r->token) {
					push_num(op_l->token % op_r->token);
					break;
				} else {
					error_handler(DIVIDE_BY_ZERO);
				}
			}
			
			free(op_r);
			free(op_l);
			free(token);
		}
		stack_ptr = get_stack_top(calc_stack_head);
	}
	
	// if `stack_ptr` doesn't hit the bottom of the stacks
	stack_ptr = get_stack_top(num_stack_head);
	if (stack_ptr->prev != num_stack_head) {
		error_handler(INVALID_EXPRESSION);
	}
	
	if (dbg) {
		print_stack(NUM_STACK);
		putchar('\n');
	}
	
	return stack_ptr->token;
}


/*
 * `push_token()` function
 *
 * Desc.: receive a token object and push it into the specified stack.
 * Parameters: pointer to the destination stack, pointer to the token to push
 * Return value: none
 */
void push_token(token_t *dst_stack_head, token_t *token)
{
	token_t *stack_ptr = get_stack_top(dst_stack_head);
	
	token->prev = stack_ptr;
	stack_ptr->next = token;
	
	return;
}


/*
 * `push_num()` function
 *
 * Desc.: create a token with a numerical number and push it into `num_stack`.
 * Parameters: number value
 * Return value: none
 */
void push_num(int number)
{
	token_t *stack_ptr;
	token_t *num_token;
	
	stack_ptr = get_stack_top(num_stack_head);
	
	num_token = malloc(sizeof(token_t));
	num_token->next = NULL;
	num_token->prev = NULL;
	num_token->token = number;
	num_token->isnum = 1;
	num_token->pri = _num;
	
	push_token(num_stack_head, num_token);
	
	return;
}


/*
 * `push_op()` function
 *
 * Desc.: creates a token with an operator and push it into `op_stack`.
 * Parameters: operator character
 * Return value: none
 */
void push_op(char ch)
{
	token_t *num_stack_ptr;
	token_t *op_stack_ptr;
	token_t *op_token;
	pri_t pri_arg;
	switch (ch) {
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
	
	op_stack_ptr = get_stack_top(op_stack_head);
	
	if (ch != '(' || ch != ')') {
		// if the stack isn't empty and the scanned operator has higher priority except for parentheses ('*', '/', or '%')
		if (op_stack_ptr != op_stack_head && pri_arg <= op_stack_ptr->pri && op_stack_ptr->token != '(' && op_stack_ptr->token != ')') {
			// pop all the elements in `op_stack` onto `num_stack`
			while (op_stack_ptr != op_stack_head && op_stack_ptr->token != '(' && op_stack_ptr->token != ')' && pri_arg <= op_stack_ptr->pri) {
				push_token(num_stack_head, pop(op_stack_head));
				op_stack_ptr = get_stack_top(op_stack_head);
			}
			goto L_push_op;
		}
	}
	
	// if the scanned character is a parenthesis
	if (ch == '(') {
		if (dbg) printf("A left bracket detected.\n");
		depth++;
		goto L_push_op;
	}
	if (ch == ')') {
		if (dbg) printf("Falling down the op_stack...\n");
		// pop all the elements in `op_stack` onto `num_stack`
		while(op_stack_ptr != op_stack_head) {
			if (op_stack_ptr->token == '(') {
				// discard a left parenthesis
				free(pop(op_stack_head));
				break;
			} else {
				if (dbg) printf("A right braket detected, poping...\n");
				push_token(num_stack_head, pop(op_stack_head));
			}
			op_stack_ptr = get_stack_top(op_stack_head);
		}
		depth--;
		return;
	}
	
L_push_op:
	op_token = malloc(sizeof(token_t));
	op_token->prev = NULL;
	op_token->next = NULL;
	op_token->token = ch;
	op_token->isnum = 0;
	op_token->pri = pri_arg;
	
	push_token(op_stack_head, op_token);
	
	return;
}


/*
 * `pop()` function
 *
 * Desc.: pop a token from the specified stack and return a pointer to the popped token.
 * Parameters: pointer to the source stack
 * Return value: pointer to the popped token
 */
token_t *pop(token_t *src_stack_head)
{
	token_t *stack_ptr;
	stack_ptr = get_stack_top(src_stack_head);
	if (stack_ptr == src_stack_head) {
		error_handler(INVALID_EXPRESSION);
	}
	stack_ptr->prev->next = NULL;
	stack_ptr->prev = NULL;
	
	return stack_ptr;
}


/*
 * `get_stack_top()` function
 *
 * Desc.: return a pointer to a top of the specified stack.
 * Parameters: pointer to the source stack
 * Return value: pointer to the top of the stack
 */
token_t *get_stack_top(token_t *src_stack_head)
{
	token_t *stack_ptr = src_stack_head;
	while (stack_ptr->next != NULL) {
		stack_ptr = stack_ptr->next;
	}
	
	return stack_ptr;
}

