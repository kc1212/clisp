
#ifndef EVAL_H_
#define EVAL_H_

#include "mpc/mpc.h"
#include "common.h"

// assume lval.type cannot be error, only double or long
#define GET_LVAL_DATA(LVAL) \
	(LVAL.type == LVAL_DBL ? LVAL.data.dbl : LVAL.data.lng)

lval eval(mpc_ast_t * ast);

#endif

