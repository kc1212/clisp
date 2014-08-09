
#include <string.h>
#include <ctype.h>

#include "parser.h"
#include "common.h"

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
	Long		= mpc_new("long");
	Double		= mpc_new("double");
	Symbol		= mpc_new("symbol");
	Sexpr		= mpc_new("sexpr");
	Qexpr		= mpc_new("qexpr");
	Expr		= mpc_new("expr");
	Lisp		= mpc_new("lisp");

	mpca_lang(MPCA_LANG_DEFAULT,
		"long		: /-?\\d+/ ;"
		"double		: /-?\\d*\\.\\d+|-?\\d+\\./ ;"
		"symbol		: /[a-zA-Z0-9_+\\-*\\/\\\\=<>!&\\^]+/ ; "
		"sexpr		: '(' <expr>* ')' ;"
		"qexpr		: '{' <expr>* '}' ;"
		"expr		: <double> | <long> | <symbol> | <sexpr> | <qexpr> ;"
		"lisp		: /^/ <expr>* /$/ ;",
		Long, Double, Symbol, Sexpr, Qexpr, Expr, Lisp);
}

// abstract syntax tree
mpc_ast_t* parse(const char* input)
{
	if (all_isspace(input))
	{
		log_info_to(logfp, "Parsing failed: %s (empty) ", input);
		return NULL;
	}

	mpc_result_t r;
	if (mpc_parse("<stdin>", input, Lisp, &r))
	{
		log_info_to(logfp, "Parsing successful: %s", input);
		mpc_ast_print_to(r.output, logfp);
		return r.output;
	}

	log_info_to(logfp, "Parsing failed: %s", input);
	mpc_err_print_to(r.error, stderr);
	mpc_err_delete(r.error);
	return NULL;
}



