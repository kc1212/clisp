
#include "common.h"
#include "assert.h"

static void _lval_expr_print(lval* v, const char open, const char close, FILE* fp);
static void _lval_print(lval* v, FILE *fp);

void lval_println(lval* v)
{
	_lval_print(v, stdout); putchar('\n');
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




