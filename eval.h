
#ifndef EVAL_H_
#define EVAL_H_

#include "mpc/mpc.h"
#include "common.h"
#include "envi.h"

// assume lval.type cannot be error, only double or long
#define GET_LVAL_DATA(LVAL) \
	(LVAL_DBL == LVAL->type? LVAL->data.dbl : LVAL->data.lng)

#define TO_LVAL_DBL(LVAL) \
	if (LVAL_LNG == LVAL->type) \
	{ LVAL->data.dbl = (double)LVAL->data.lng; LVAL->type = LVAL_DBL; }

#define LVAL_ASSERT(args, cond, err) \
	if (!(cond)) { lval_del(args); return _lval_err(err); }

#define LVAL_ASSERT_SYM(args, cond, err, sym) \
	if (!(cond)) { lval_del(args); return _lval_err(err); }

lval* ast_to_lval(mpc_ast_t* ast);
lval* eval(lval* v);
lval* builtin_op(lval* v, char* op);
lval* builtin_head(lval* a);
lval* builtin_tail(lval* a);
lval* builtin_eval(lval* a);
lval* builtin_quote(lval* a);
lval* builtin_join(lval* a);
lval* builtin_init(lval* a);
lval* builtin_len(lval* a);
lval* builtin_cons(lval* a);
lval* builtin(lval* a, char* x);
lval* builtin_add(lenv* e, lval* a);
lval* builtin_sub(lenv* e, lval* a);
lval* builtin_mul(lenv* e, lval* a);
lval* builtin_div(lenv* e, lval* a);
lval* builtin_mod(lenv* e, lval* a);
lval* builtin_pow(lenv* e, lval* a);
lval* builtin_min(lenv* e, lval* a);
lval* builtin_max(lenv* e, lval* a);

#endif

