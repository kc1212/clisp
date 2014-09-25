
#ifndef EVAL_H_
#define EVAL_H_

#include "mpc/mpc.h"
#include "common.h"
#include "envi.h"

// assume lval.type cannot be error, only double or long
#define GET_LVAL_NUM_TYPE(LVAL) \
	(LVAL_DBL == LVAL->type? LVAL->data.dbl : LVAL->data.lng)

#define TO_LVAL_DBL(LVAL) \
	if (LVAL_LNG == LVAL->type) { \
		LVAL->data.dbl = (double)LVAL->data.lng; \
		LVAL->type = LVAL_DBL; \
	}

// TODO print debug info in assert
#define LVAL_ASSERT(e, args, cond, err) \
	if (!(cond)) { \
		if (e->debug) \
			debug("LVAL_ASSERT failed - %s", #cond); \
		lval_del(args); \
		return lval_err(err); \
	}

// TODO print debug info in assert
#define LVAL_ASSERT_SYM(args, cond, err, sym) \
	if (!(cond)) { \
		lval_del(args); \
		return lval_err(err); \
	}

lval* ast_to_lval(mpc_ast_t* ast);
lval* eval(lenv* e, lval* v);
int init_env(lenv* e);
lval* builtin_op(lenv* e, lval* v, char* op);
lval* builtin(lval* a, char* x);

lval* builtin_head(lenv* e, lval* a);
lval* builtin_tail(lenv* e, lval* a);
lval* builtin_eval(lenv* e, lval* a);
lval* builtin_quote(lenv* e, lval* a);
lval* builtin_join(lenv* e, lval* a);
lval* builtin_init(lenv* e, lval* a);
lval* builtin_len(lenv* e, lval* a);
lval* builtin_cons(lenv* e, lval* a);
lval* builtin_add(lenv* e, lval* a);
lval* builtin_sub(lenv* e, lval* a);
lval* builtin_mul(lenv* e, lval* a);
lval* builtin_div(lenv* e, lval* a);
lval* builtin_mod(lenv* e, lval* a);
lval* builtin_pow(lenv* e, lval* a);
lval* builtin_min(lenv* e, lval* a);
lval* builtin_max(lenv* e, lval* a);
lval* builtin_def(lenv* e, lval* a);

#endif

