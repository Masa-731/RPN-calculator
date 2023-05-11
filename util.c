/*
 * Personal project - A simple reverse polish notation calculator.
 *
 * File name: util.c
 * Desc.: define functions which don't modify the stacks.
 */
#include "calc.h"


/*
 * `get_equation()` function
 *
 * Desc.: call `get_string()` to get input equation and trim whitespaces.
 * Parameters: prompt string, input buffer, size of input buffer
 * Return value: none
 */
void get_equation(char *prompt, char *input, int size)
{
	int ret;
	
	ret = get_string(prompt, input, size);
	if (ret) {
		error_handler(SYSTEM_ERROR);
	}
	if (input[0] == '\n') {
		error_handler(EMPTY_EQUATION);
	}
	
	trim(input);
	
	return;
}


/*
 * `get_string()` function
 *
 * Desc.: get a user input string from `stdin`.
 * Parameters: prompt string, input buffer, size of input buffer
 * Return value: 0 on normal, -1 on error
 */
int get_string(char *prompt, char *buffer, int size)
{
	int len;
	char ch;
	
	if (buffer == NULL || size < 2) {
		return -1;
	}
	
	while (1) {
		printf("%s ", prompt);
	
		if (fgets(buffer, size, stdin) == NULL) {
			return -1;
		}
		
		len = strlen(buffer);
		
		if (buffer[len - 1] == '\n') {
			buffer[len - 1] = '\0';
			break;
		} else {
			while ((ch = getchar()) != EOF && ch != '\n') {
				continue;
			}
			fprintf(stderr, "Error: buffer overflow, max character count is %d!\n\n", MAX_EQ_SIZE - 2);
		}
	}
	
	return 0;
}


/*
 * `trim()` function
 *
 * Desc.: trim any whitespace in the string.
 * Parameters: input string
 * Return value: none
 */
void trim(char *input)
{
	int i;
	char *loc;
	
	i = 0;
	while (input[i] != '\0') {
		loc = input + i;
			if (isspace(*loc)) {
				memmove(loc, loc + 1, strlen(loc + 1) + 1);
			}
		i++;
	}
}


/*
 * `error_handler()` function
 *
 * Desc.: print out various types of errors and exit the program with `EXIT_FAILURE`.
 * Parameters: error type (defined in `calc.h`)
 * Return value: none
 */
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
	case EMPTY_EQUATION:
		fprintf(stderr, "Error: input is empty\n");
	default:
		fprintf(stderr, "Error: something went wrong\n");
		break;
	}
	
	exit(EXIT_FAILURE);
}


/*
 * `check_int()` function
 *
 * Desc.: scan the input string and find whether the string is a numeric value.
 * Parameters: input string
 * Return value: length of the value (as a string)
 */
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


/*
 * `traverse()` function
 *
 * Desc.: scan the input string and modify it.
 * Parameters: input string
 * Return value: none
 */
void traverse(char *input)
{
	char *found;
	found = input;
	while (found != NULL) {
		found = substr_char(found, ")(", "*");
	}
	
	found = input;
	while (found != NULL) {
		found = substr_char(found, "-", "(0");
	}
	return;
}


/*
 * `subst_char()` function
 *
 * Desc.: return a pointer to the character following substitution, or NULL.
 * Parameters: pointer to a location of a string, pattern string, string to substitute with
 * Return value: pointer to a location of the input string, at right after the substitution
 */
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

