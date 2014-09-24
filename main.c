#include <stdio.h>
#include <stdlib.h>
#include <editline/readline.h>

#include "common.h"
#include "parser.h"
#include "eval.h"

int main(void)
{
	// TODO make this optional, i.e. parse argc argv
	logfp = fopen(LOGFILE, "w+");
	if (NULL == logfp)
	{
		log_err("freopen failed on %s", LOGFILE);
		return 1;
	}

	puts("toylist v0.0");
	init_parser();
	lenv* e = lenv_new();

	for (;;)
	{
		char* input = readline("->> ");
		add_history(input);

		if (!strcmp(input, "quit"))
		{
			free(input);
			break;
		}

		mpc_ast_t* ast = parse(input);

		if (!ast)
		{
			free(input);
			continue;
		}

		lval* x = eval(ast_to_lval(ast));
		lval_println(x);
		lval_del(x);

		mpc_ast_delete(ast);
		free(input);
	}

	clear_history();
	mpc_cleanup(7, Long, Double, Symbol, Qexpr, Sexpr, Expr, Lisp);
	fclose(logfp);
	return 0;
}



