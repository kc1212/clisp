
#include "common.h"

lval lval_long(int64_t x)
{
	lval v;
	v.type = LVAL_LNG;
	v.data.lng = x;
	return v;
}

lval lval_double(double x)
{
	lval v;
	v.type = LVAL_DBL;
	v.data.dbl = x;
	return v;
}

lval lval_err(int x)
{
	lval v;
	v.type = LVAL_ERR;
	v.err = x;
	return v;
}

void lval_print(lval v)
{
	switch (v.type) {
		case LVAL_LNG:
			printf("%li", v.data.lng);
			break;
		case LVAL_DBL:
			printf("%f", v.data.dbl);
			break;
		case LVAL_ERR:
			if (v.err == LERR_DIV_ZERO) { printf("Error: Division By Zero!"); }
			if (v.err == LERR_BAD_OP)   { printf("Error: Invalid Operator!"); }
			if (v.err == LERR_BAD_NUM)  { printf("Error: Invalid Number!"); }
			break;
	}
}

void lval_println(lval v)
{
	lval_print(v); putchar('\n');
}


