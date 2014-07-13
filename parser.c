
#include <string.h>
#include <ctype.h>

#include "parser.h"

static int all_isspace(const char* input)
{
	while (isspace(*input))
		input++;

	if (*input== '\0')
		return 1;

	return 0;
}

void init_parser()
{
	Number		= mpc_new("number");
	Operator	= mpc_new("operator");
	Expr		= mpc_new("expr");
	Lisp		= mpc_new("lisp");

	mpca_lang(MPCA_LANG_DEFAULT,
		"\
		number		: /-?[0-9]+/ ;\
		operator	: '+' | '-' | '*' | '/' | '%' | '^' | \"min\" | \"max\" ;\
		expr		: <number> | '(' <operator> <expr>+ ')' ;\
		lisp		: /^/ <operator> <expr>+ /$/ ;\
		",
		Number, Operator, Expr, Lisp);
}

// abstract syntax tree
mpc_ast_t* parse(const char* input)
{
	mpc_result_t r;
	if (mpc_parse("<stdin>", input, Lisp, &r))
	{
		mpc_ast_print(r.output);
		return r.output;
	}
	else if (all_isspace(input))
	{
		mpc_err_delete(r.error);
	}
	else
	{
		mpc_err_print(r.error);
		mpc_err_delete(r.error);
	}
	return NULL;
}



