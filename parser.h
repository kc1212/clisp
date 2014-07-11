#ifndef PARSER_H_
#define PARSER_H_

#include "mpc/mpc.h"

mpc_parser_t* Number;
mpc_parser_t* Operator;
mpc_parser_t* Expr;
mpc_parser_t* Lisp;

void init_parser();
void parse(const char* input);

#endif


