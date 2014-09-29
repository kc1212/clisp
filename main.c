#include <stdio.h>
#include <stdlib.h>
#include <editline/readline.h>

#include "common.h"
#include "parser.h"
#include "eval.h"
#include "envi.h"

int main(void)
{
	int ret = 0;
	// TODO make this optional, i.e. parse argc argv
	logfp = fopen(LOGFILE, "w+");
	if (NULL == logfp)
	{
		log_err("freopen failed on %s", LOGFILE);
		return 1;
	}

	puts("toylist v0.1");
	ret = init_parser();
	if ( 0 != ret )
		goto cleanup;

	lenv* e = lenv_new();
	if (NULL == e)
	{
		ret = 1;
		goto cleanup;
	}

	ret = init_env(e);
	if ( 0 != ret )
		goto cleanup_env;

	for (;;)
	{
		char* input = readline("->> ");
		add_history(input);

		int command = colon_commands(input, e);
		if (COLON_CONTINUE == command) {
			free(input);
			continue;
		}
		else if (COLON_BREAK == command) {
			free(input);
			break;
		}

		mpc_ast_t* ast = parse(input);

		if (!ast)
		{
			free(input);
			continue;
		}

		lval* x = eval(e, ast_to_lval(ast));
		lval_println(x);
		lval_del(x);

		mpc_ast_delete(ast);
		free(input);
	}

cleanup_env:
	lenv_del(e);
cleanup:
	clear_history();
	mpc_cleanup(7, Long, Double, Symbol, Qexpr, Sexpr, Expr, Lisp);
	fclose(logfp);
	return ret;
}



