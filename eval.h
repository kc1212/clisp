
#ifndef EVAL_H_
#define EVAL_H_

#include "mpc/mpc.h"
#include "common.h"

// assume lval.type cannot be error, only double or long
#define GET_LVAL_DATA(LVAL) \
	(LVAL_DBL == LVAL->type? LVAL->data.dbl : LVAL->data.lng)

#define TO_LVAL_DBL(LVAL) \
	if (LVAL_LNG == LVAL->type) \
	{ LVAL->data.dbl = (double)LVAL->data.lng; LVAL->type = LVAL_DBL; }

// lval* eval(mpc_ast_t * ast);
lval* lval_read(mpc_ast_t* ast);
lval* lval_eval(lval* v);
lval* lval_eval_sexpr(lval* v);
lval* builtin_op(lval* v, char* op);

#endif

