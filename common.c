
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
			if (LERR_DIV_ZERO == v->err) { fprintf(fp, "Error: Division By Zero!\n"); }
			if (LERR_BAD_OP == v->err) { fprintf(fp, "Error: Invalid Operator!\n"); }
			if (LERR_BAD_NUM == v->err) { fprintf(fp, "Error: Invalid Number!\n"); }
			if (LERR_OTHER == v->err) { fprintf(fp, "Critical Error!\n"); }
			if (LERR_BAD_SEXPR_START == v->err) { fprintf(fp, "S-expression Does not start with symbol!\n"); }
			break;
		default:
			fprintf(fp, "Critical Error! - undefined lval.type\n");
			break;
	}
}




