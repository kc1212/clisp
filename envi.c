
#include "envi.h"

lenv* lenv_new(void)
{
	lenv* e = (lenv*)malloc(sizeof(lenv));
	if (NULL == e) return NULL;
	e->count = 0;
	e->syms = NULL;
	e->vals = NULL;
	e->debug = 1;
	return e;
}

void lenv_del(lenv* e)
{
	for (int i = 0; i < e->count && NULL != e->syms[i]; i++)
	{
		free(e->syms[i]);
		lval_del(e->vals[i]);
	}
	e->count = 0;

	if (NULL != e->syms)
		free(e->syms);
	e->syms = NULL;

	if (NULL != e->vals)
		free(e->vals);
	e->vals = NULL;

	free(e);
	e = NULL;
}

lval* lenv_get(lenv* e, lval* k)
{
	for (int i = 0; i < e->count; i++)
	{
		if (strcmp(e->syms[i], k->sym) == 0)
			return lval_copy(e->vals[i]);
	}
	if (e->debug)
		debug("Symbol: '%s' not found.", k->sym);
	return lval_err(LERR_BAD_SYMBOL);
}

int lenv_put(lenv* e, lval* k, lval* v) {

	for (int i = 0; i < e->count; i++)
	{
		if (strcmp(e->syms[i], k->sym) == 0)
		{
			lval_del(e->vals[i]);
			e->vals[i] = lval_copy(v);
			return 0;
		}
	}

	// TODO need error checking
	e->count++;
	e->vals = realloc(e->vals, sizeof(lval*) * e->count);
	e->syms = realloc(e->syms, sizeof(char*) * e->count);

	e->vals[e->count-1] = lval_copy(v);
	e->syms[e->count-1] = malloc(strlen(k->sym)+1);
	strcpy(e->syms[e->count-1], k->sym);
	return 0;
}



