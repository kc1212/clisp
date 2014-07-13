
#include <assert.h>
#include "mpc/mpc.h"

#include "common.h"
#include "eval.h"
#include "parser.h"

#define run_test(fn_name)\
	printf("%s\n", #fn_name);\
	fn_name();

int ast_size(mpc_ast_t* ast)
{
	if (!ast)
		return -1;

	int ctr = 1;
	if (ast->children_num > 0) // if child_num is 0, 1 is returned
	{
		for (int i = 0; i < ast->children_num; i++)
		{
			ctr += ast_size(ast->children[i]);
		}
	}
	return ctr;
}

void test_ast_size_5()
{
	mpc_ast_t* ast = parse("+ 1");
	assert(5 == ast_size(ast));
}

int main(void)
{
	init_parser();
	run_test(test_ast_size_5);
	printf("OK\n");
	return 0;
}



