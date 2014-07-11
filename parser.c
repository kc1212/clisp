
#include "parser.h"

void init_parser()
{
	Number		= mpc_new("number");
	Operator	= mpc_new("operator");
	Expr		= mpc_new("expr");
	Lisp		= mpc_new("lisp");

	mpca_lang(MPCA_LANG_DEFAULT,
		"\
		number		: /-?[0-9]+/ ;\
		operator	: '+' | '-' | '*' | '/' ;\
		expr		: <number> | '(' <operator> <expr>+ ')' ;\
		lisp		: /^/ <operator> <expr>+ /$/ ;\
		",
		Number, Operator, Expr, Lisp);
}

void parse(const char* input)
{
	mpc_result_t r;
	if (mpc_parse("<stdin>", input, Lisp, &r)) {
		mpc_ast_print(r.output);
		mpc_ast_delete(r.output);
	} else {
		mpc_err_print(r.error);
		mpc_err_delete(r.error);
	}
}



