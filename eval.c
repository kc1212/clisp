
#include "eval.h"
#include "math.h"

#include <string.h>
#include <float.h>
#include <assert.h>

static lval* _eval_sexpr(lval* v);

static lval* _lval_take(lval* v, int i);
static lval* _lval_pop(lval* v, int i);
static lval* _lval_join(lval* x, lval* y);
static lval* _lval_sexpr(void);
static lval* _lval_qexpr(void);
static lval* _lval_err(int e);
static lval* _lval_add_toback(lval* v, lval* x);
static lval* _lval_add_tofront(lval*v, lval* x);
static lval* _ast_to_long(mpc_ast_t* ast);
static lval* _lval_long(int64_t x);
static lval* _ast_to_double(mpc_ast_t* ast);
static lval* _lval_double(double x);
static lval* _lval_sym(const char sym[]);
static lval* _lval_fun(lbuiltin func);

// public functions ////////////////////////////////////////////////////////////
lval* eval(lval* v)
{
	if (v->type == LVAL_SEXPR) { return _eval_sexpr(v); }
	return v; // return same v if not sexpr
}

lval* builtin(lval* a, char* x)
{
	if (0 == strcmp("quote", x)) { return builtin_quote(a); }
	if (0 == strcmp("head", x) || 0 == strcmp("car", x)) { return builtin_head(a); }
	if (0 == strcmp("tail", x) || 0 == strcmp("cdr", x)) { return builtin_tail(a); }
	if (0 == strcmp("join", x)) { return builtin_join(a); }
	if (0 == strcmp("eval", x)) { return builtin_eval(a); }
	if (0 == strcmp("cons", x)) { return builtin_cons(a); }
	if (0 == strcmp("len", x)) { return builtin_len(a); }
	if (0 == strcmp("init", x)) { return builtin_init(a); }

	if (strstr("^%+-/*", x) || strstr("min", x) || strstr("max", x) || strstr("pow", x))
	{
		return builtin_op(a, x);
	}

	lval_del(a);
	return _lval_err(LERR_BAD_FUNCTION);
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

	if (strstr(ast->tag, "long")) { return _ast_to_long(ast); }
	else if (strstr(ast->tag, "double")) { return _ast_to_double(ast); }
	else if (strstr(ast->tag, "symbol")) { return _lval_sym(ast->contents); }

	lval* x = NULL;   // ">" is root
	if (strcmp(ast->tag, ">") == 0 ) { x = _lval_sexpr(); }
	else if (strstr(ast->tag, "sexpr")) { x = _lval_sexpr(); }
	else if (strstr(ast->tag, "qexpr")) { x = _lval_qexpr(); }
	assert(x); // TODO

	for (int i = 0; i < ast->children_num; i++)
	{
		if (strcmp(ast->children[i]->contents, "(") == 0
			|| strcmp(ast->children[i]->contents, ")") == 0
			|| strcmp(ast->children[i]->contents, "}") == 0
			|| strcmp(ast->children[i]->contents, "{") == 0
			|| strcmp(ast->children[i]->tag,  "regex") == 0
			) { continue; }
		x = _lval_add_toback(x, ast_to_lval(ast->children[i]));
	}

	return x;
}

lval* builtin_head(lval* a)
{
	LVAL_ASSERT(a, (a->count == 1), LERR_TOO_MANY_ARGS);
	LVAL_ASSERT(a, (a->cell[0]->type == LVAL_QEXPR), LERR_BAD_TYPE);
	LVAL_ASSERT(a, (a->cell[0]->count != 0), LERR_EMPTY);

	lval* v = _lval_take(a, 0);
	while (v->count > 1) { lval_del(_lval_pop(v, 1)); }
	return v;
}

lval* builtin_tail(lval* a)
{
	LVAL_ASSERT(a, (a->count == 1), LERR_TOO_MANY_ARGS);
	LVAL_ASSERT(a, (a->cell[0]->type == LVAL_QEXPR), LERR_BAD_TYPE);
	LVAL_ASSERT(a, (a->cell[0]->count != 0), LERR_EMPTY);

	lval* v = _lval_take(a, 0);
	lval_del(_lval_pop(v, 0));
	return v;
}

lval* builtin_quote(lval* a)
{
	a->type = LVAL_QEXPR;
	return a;
}

lval* builtin_eval(lval* a)
{
	LVAL_ASSERT(a, (a->count == 1), LERR_TOO_MANY_ARGS);
	LVAL_ASSERT(a, (a->cell[0]->type == LVAL_QEXPR), LERR_BAD_TYPE);

	lval* x = _lval_take(a, 0);
	x->type = LVAL_SEXPR;
	return eval(x);
}

lval* builtin_join(lval* a)
{
	for (int i = 0; i < a->count; i++)
	{
		LVAL_ASSERT(a, (a->cell[i]->type == LVAL_QEXPR), LERR_BAD_TYPE);
	}

	lval* x = _lval_pop(a, 0);

	while (a->count) { x = _lval_join(x, _lval_pop(a, 0)); }

	lval_del(a);
	return x;
}

lval* builtin_cons(lval* a)
{
	LVAL_ASSERT(a, (2 == a->count), LERR_BAD_ARGS_COUNT);
	LVAL_ASSERT(a, (LVAL_QEXPR == a->cell[1]->type), LERR_BAD_TYPE);

	lval* item = _lval_pop(a, 0);
	lval* list = _lval_take(a, 0); // take will free 'a'
	list = _lval_add_tofront(list, item);
	return list;
}

lval* builtin_len(lval* a)
{
	LVAL_ASSERT(a, (1 == a->count), LERR_TOO_MANY_ARGS);
	LVAL_ASSERT(a, (LVAL_QEXPR == a->cell[0]->type), LERR_BAD_TYPE);

	return _lval_long(a->cell[0]->count);
}

lval* builtin_init(lval* a)
{
	LVAL_ASSERT(a, (1 == a->count), LERR_TOO_MANY_ARGS);
	LVAL_ASSERT(a, (LVAL_QEXPR == a->cell[0]->type), LERR_BAD_TYPE);
	LVAL_ASSERT(a, (0 != a->cell[0]->count), LERR_EMPTY);

	lval* v = _lval_take(a, 0); // take main qexpr
	lval_del(_lval_pop(v, v->count-1));
	return v;
}

// private functions: //////////////////////////////////////////////////////////

static lval* _lval_add_toback(lval* v, lval* x)
{
	v->count++;
	v->cell = (lval**)realloc(v->cell, sizeof(lval*)*v->count);
	if (NULL == v->cell) { return NULL; }
	v->cell[v->count-1] = x; // set the last element
	return v;
}

static lval* _lval_add_tofront(lval*v, lval* x)
{
	v->count++;
	v->cell = (lval**)realloc(v->cell, sizeof(lval*)*v->count);
	if (NULL == v->cell) { return NULL; }
	memmove(v->cell+1, v->cell, sizeof(lval*)*(v->count-1));
	v->cell[0] = x;
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
		if (v->cell[i]->type == LVAL_ERR) { return _lval_take(v, i); }
	}

	if (v->count == 0) { return v; }
	if (v->count == 1) { return _lval_take(v, 0); }

	// take the first element and make sure it's a symbol (op)
	lval* f = _lval_pop(v, 0);
	if (f->type != LVAL_SYM)
	{
		// log_err("First element must be a symbol, not of type %d", f->type);
		lval_del(f); lval_del(v);
		return _lval_err(LERR_BAD_SEXPR_START);
	}

	lval* result = builtin(v, f->sym);
	lval_del(f);
	return result;
}

static lval* _lval_pop(lval* v, int i)
{
	lval* x = v->cell[i];
	memmove(&v->cell[i], &v->cell[i+1], sizeof(lval*)*(v->count-i-1));

	v->count--;
	v->cell = (lval**)realloc(v->cell, sizeof(lval*)*v->count);
	if (0 != v->count && NULL == v->cell ) { return NULL; }
	return x;
}

static lval* _lval_take(lval* v, int i)
{
	lval* x = _lval_pop(v, i);
	lval_del(v);
	return x;
}

static lval* _ast_to_long(mpc_ast_t* ast)
{
	errno = 0;
	int64_t x = strtol(ast->contents, NULL, 10);
	if (errno)
	{
		// log_err("strtol conversion failed for %s", ast->contents);
		return _lval_err(LERR_BAD_NUM);
	}
	return _lval_long(x);
}

static lval* _lval_long(int64_t x)
{
	lval* v = (lval*)calloc(1, sizeof(lval));
	if (NULL == v) { return NULL; }
	v->type = LVAL_LNG;
	v->data.lng = x;
	return v;
}

static lval* _lval_sym(const char sym[])
{
	lval* v = (lval*)calloc(1, sizeof(lval));
	if (NULL == v) { return NULL; }
	v->type = LVAL_SYM;
	v->sym = (char*)calloc(strlen(sym)+1, sizeof(char));
	assert(v->sym);
	strcpy(v->sym, sym);
	return v;
}

static lval* _ast_to_double(mpc_ast_t* ast)
{
	errno = 0;
	double x = strtod(ast->contents, NULL);
	if (errno)
	{
		// log_err("strtod conversion failed for %s", ast->contents);
		return _lval_err(LERR_BAD_NUM);
	}
	return _lval_double(x);
}

static lval* _lval_double(double x)
{
	lval* v = (lval*)calloc(1, sizeof(lval));
	if (NULL == v) { return NULL; }
	v->type = LVAL_DBL;
	v->data.dbl = x;
	return v;
}

static lval* _lval_sexpr(void)
{
	lval* v = (lval*)calloc(1, sizeof(lval));
	if (NULL == v) { return NULL; }
	v->type = LVAL_SEXPR;
	v->count = 0;
	v->cell = NULL;
	return v;
}

static lval* _lval_qexpr(void)
{
	lval* v = (lval*)malloc(sizeof(lval));
	if (NULL == v) { return NULL; }
	v->type = LVAL_QEXPR;
	v->count = 0;
	v->cell = NULL;
	return v;
}

static lval* _lval_fun(lbuiltin func)
{
	lval* v = (lval*)malloc(sizeof(lval));
	if (NULL == v) { return NULL; }
	v->type = LVAL_FUN;
	v->fun = func;
	return v;
}

static lval* _lval_err(int e)
{
	lval* v = (lval*)calloc(1, sizeof(lval));
	if (NULL == v) { return NULL; }
	v->type = LVAL_ERR;
	v->err = e;
	return v;
}

static lval* _lval_join(lval* x, lval* y)
{
	while (y->count) { x = _lval_add_toback(x, _lval_pop(y, 0)); }

	lval_del(y);
	return x; // x is reallocated so it's fine
}

