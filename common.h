#ifndef COMMON_H_
#define COMMON_H_

#include <stdio.h>

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

enum {LVAL_NUM, LVAL_ERR};
enum {LERR_DIV_ZERO, LERR_BAD_OP, LERR_BAD_NUM};

typedef struct
{
	int type;
	long num;
	int err;
} lval;

lval lval_num(long x)
{
	lval v;
	v.type = LVAL_NUM;
	v.num = x;
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
		case LVAL_NUM:
			printf("%li", v.num);
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


#endif


