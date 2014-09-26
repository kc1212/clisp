
#include "eval.h"
#include "envi.h"
#include "math.h"

#include <string.h>
#include <float.h>
#include <assert.h>

static lval* _eval_sexpr(lenv* e, lval* v);
static lval* _lval_take(lval* v, int i);
static lval* _lval_pop(lval* v, int i);
static lval* _lval_join(lval* x, lval* y);
static lval* _lval_sexpr(void);
static lval* _lval_qexpr(void);
static lval* _lval_add_toback(lval* v, lval* x);
static lval* _lval_add_tofront(lval*v, lval* x);
static lval* _ast_to_long(mpc_ast_t* ast);
static lval* _lval_long(int64_t x);
static lval* _ast_to_double(mpc_ast_t* ast);
static lval* _lval_double(double x);
static lval* _lval_sym(const char sym[]);
static lval* _lval_fun(lbuiltin func);
static int _add_builtin_to_env(lenv* e, char name[], lbuiltin func);

// public functions ////////////////////////////////////////////////////////////
int init_env(lenv* e)
{
	_add_builtin_to_env(e, "quote", builtin_quote);
	_add_builtin_to_env(e, "head", builtin_head);
	_add_builtin_to_env(e, "tail", builtin_tail);
	_add_builtin_to_env(e, "join", builtin_join);
	_add_builtin_to_env(e, "eval", builtin_eval);
	_add_builtin_to_env(e, "cons", builtin_cons);
	_add_builtin_to_env(e, "len",  builtin_len );
	_add_builtin_to_env(e, "init", builtin_init);
	_add_builtin_to_env(e, "def",  builtin_def);

	_add_builtin_to_env(e, "+", builtin_add);
	_add_builtin_to_env(e, "-", builtin_sub);
	_add_builtin_to_env(e, "*", builtin_mul);
	_add_builtin_to_env(e, "/", builtin_div);
	_add_builtin_to_env(e, "%", builtin_mod);
	_add_builtin_to_env(e, "^", builtin_pow);
	_add_builtin_to_env(e, "min", builtin_min);
	_add_builtin_to_env(e, "max", builtin_max);

	// builtins under different name
	_add_builtin_to_env(e, "list", builtin_quote);
	_add_builtin_to_env(e, "car", builtin_head);
	_add_builtin_to_env(e, "cdr", builtin_tail);

	return 0; // TODO error checking
}

lval* eval(lenv* e, lval* v)
{
	if (v->type == LVAL_SYM) {
		lval* x = lenv_get(e, v);
		lval_del(v);
		return x;
	}
	if (v->type == LVAL_SEXPR)
		return _eval_sexpr(e, v);

	return v; // return same v if not sexpr
}

lval* builtin_op(lenv* e, lval* v, char* op)
{
	for (int i = 0; i < v->count; i++) { // ensure all children are numbers
		if (v->cell[i]->type != LVAL_LNG && v->cell[i]->type != LVAL_DBL) {
			if (e->debug)
				debug("Not all children are numbers - type: %d", v->cell[i]->type);
			lval_del(v);
			return lval_err(LERR_BAD_NUM);
		}
	}

	lval* x = _lval_pop(v, 0);
	if ((strcmp(op, "-") == 0) && v->count == 0) {
		if (LVAL_DBL == x->type) { x->data.dbl = -x->data.dbl; }
		if (LVAL_LNG == x->type) { x->data.lng = -x->data.lng; }
	}

	while (v->count > 0) {
		lval* y = _lval_pop(v, 0);

		if (x->type == LVAL_DBL || y->type == LVAL_DBL) {
			TO_LVAL_DBL(x); TO_LVAL_DBL(y);
			if (!strcmp(op, "+")) { x->data.dbl += y->data.dbl; }
			if (!strcmp(op, "-")) { x->data.dbl -= y->data.dbl; }
			if (!strcmp(op, "*")) { x->data.dbl *= y->data.dbl; }

			if (!strcmp(op, "/")) {
				if (DBL_EPSILON > y->data.dbl) {
					if (e->debug)
						debug("Division by zero! (%f/%f)", x->data.dbl, y->data.dbl);
					lval_del(x);
					lval_del(y); // v is deleted after while
					x = lval_err(LERR_DIV_ZERO);
					break;
				}
				x->data.dbl /= y->data.dbl;
			}

			if (!strcmp(op, "^")) { x->data.dbl = pow(x->data.dbl, y->data.dbl); }
			if (!strcmp(op, "min")) { x->data.dbl = MIN(x->data.dbl, y->data.dbl); }
			if (!strcmp(op, "max")) { x->data.dbl = MAX(x->data.dbl, y->data.dbl); }
		}
		else {
			if (!strcmp(op, "+")) { x->data.lng += y->data.lng; }
			if (!strcmp(op, "-")) { x->data.lng -= y->data.lng; }
			if (!strcmp(op, "*")) { x->data.lng *= y->data.lng; }

			if (!strcmp(op, "/")) {
				if (0 == y->data.lng) {
					if (e->debug)
						debug("Division by zero! (%ld/%ld)", x->data.lng, y->data.lng);
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

lval* ast_to_lval(mpc_ast_t* ast)
{
	if (NULL == ast)
		return lval_err(LERR_OTHER);

	if (strstr(ast->tag, "long"))
		return _ast_to_long(ast);
	else if (strstr(ast->tag, "double"))
		return _ast_to_double(ast);
	else if (strstr(ast->tag, "symbol"))
		return _lval_sym(ast->contents);

	lval* x = NULL; // ">" is root
	if (0 == strcmp(ast->tag, ">"))
		x = _lval_sexpr();
	else if (strstr(ast->tag, "sexpr"))
		x = _lval_sexpr();
	else if (strstr(ast->tag, "qexpr"))
		x = _lval_qexpr();

	if (NULL == x)
		return lval_err(LERR_OTHER);

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

lval* builtin_head(lenv* e, lval* a)
{
	LVAL_ASSERT(e, a, (a->count == 1), LERR_TOO_MANY_ARGS);
	LVAL_ASSERT(e, a, (a->cell[0]->type == LVAL_QEXPR), LERR_BAD_TYPE);
	LVAL_ASSERT(e, a, (a->cell[0]->count != 0), LERR_EMPTY);

	lval* v = _lval_take(a, 0);
	while (v->count > 1)
		lval_del(_lval_pop(v, 1));
	return v;
}

lval* builtin_tail(lenv* e, lval* a)
{
	LVAL_ASSERT(e, a, (a->count == 1), LERR_TOO_MANY_ARGS);
	LVAL_ASSERT(e, a, (a->cell[0]->type == LVAL_QEXPR), LERR_BAD_TYPE);
	LVAL_ASSERT(e, a, (a->cell[0]->count != 0), LERR_EMPTY);

	lval* v = _lval_take(a, 0);
	lval_del(_lval_pop(v, 0));
	return v;
}

lval* builtin_quote(lenv* e, lval* a)
{
	(void*)e;
	a->type = LVAL_QEXPR;
	return a;
}

lval* builtin_eval(lenv* e, lval* a)
{
	LVAL_ASSERT(e, a, (a->count == 1), LERR_TOO_MANY_ARGS);
	LVAL_ASSERT(e, a, (a->cell[0]->type == LVAL_QEXPR), LERR_BAD_TYPE);

	lval* x = _lval_take(a, 0);
	x->type = LVAL_SEXPR;
	return eval(e, x);
}

lval* builtin_join(lenv* e, lval* a)
{
	for (int i = 0; i < a->count; i++)
		LVAL_ASSERT(e, a, (a->cell[i]->type == LVAL_QEXPR), LERR_BAD_TYPE);

	lval* x = _lval_pop(a, 0);

	while (a->count) { x = _lval_join(x, _lval_pop(a, 0)); }

	lval_del(a);
	return x;
}

lval* builtin_cons(lenv* e, lval* a)
{
	LVAL_ASSERT(e, a, (2 == a->count), LERR_BAD_ARGS_COUNT);
	LVAL_ASSERT(e, a, (LVAL_QEXPR == a->cell[1]->type), LERR_BAD_TYPE);

	lval* item = _lval_pop(a, 0);

	// !!! TODO this may not be correct, if the user goes: cons {a} {1 2}
	// do we return {a 1 2} or {{a} 1 2}?
	if (item->count == 1) {
		item = _lval_take(item, 0);
	}

	lval* list = _lval_take(a, 0); // take will free 'a'
	list = _lval_add_tofront(list, item);
	return list;
}

lval* builtin_len(lenv* e, lval* a)
{
	LVAL_ASSERT(e, a, (1 == a->count), LERR_TOO_MANY_ARGS);
	LVAL_ASSERT(e, a, (LVAL_QEXPR == a->cell[0]->type), LERR_BAD_TYPE);

	lval* x = _lval_long(a->cell[0]->count);
	lval_del(a);
	return x;
}

lval* builtin_init(lenv* e, lval* a)
{
	LVAL_ASSERT(e, a, (1 == a->count), LERR_TOO_MANY_ARGS);
	LVAL_ASSERT(e, a, (LVAL_QEXPR == a->cell[0]->type), LERR_BAD_TYPE);
	LVAL_ASSERT(e, a, (0 != a->cell[0]->count), LERR_EMPTY);

	lval* v = _lval_take(a, 0); // take main qexpr
	lval_del(_lval_pop(v, v->count-1));
	return v;
}

lval* builtin_def(lenv* e, lval* a)
{
	LVAL_ASSERT(e, a, (a->cell[0]->type == LVAL_QEXPR), LERR_BAD_TYPE);

	lval* syms = a->cell[0]; // first arg is symbol list

	for (int i = 0; i < syms->count; i++)
		LVAL_ASSERT(e, a, (syms->cell[i]->type == LVAL_SYM), LERR_BAD_TYPE);

	LVAL_ASSERT(e, a, (syms->count == a->count-1), LERR_BAD_ARGS_COUNT);

	for (int i = 0; i < syms->count; i++)
		lenv_put(e, syms->cell[i], a->cell[i+1]);

	lval_del(a);
	return _lval_sexpr();
}

lval* builtin_add(lenv* e, lval* a) { return builtin_op(e, a, "+"); }
lval* builtin_sub(lenv* e, lval* a) { return builtin_op(e, a, "-"); }
lval* builtin_mul(lenv* e, lval* a) { return builtin_op(e, a, "*"); }
lval* builtin_div(lenv* e, lval* a) { return builtin_op(e, a, "/"); }
lval* builtin_mod(lenv* e, lval* a) { return builtin_op(e, a, "%"); }
lval* builtin_pow(lenv* e, lval* a) { return builtin_op(e, a, "^"); }
lval* builtin_min(lenv* e, lval* a) { return builtin_op(e, a, "min"); }
lval* builtin_max(lenv* e, lval* a) { return builtin_op(e, a, "max"); }


// private functions: //////////////////////////////////////////////////////////

static lval* _lval_add_toback(lval* v, lval* x)
{
	// TODO v and return value are the same
	v->count++;
	v->cell = (lval**)realloc(v->cell, sizeof(lval*)*v->count);
	if (NULL == v->cell)
		return NULL;
	v->cell[v->count-1] = x; // set the last element
	return v;
}

static lval* _lval_add_tofront(lval*v, lval* x)
{
	// TODO v and return value are the same
	v->count++;
	v->cell = (lval**)realloc(v->cell, sizeof(lval*)*v->count);
	if (NULL == v->cell)
		return NULL;
	memmove(v->cell+1, v->cell, sizeof(lval*)*(v->count-1));
	v->cell[0] = x;
	return v;
}

static lval* _eval_sexpr(lenv* e, lval* v)
{
	int is_qexpr = 0;
	if (v->cell && v->cell[0] && v->cell[0]->sym)
		is_qexpr = (0 == strncmp("quote", v->cell[0]->sym, 5))
			|| (0 == strncmp("list", v->cell[0]->sym, 5));
	// TODO change the above to regex

	for (int i = 0; i < v->count; i++) {
		// skip eval if the function is qexpr
		if (!is_qexpr || 0 == i)
			v->cell[i] = eval(e, v->cell[i]);
		if (v->cell[i]->type == LVAL_ERR)
			return _lval_take(v, i);
	}

	if (v->count == 0)
		return v;
	if (v->count == 1)
		return _lval_take(v, 0);

	// take the first element and make sure it's a function
	lval* f = _lval_pop(v, 0);
	if (f->type != LVAL_FUN) {
		if (e->debug)
			debug("First element must be a symbol, not of type %d", f->type);
		lval_del(f);
		lval_del(v);
		return lval_err(LERR_BAD_SEXPR_START);
	}

	lval* result = f->fun(e, v);
	lval_del(f);
	return result;
}

static lval* _lval_pop(lval* v, int i)
{
	lval* x = v->cell[i];
	memmove(&v->cell[i], &v->cell[i+1], sizeof(lval*)*(v->count-i-1));

	v->count--;
	v->cell = (lval**)realloc(v->cell, sizeof(lval*)*v->count);
	if (0 != v->count && NULL == v->cell )
		return NULL;
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
		return lval_err(LERR_BAD_NUM);
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
	if (NULL == v)
		return NULL;
	v->type = LVAL_SYM;
	v->sym = (char*)calloc(strlen(sym)+1, sizeof(char));
	if (NULL == v->sym)
		return NULL;
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
		return lval_err(LERR_BAD_NUM);
	}
	return _lval_double(x);
}

static lval* _lval_double(double x)
{
	lval* v = (lval*)calloc(1, sizeof(lval));
	if (NULL == v)
		return NULL;
	v->type = LVAL_DBL;
	v->data.dbl = x;
	return v;
}

static lval* _lval_sexpr(void)
{
	lval* v = (lval*)calloc(1, sizeof(lval));
	if (NULL == v)
		return NULL;
	v->type = LVAL_SEXPR;
	v->count = 0;
	v->cell = NULL;
	return v;
}

static lval* _lval_qexpr(void)
{
	lval* v = (lval*)calloc(1, sizeof(lval));
	if (NULL == v)
		return NULL;
	v->type = LVAL_QEXPR;
	v->count = 0;
	v->cell = NULL;
	return v;
}

static lval* _lval_fun(lbuiltin func)
{
	lval* v = (lval*)calloc(1, sizeof(lval));
	if (NULL == v)
		return NULL;
	v->type = LVAL_FUN;
	v->fun = func;
	return v;
}

static lval* _lval_join(lval* x, lval* y)
{
	while (y->count)
		x = _lval_add_toback(x, _lval_pop(y, 0));

	lval_del(y);
	return x; // x is reallocated so it's fine
}

static int _add_builtin_to_env(lenv* e, char name[], lbuiltin func)
{
	lval* k = _lval_sym(name);
	lval* v = _lval_fun(func);
	lenv_put(e, k, v);
	lval_del(k);
	lval_del(v);
	return 0;
}




