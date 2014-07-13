
#include "eval.h"
#include "math.h"
#include "common.h"

#include <string.h>

int eval(mpc_ast_t * ast)
{
	if (!ast)
		return -1; // need better return code

	if (strstr(ast->tag, "number"))
		return atoi(ast->contents);

	char* op = ast->children[1]->contents;
	long x = eval(ast->children[2]);

	int i = 3;
	while (strstr(ast->children[i]->tag, "expr"))
	{
		x = eval_op(x, op, eval(ast->children[i]));
		i++;
	}

	return x;
}

long eval_op(long x, char* op, long y)
{
	if (!strcmp(op, "+")) return x + y;
	if (!strcmp(op, "-")) return x - y;
	if (!strcmp(op, "*")) return x * y;
	if (!strcmp(op, "/")) return x / y;
	if (!strcmp(op, "%")) return x % y;
	if (!strcmp(op, "^")) return pow(x,y);
	if (!strcmp(op, "min")) return MIN(x,y);
	if (!strcmp(op, "max")) return MAX(x,y);
	return 0;
}


