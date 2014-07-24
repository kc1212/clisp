
#include "common.h"
#include "assert.h"

static void _lval_print(lval* v);
static void _lval_expr_print(lval* v, const char open, const char close);

lval* lval_long(int64_t x)
{
	lval* v = (lval*)calloc(1, sizeof(lval));
	assert(v); // TODO need to improve
	v->type = LVAL_LNG;
	v->data.lng = x;
	return v;
}

lval* lval_double(double x)
{
	lval* v = (lval*)calloc(1, sizeof(lval));
	assert(v); // TODO need to improve
	v->type = LVAL_DBL;
	v->data.dbl = x;
	return v;
}

lval* lval_sym(const char sym[])
{
	lval* v = (lval*)calloc(1, sizeof(lval));
	assert(v); // TODO need to improve
	v->type = LVAL_SYM;
	v->sym = (char*)calloc(strlen(sym)+1, sizeof(char));
	strcpy(v->sym, sym);
	return v;
}

lval* lval_sexpr(void)
{
	lval* v = (lval*)calloc(1, sizeof(lval));
	assert(v); // TODO need to improve
	v->type = LVAL_SEXPR;
	v->count = 0;
	v->cell = NULL;
	return v;
}

lval* lval_err(const char msg[])
{
	lval* v = (lval*)calloc(1, sizeof(lval));
	assert(v); // TODO need to improve
	v->type = LVAL_ERR;
	v->err = (char*)calloc(strlen(msg)+1, sizeof(char));
	strcpy(v->err, msg);
	return v;
}

void lval_del(lval* v)
{
	switch (v->type)
	{
		case LVAL_DBL:
		case LVAL_LNG: break;
		case LVAL_ERR: free(v->err); break;
		case LVAL_SYM: free(v->sym); break;

		case LVAL_SEXPR:
			for (int i = 0; i < v->count; i++) { lval_del(v->cell[i]); }
			free(v->cell);
			break;
	}
	free(v);
}

lval* lval_add(lval* v, lval* x)
{
	v->count++;
	v->cell = (lval**)realloc(v->cell, sizeof(lval*)*v->count);
	assert(v->cell);
	v->cell[v->count-1] = x; // set the last element
	return v;
}

void lval_println(lval* v)
{
	_lval_print(v); putchar('\n');
}

// private functions: //////////////////////////////////////////////////////////

static void _lval_expr_print(lval* v, const char open, const char close)
{
	putchar(open);
	for (int i = 0; i < v->count; i++)
	{
		_lval_print(v->cell[i]);
		if (i != (v->count-1)) { putchar(' '); }
	}
	putchar(close);
}

static void _lval_print(lval* v)
{
	switch (v->type)
	{
		case LVAL_LNG:		printf("%li", v->data.lng);		break;
		case LVAL_DBL:		printf("%f", v->data.dbl);		break;
		case LVAL_SYM:		printf("%s", v->sym);			break;
		case LVAL_SEXPR:	_lval_expr_print(v, '(', ')');	break;
		case LVAL_ERR:		printf("%s", v->err);			break;
	}
}
