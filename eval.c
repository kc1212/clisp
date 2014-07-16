
#include "eval.h"
#include "math.h"

#include <string.h>

static lval eval_op(lval x, char* op, lval y);

lval eval(mpc_ast_t * ast)
{
	if (!ast)
		return lval_err(LERR_OTHER);

	if (strstr(ast->tag, "number"))
	{
		errno = 0;
		long x = strtol(ast->contents, NULL, 10);
		return errno != ERANGE ? lval_num(x) : lval_err(LERR_BAD_NUM);
	}

	char* op = ast->children[1]->contents;
	lval x = eval(ast->children[2]);

	int i = 3;
	while (strstr(ast->children[i]->tag, "expr"))
	{
		x = eval_op(x, op, eval(ast->children[i]));
		i++;
	}

	return x;
}

static lval eval_op(lval x, char* op, lval y)
{
	if (x.type == LVAL_ERR) return x;
	if (y.type == LVAL_ERR) return y;

	// TODO check for overflow?
	if (!strcmp(op, "+")) return lval_num(x.num + y.num);
	if (!strcmp(op, "-")) return lval_num(x.num - y.num);
	if (!strcmp(op, "*")) return lval_num(x.num * y.num);

	if (!strcmp(op, "/"))
	{
		return (0 == y.num) ? lval_err(LERR_DIV_ZERO) : lval_num(x.num / y.num);
	}

	if (!strcmp(op, "%")) return lval_num(x.num % y.num);
	if (!strcmp(op, "^")) return lval_num((long)pow(x.num, y.num));
	if (!strcmp(op, "min")) return lval_num(MIN(x.num, y.num));
	if (!strcmp(op, "max")) return lval_num(MAX(x.num, y.num));

	return lval_err(LERR_BAD_OP);
}


