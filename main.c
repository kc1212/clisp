#include <stdio.h>
#include <stdlib.h>
#include <editline/readline.h>

#include "common.h"
#include "parser.h"
#include "eval.h"

extern const char LERR_DIV_ZERO[] = "Error: Division By Zero!";
extern const char LERR_BAD_OP[] = "Error: Invalid Operator!";
extern const char LERR_BAD_NUM[] = "Error: Invalid Number!";
extern const char LERR_OTHER[] = "Critical Error!";

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

		lval* x = lval_read(ast);
		lval_println(x);
		lval_del(x);

		mpc_ast_delete(ast);
		free(input);
	}

	clear_history();
	mpc_cleanup(6, Long, Double, Symbol, Sexpr, Expr, Lisp);
	fclose(logfp);
	return 0;
}



