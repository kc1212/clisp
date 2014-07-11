#include <stdio.h>
#include <stdlib.h>
#include <editline/readline.h>

#include "parser.h"

int main(void)
{
	init_parser();
	puts("clisp v0.0");

	for (;;)
	{
		char* input = readline("clisp> ");
		add_history(input);

		parse(input);
		free(input);
	}

	clear_history();
	mpc_cleanup(4, Number, Operator, Expr, Lispy);
	return 0;
}



