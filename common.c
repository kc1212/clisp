
#include "common.h"
#include "assert.h"

static void _lval_expr_print(lval* v, const char open, const char close, FILE* fp);
static long _lval_expr_snprint(lval* v, const char open, const char close, char* str, const long n);
static void _lval_print(lval* v, FILE *fp);
static long _lval_snprint(lval* v, char* str, const long n);
int _lenv_print(lenv* e);
int _lenv_fprint(lenv* e, FILE* f);

void lval_println(lval* v)
{
	_lval_print(v, stdout); putchar('\n');
}

int lval_snprintln(lval *v, char* str, const long n)
{
	return _lval_snprint(v, str, n);
}

lval* lval_err(enum LVAL_ERRS e)
{
	lval* v = (lval*)calloc(1, sizeof(lval));
	if (NULL == v) { return NULL; }
	v->type = LVAL_ERR;
	v->err = e;
	return v;
}

void lval_del(lval* v)
{
	switch (v->type)
	{
		case LVAL_DBL:
		case LVAL_LNG: break;
		case LVAL_ERR: break;
		case LVAL_FUN: break;
		case LVAL_SYM: free(v->sym); break;

		case LVAL_QEXPR:
		case LVAL_SEXPR:
			for (int i = 0; i < v->count; i++) { lval_del(v->cell[i]); }
			free(v->cell);
			break;
	}
	free(v);
}

lval* lval_copy(lval* v)
{

	lval* x = (lval*)malloc(sizeof(lval));
	assert(x);
	x->type = v->type;

	switch (v->type)
	{
		case LVAL_FUN: x->fun = v->fun; break;
		case LVAL_DBL: x->data.dbl = v->data.dbl; break;
		case LVAL_LNG: x->data.lng = v->data.lng; break;
		case LVAL_ERR: x->err = v->err; break;
		case LVAL_SYM:
			x->sym = (char*)malloc(strlen(v->sym) + 1);
			strcpy(x->sym, v->sym);
			break;

		case LVAL_SEXPR:
		case LVAL_QEXPR:
			x->count = v->count;
			x->cell = malloc(sizeof(lval*) * x->count);
			for (int i = 0; i < x->count; i++)
			{
				x->cell[i] = lval_copy(v->cell[i]);
			}
			break;
		default:
			// something terrible happened
			v->type = LVAL_ERR;
			v->err = LERR_OTHER;
			break;
	}

	return x;
}

int colon_commands(const char* input, lenv* e)
{
	int action = COLON_OTHER;

	if (!strncmp(input, ":q", 2)) {
		action = COLON_BREAK;

	}
	else if (!strncmp(input, ":debug", 6)) {
		if (!strncmp(input+6, " true", 5)) {
			e->debug = 1;
			printf("debug mode on\n");
		}
		else if (!strncmp(input+6, " false", 6)) {
			e->debug = 0;
			printf("debug mode off\n");
		}
		else // TODO we can improve error reporting, maybe
			printf("ERROR: valid options are 'true' or 'false'\n");
		action = COLON_CONTINUE;
	}
	else if (!strncmp(input, ":env", 4)) {
		_lenv_print(e);
		action = COLON_CONTINUE;
	}
	return action;
}

// private functions: //////////////////////////////////////////////////////////

static void _lval_expr_print(lval* v, const char open, const char close, FILE* fp)
{
	putc(open, fp);
	for (int i = 0; i < v->count; i++)
	{
		_lval_print(v->cell[i], fp);
		if (i != (v->count-1)) { putc(' ', fp); }
	}
	putc(close, fp);
}

static long _lval_expr_snprint(lval* v, const char open, const char close, char* str, const long n)
{
	long tot = 0; // total number of characters printed (copied) to str excluding '\0'
	long ret = 0;

	if (n < 2) { return tot; };

	ret = snprintf(str, 2, "%c", open);
	tot += ret; // should be 1

	for (int i = 0; i < v->count; i++)
	{
		if (n-tot < 2) { return tot; }

		ret = _lval_snprint(v->cell[i], str+tot, n-tot);
		if (ret < 0) { return ret; }
		tot += ret;

		if (i != (v->count-1))
		{
			if (n-tot < 2) { return tot; }

			ret = snprintf(str+tot, n-tot, " ");
			if (ret < 0) { return ret; }
			tot += ret;
		}
	}

	if (n-tot < 2) { return tot; }

	ret = snprintf(str+tot, 2, "%c", close);
	tot += ret; // should be 0

	return tot;
}

static void _lval_print(lval* v, FILE *fp)
{
	switch (v->type)
	{
		case LVAL_LNG:		fprintf(fp, "%li", v->data.lng);	break;
		case LVAL_DBL:		fprintf(fp, "%f", v->data.dbl);	break;
		case LVAL_SYM:		fprintf(fp, "%s", v->sym);	break;
		case LVAL_SEXPR:	_lval_expr_print(v, '(', ')', fp);	break;
		case LVAL_QEXPR:	_lval_expr_print(v, '{', '}', fp);	break;
		case LVAL_FUN:		fprintf(fp, "%s", "<function>");	break;
		case LVAL_ERR:
			fprintf(fp, "%s", err_strings[v->err]);
			break;
		default:
			fprintf(fp, "%s", err_strings[LERR_OTHER]);
			break;
	}
}

static long _lval_snprint(lval* v, char* str, const long n)
{
	long ret = -1;
	if (n < 0) { return ret; }

	switch (v->type)
	{
		case LVAL_LNG:		ret = snprintf(str, n, "%li", v->data.lng);	break;
		case LVAL_DBL:		ret = snprintf(str, n, "%f", v->data.dbl);	break;
		case LVAL_SYM:		ret = snprintf(str, n, "%s", v->sym);		break;
		case LVAL_FUN:		ret = snprintf(str, n, "%s", "<function>");	break;
		case LVAL_SEXPR:
			ret = _lval_expr_snprint(v, '(', ')', str, n);
			break;
		case LVAL_QEXPR: // TODO, change to use quote
			ret = _lval_expr_snprint(v, '{', '}', str, n);
			break;
		case LVAL_ERR:
			ret = snprintf(str, n, "%s", err_strings[v->err]);
			break;
		default:
			ret = snprintf(str, n, "%s", err_strings[LERR_OTHER]);
			break;
	}
	return ret;
}


int _lenv_print(lenv* e)
{
	_lenv_fprint(e, stdout);
	return 0;
}

int _lenv_fprint(lenv* e, FILE* f)
{
	int i = 0;
	for (; i < e->count-1; i++) {
		fprintf(f, "%s ", e->syms[i]);
	}
	fprintf(f, "%s\n", e->syms[i]);
	return 0;
}


