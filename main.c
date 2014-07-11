#include <stdio.h>
#include <stdlib.h>

#include <editline/readline.h>

#include "mpc/mpc.h"
#include "parser.h"

int main(void)
{
	puts("lisp v0.0");

	for (;;)
	{
		char* input = readline("lisp> ");
		add_history(input);
		printf("u typed: %s\n", input);

		free(input);
	}
	clear_history();
	return 0;
}



