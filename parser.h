#ifndef PARSER_H_
#define PARSER_H_

#include "mpc/mpc.h"

mpc_parser_t* Long;
mpc_parser_t* Double;
mpc_parser_t* Symbol;
mpc_parser_t* Sexpr;
mpc_parser_t* Qexpr;
mpc_parser_t* Expr;
mpc_parser_t* Lisp;

void init_parser();
mpc_ast_t* parse(const char* input);
void del_ast(mpc_ast_t*  ast);

#endif


