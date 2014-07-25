
#include "eval.h"
#include "math.h"

#include <string.h>
#include <float.h>

// static lval eval_op(lval x, char* op, lval y);
static lval* _read_long(mpc_ast_t* ast);
static lval* _read_double(mpc_ast_t* ast);
static lval* _lval_pop(lval* v, int i);
static lval* _lval_take(lval* v, int i);

lval* lval_eval_sexpr(lval* v)
{
	// eval children
	for (int i = 0; i < v->count; i++)
	{
		v->cell[i] = lval_eval(v->cell[i]);
	}

	for (int i = 0; i < v->count; i++)
	{
		if (v->cell[i]->type == LVAL_ERR) { return _lval_take(v, i); }
	}

	if (v->count == 0) { return v; }
	if (v->count == 1) { return _lval_take(v, 0); }

	// take the first element and make sure it's a symbol (op)
	lval* f = _lval_pop(v, 0);
	if (f->type != LVAL_SYM)
	{
		lval_del(f); lval_del(v);
		return lval_err(LERR_BAD_SEXPR_START);
	}

	lval* result = builtin_op(v, f->sym);
	lval_del(f);
	return result;
}

lval* lval_eval(lval* v)
{
	if (v->type == LVAL_SEXPR) { return lval_eval_sexpr(v); }
	return v; // return same v if not sexpr
}

lval* builtin_op(lval* v, char* op)
{
	for (int i = 0; i < v->count; i++) // ensure all children are numbers
	{
		if (v->cell[i]->type != LVAL_LNG && v->cell[i]->type != LVAL_DBL)
		{
			lval_del(v);
			return lval_err(LERR_BAD_NUM);
		}
	}

	lval* x = _lval_pop(v, 0);
	if ((strcmp(op, "-") == 0) && v->count == 0)
	{
		if (LVAL_DBL == x->type) { x->data.dbl = -x->data.dbl; }
		if (LVAL_LNG == x->type) { x->data.lng = -x->data.lng; }
	}

	while (v->count > 0)
	{
		lval* y = _lval_pop(v, 0);

		if (x->type == LVAL_DBL || y->type == LVAL_DBL)
		{
			TO_LVAL_DBL(x); TO_LVAL_DBL(y);
			if (!strcmp(op, "+")) { x->data.dbl += y->data.dbl; }
			if (!strcmp(op, "-")) { x->data.dbl -= y->data.dbl; }
			if (!strcmp(op, "*")) { x->data.dbl *= y->data.dbl; }

			if (!strcmp(op, "/"))
			{
				if (DBL_EPSILON > y->data.dbl)
				{
					lval_del(x); lval_del(y); // v is deleted after while
					x = lval_err(LERR_DIV_ZERO);
					break;
				}
				x->data.dbl /= y->data.dbl;
			}

			if (!strcmp(op, "^")) { x->data.dbl = pow(x->data.dbl, y->data.dbl); }
			if (!strcmp(op, "min")) { x->data.dbl = MIN(x->data.dbl, y->data.dbl); }
			if (!strcmp(op, "max")) { x->data.dbl = MAX(x->data.dbl, y->data.dbl); }
		}
		else
		{
			if (!strcmp(op, "+")) { x->data.lng += y->data.lng; }
			if (!strcmp(op, "-")) { x->data.lng -= y->data.lng; }
			if (!strcmp(op, "*")) { x->data.lng *= y->data.lng; }

			if (!strcmp(op, "/"))
			{
				if (0 == y->data.lng)
				{
					lval_del(x); lval_del(y); // v is deleted after while
					x = lval_err(LERR_DIV_ZERO);
					break;
				}
				x->data.lng /= y->data.lng;
			}

			if (!strcmp(op, "%")) { x->data.lng %= y->data.lng; }
			if (!strcmp(op, "^")) { x->data.lng = (long)pow(x->data.lng, y->data.lng); }
			if (!strcmp(op, "min")) { x->data.lng = MIN(x->data.lng, y->data.lng); }
			if (!strcmp(op, "max")) { x->data.lng = MAX(x->data.lng, y->data.lng); }
		}
		lval_del(y);
	}
	lval_del(v);
	return x;
}

lval* lval_read(mpc_ast_t* ast)
{
	if (!ast) { return lval_err(LERR_OTHER); }

	if (strstr(ast->tag, "long")) { return _read_long(ast); }
	else if (strstr(ast->tag, "double")) { return _read_double(ast); }
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

static lval* _lval_pop(lval* v, int i)
{
	lval* x = v->cell[i];
	memmove(&v->cell[i], &v->cell[i+1], sizeof(lval*)*(v->count-i-1));

	v->count--;
	v->cell = (lval**)realloc(v->cell, sizeof(lval*)*v->count);

	return x;
}

static lval* _lval_take(lval* v, int i)
{
	lval* x = _lval_pop(v, i);
	lval_del(v);
	return x;
}

static lval* _read_long(mpc_ast_t* ast)
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

static lval* _read_double(mpc_ast_t* ast)
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
// 
// 	return lval_err(LERR_BAD_OP);
// }


