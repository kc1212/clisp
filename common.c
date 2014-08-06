
#include "common.h"
#include "assert.h"

static void _lval_expr_print(lval* v, const char open, const char close, FILE* fp);
static long _lval_expr_snprint(lval* v, const char open, const char close, char* str, const long n);
static void _lval_print(lval* v, FILE *fp);
static long _lval_snprint(lval* v, char* str, const long n);

void lval_println(lval* v)
{
	_lval_print(v, stdout); putchar('\n');
}

int lval_snprintln(lval *v, char* str, const long n)
{
	return _lval_snprint(v, str, n);
}

void lval_del(lval* v)
{
	switch (v->type)
	{
		case LVAL_DBL:
		case LVAL_LNG: break;
		case LVAL_ERR: break;
		case LVAL_SYM: free(v->sym); break;

		case LVAL_QEXPR:
		case LVAL_SEXPR:
			for (int i = 0; i < v->count; i++) { lval_del(v->cell[i]); }
			free(v->cell);
			break;
	}
	free(v);
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
	long tot = 0; // total number of characters printed (copied) to str
	long ret = 0;

	if (n < 1) { return -1; };

	*str = open; *(str+1) = '\0';
	tot++;

	for (int i = 0; i < v->count; i++)
	{
		if (n-tot < 1) { return -1; }
		ret = _lval_snprint(v->cell[i], str+tot, n-tot);
		if (ret < 0) { return ret; }
		tot += ret;

		if (i != (v->count-1))
		{
			if (n-tot < 1) { return -1; }
			ret = snprintf(str+tot, n-tot, " ");
			if (ret < 0) { return ret; }
			tot += ret;
		}
	}

	if (n-tot < 1) { return -1; }
	*(str+tot) = close; *(str+tot+1) = '\0';
	tot++;

	return tot;
}

static void _lval_print(lval* v, FILE *fp)
{
	switch (v->type)
	{
		case LVAL_LNG:		fprintf(fp, "%li", v->data.lng);	break;
		case LVAL_DBL:		fprintf(fp, "%f", v->data.dbl);		break;
		case LVAL_SYM:		fprintf(fp, "%s", v->sym);			break;
		case LVAL_SEXPR:	_lval_expr_print(v, '(', ')', fp);	break;
		case LVAL_QEXPR:	_lval_expr_print(v, '{', '}', fp);	break;
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
		case LVAL_SEXPR:
			ret = _lval_expr_snprint(v, '(', ')', str, n);
			break;
		case LVAL_QEXPR:
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



