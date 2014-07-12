
#ifndef EVAL_H_
#define EVAL_H_

#include "mpc/mpc.h"

int eval(mpc_ast_t * ast);
long eval_op(long x, char* op, long y);

#endif

