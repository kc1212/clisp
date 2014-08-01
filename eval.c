
#include "eval.h"
#include "math.h"

#include <string.h>
#include <float.h>
#include <assert.h>

static lval* _take_lval(lval* v, int i);
static lval* _pop_lval(lval* v, int i);
static lval* _eval_sexpr(lval* v);

static lval* _lval_sexpr(void);
static lval* _lval_qexpr(void);
static lval* _lval_err(int e);
static lval* _lval_add(lval* v, lval* x);
static lval* _lval_long(mpc_ast_t* ast);
static lval* _lval_double(mpc_ast_t* ast);
static lval* _lval_sym(const char sym[]);

// public functions ////////////////////////////////////////////////////////////
lval* eval(lval* v)
{
	if (v->type == LVAL_SEXPR) { return _eval_sexpr(v); }
	return v; // return same v if not sexpr
}

lval* builtin_op(lval* v, char* op)
{
	for (int i = 0; i < v->count; i++) // ensure all children are numbers
	{
		if (v->cell[i]->type != LVAL_LNG && v->cell[i]->type != LVAL_DBL)
		{
			// log_err("not all children are numbers - type: %d", v->cell[i]->type);
			lval_del(v);
			return _lval_err(LERR_BAD_NUM);
		}
	}

	lval* x = _pop_lval(v, 0);
	if ((strcmp(op, "-") == 0) && v->count == 0)
	{
		if (LVAL_DBL == x->type) { x->data.dbl = -x->data.dbl; }
		if (LVAL_LNG == x->type) { x->data.lng = -x->data.lng; }
	}

	while (v->count > 0)
	{
		lval* y = _pop_lval(v, 0);

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
					x = _lval_err(LERR_DIV_ZERO);
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
					x = _lval_err(LERR_DIV_ZERO);
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

lval* ast_to_lval(mpc_ast_t* ast) // converts ast to lval
{
	if (NULL == ast) { return _lval_err(LERR_OTHER); }

	if (strstr(ast->tag, "long")) { return _lval_long(ast); }
	else if (strstr(ast->tag, "double")) { return _lval_double(ast); }
	else if (strstr(ast->tag, "symbol")) { return _lval_sym(ast->contents); }

	lval* x = NULL;   // ">" is root
	if (strcmp(ast->tag, ">") == 0 ) { x = _lval_sexpr(); }
	else if (strstr(ast->tag, "sexpr")) { x = _lval_sexpr(); }
	else if (strstr(ast->tag, "qexpr")) { x = _lval_qexpr(); }

	for (int i = 0; i < ast->children_num; i++)
	{
		if (strcmp(ast->children[i]->contents, "(") == 0
			|| strcmp(ast->children[i]->contents, ")") == 0
			|| strcmp(ast->children[i]->contents, "}") == 0
			|| strcmp(ast->children[i]->contents, "{") == 0
			|| strcmp(ast->children[i]->tag,  "regex") == 0
			) { continue; }
		x = _lval_add(x, ast_to_lval(ast->children[i]));
	}

	return x;
}

// private functions: //////////////////////////////////////////////////////////

static lval* _lval_add(lval* v, lval* x)
{
	v->count++;
	v->cell = (lval**)realloc(v->cell, sizeof(lval*)*v->count);
	assert(v->cell);
	v->cell[v->count-1] = x; // set the last element
	return v;
}

static lval* _eval_sexpr(lval* v)
{
	// eval children
	for (int i = 0; i < v->count; i++)
	{
		v->cell[i] = eval(v->cell[i]);
	}

	for (int i = 0; i < v->count; i++)
	{
		if (v->cell[i]->type == LVAL_ERR) { return _take_lval(v, i); }
	}

	if (v->count == 0) { return v; }
	if (v->count == 1) { return _take_lval(v, 0); }

	// take the first element and make sure it's a symbol (op)
	lval* f = _pop_lval(v, 0);
	if (f->type != LVAL_SYM)
	{
		// log_err("First element must be a symbol, not of type %d", f->type);
		lval_del(f); lval_del(v);
		return _lval_err(LERR_BAD_SEXPR_START);
	}

	lval* result = builtin_op(v, f->sym);
	lval_del(f);
	return result;
}

static lval* _pop_lval(lval* v, int i)
{
	lval* x = v->cell[i];
	memmove(&v->cell[i], &v->cell[i+1], sizeof(lval*)*(v->count-i-1));

	v->count--;
	v->cell = (lval**)realloc(v->cell, sizeof(lval*)*v->count);
	if (0 != v->count) { assert(v->cell); } // TODO
	return x;
}

static lval* _take_lval(lval* v, int i)
{
	lval* x = _pop_lval(v, i);
	lval_del(v);
	return x;
}

static lval* _lval_long(mpc_ast_t* ast)
{
	errno = 0;
	int64_t x = strtol(ast->contents, NULL, 10);
	if (errno)
	{
		// log_err("strtol conversion failed for %s", ast->contents);
		return _lval_err(LERR_BAD_NUM);
	}

	lval* v = (lval*)calloc(1, sizeof(lval));
	assert(v); // TODO need to improve
	v->type = LVAL_LNG;
	v->data.lng = x;
	return v;
}

static lval* _lval_sym(const char sym[])
{
	lval* v = (lval*)calloc(1, sizeof(lval));
	assert(v);
	v->type = LVAL_SYM;
	v->sym = (char*)calloc(strlen(sym)+1, sizeof(char));
	assert(v->sym);
	strcpy(v->sym, sym);
	return v;
}


static lval* _lval_double(mpc_ast_t* ast)
{
	errno = 0;
	double x = strtod(ast->contents, NULL);
	if (errno)
	{
		// log_err("strtod conversion failed for %s", ast->contents);
		return _lval_err(LERR_BAD_NUM);
	}

	lval* v = (lval*)calloc(1, sizeof(lval));
	assert(v); // TODO need to improve
	v->type = LVAL_DBL;
	v->data.dbl = x;
	return v;
}

static lval* _lval_sexpr(void)
{
	lval* v = (lval*)calloc(1, sizeof(lval));
	assert(v); // TODO need to improve
	v->type = LVAL_SEXPR;
	v->count = 0;
	v->cell = NULL;
	return v;
}

static lval* _lval_qexpr(void)
{
	lval* v = (lval*)malloc(sizeof(lval));
	assert(v); // TODO
	v->type = LVAL_QEXPR;
	v->count = 0;
	v->cell = NULL;
	return v;
}

static lval* _lval_err(int e)
{
	lval* v = (lval*)calloc(1, sizeof(lval));
	assert(v); // TODO need to improve
	v->type = LVAL_ERR;
	v->err = e;
	return v;
}


