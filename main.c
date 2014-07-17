#include <stdio.h>
#include <stdlib.h>
#include <editline/readline.h>

#include "parser.h"
#include "eval.h"
#include "common.h"

int main(void)
{
	// TODO make this optional, i.e. parse argc argv
	FILE* fp = freopen(LOGFILE, "w+", stderr);
	if (NULL == fp)
	{
		log_err("freopen failed on %s", LOGFILE);
		return 1;
	}

	puts("toylist v0.0");
	init_parser();

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

		lval_println(eval(ast));

		mpc_ast_delete(ast);
		free(input);
	}

	clear_history();
	mpc_cleanup(5, Integer, Float, Operator, Expr, Lisp);
	fclose(fp);
	return 0;
}



