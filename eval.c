
#include "eval.h"
#include "math.h"

#include <string.h>
#include <float.h>

// static lval eval_op(lval x, char* op, lval y);
static lval* read_long(mpc_ast_t* ast);
static lval* read_double(mpc_ast_t* ast);

lval* lval_read(mpc_ast_t* ast)
{
	if (!ast) { return lval_err(LERR_OTHER); }

	if (strstr(ast->tag, "long")) { return read_long(ast); }
	else if (strstr(ast->tag, "double")) { return read_double(ast); }
	else if (strstr(ast->tag, "symbol")) { return lval_sym(ast->contents); }

	lval* x = NULL;   // ">" is root
	if (strcmp(ast->tag, ">") == 0 || strstr(ast->tag, "sexpr"))
	{
		x = lval_sexpr();
	}

	for (int i = 0; i < ast->children_num; i++)
	{
		if (strcmp(ast->children[i]->contents, "(") == 0
			|| strcmp(ast->children[i]->contents, ")") == 0
			|| strcmp(ast->children[i]->contents, "}") == 0
			|| strcmp(ast->children[i]->contents, "{") == 0
			|| strcmp(ast->children[i]->tag,  "regex") == 0
			) { continue; }
		x = lval_add(x, lval_read(ast->children[i]));
	}

	// char* op = ast->children[1]->contents;
	// lval x = eval(ast->children[2]);

	// int i = 3;
	// while (strstr(ast->children[i]->tag, "expr"))
	// {
	// 	x = eval_op(x, op, eval(ast->children[i]));
	// 	i++;
	// }

	return x;
}

// private functions: //////////////////////////////////////////////////////////

static lval* read_long(mpc_ast_t* ast)
{
	errno = 0;
	int64_t x = strtol(ast->contents, NULL, 10);
	if (errno)
	{
		log_err("strtol conversion failed for %s", ast->contents);
		return lval_err(LERR_BAD_NUM);
	}
	return lval_long(x);
}

static lval* read_double(mpc_ast_t* ast)
{
	errno = 0;
	double x = strtod(ast->contents, NULL);
	if (errno)
	{
		log_err("strtod conversion failed for %s", ast->contents);
		return lval_err(LERR_BAD_NUM);
	}
	return lval_double(x);
}

// static lval eval_op(lval x, char* op, lval y)
// {
// 	if (LVAL_ERR == x.type) return x;
// 	if (LVAL_ERR == y.type) return y;
// 
// 	// TODO check for overflow
// 	if (x.type == LVAL_DBL || y.type == LVAL_DBL)
// 	{
// 		if (!strcmp(op, "+")) return lval_double(GET_LVAL_DATA(x) + GET_LVAL_DATA(y));
// 		if (!strcmp(op, "-")) return lval_double(GET_LVAL_DATA(x) - GET_LVAL_DATA(y));
// 		if (!strcmp(op, "*")) return lval_double(GET_LVAL_DATA(x) * GET_LVAL_DATA(y));
// 
// 		if (!strcmp(op, "/"))
// 		{
// 			return (DBL_EPSILON > GET_LVAL_DATA(y)) ?
// 				lval_err(LERR_DIV_ZERO) : lval_double(GET_LVAL_DATA(x) / GET_LVAL_DATA(y));
// 		}
// 
// 		if (!strcmp(op, "^")) return lval_double(pow(GET_LVAL_DATA(x), GET_LVAL_DATA(y)));
// 		if (!strcmp(op, "min")) return lval_double(MIN(GET_LVAL_DATA(x), GET_LVAL_DATA(y)));
// 		if (!strcmp(op, "max")) return lval_double(MAX(GET_LVAL_DATA(x), GET_LVAL_DATA(y)));
// 	}
// 	else
// 	{
// 		if (!strcmp(op, "+")) return lval_long(x.data.lng + y.data.lng);
// 		if (!strcmp(op, "-")) return lval_long(x.data.lng - y.data.lng);
// 		if (!strcmp(op, "*")) return lval_long(x.data.lng * y.data.lng);
// 
// 		if (!strcmp(op, "/"))
// 		{
// 			return (0 == y.data.lng) ? lval_err(LERR_DIV_ZERO) : lval_long(x.data.lng / y.data.lng);
// 		}
// 
// 		if (!strcmp(op, "%")) return lval_long(x.data.lng % y.data.lng);
// 		if (!strcmp(op, "^")) return lval_long((long)pow(x.data.lng, y.data.lng));
// 		if (!strcmp(op, "min")) return lval_long(MIN(x.data.lng, y.data.lng));
// 		if (!strcmp(op, "max")) return lval_long(MAX(x.data.lng, y.data.lng));
// 	}
// 
// 	return lval_err(LERR_BAD_OP);
// }


