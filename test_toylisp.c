
#include <assert.h>

#include "mpc/mpc.h"

#include "common.h"
#include "eval.h"
#include "parser.h"

#define run_test(fn_name)\
	printf("%s", #fn_name);\
	fprintf(logfp, "\n%s\n", #fn_name);\
	fprintf(stderr, "\n%s\n", #fn_name);\
	fn_name();\
	for (size_t i = 0; i < 26 - strlen(#fn_name); i++) printf(".");\
	printf("OK\n");

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

void test_lval_long()
{
	lval* v = lval_long(123);
	assert(LVAL_LNG == v->type);
	assert(123 == v->data.lng);
	lval_delete(v);
}

void test_lval_err()
{
	lval* v = lval_err(123);
	assert(LVAL_ERR == v->type);
	assert(123 == v->err);
	lval_delete(v);
}

void test_ast_type()
{
	mpc_ast_t* ast = parse("+ 1.1 1");
	assert(6 == ast_size(ast));
	assert(strstr(ast->children[2]->tag, "double"));
	assert(strstr(ast->children[3]->tag, "long"));
	mpc_ast_delete(ast);
}

void test_ast_failure()
{
	mpc_ast_t* ast = parse("+ 4 (");
	assert(NULL == ast);
	mpc_ast_delete(ast);
}

void test_eval_arithmetic()
{
	mpc_ast_t* ast = parse("+ 1 2 (- 20 23) (* 3 7) (/ 9 (/ 14 2))");
	assert(22 == eval(ast).data.lng);
	mpc_ast_delete(ast);
}

void test_eval_arithmetic_dbl()
{
	mpc_ast_t* ast = parse("+ 1.5 1.5 (- 20. 23) (* 3. 7) (/ 9 (/ 6.0 2))");
	assert(24. == eval(ast).data.dbl);
	mpc_ast_delete(ast);
}

void test_eval_pow()
{
	mpc_ast_t* ast = parse("^ 2 2 2 2 2");
	assert(65536 == eval(ast).data.lng);
	mpc_ast_delete(ast);
}

void test_eval_pow_dbl()
{
	mpc_ast_t* ast = parse("^ 2 .5");
	assert(sqrt(2.0) == eval(ast).data.dbl);
	mpc_ast_delete(ast);
}

void test_eval_maxmin()
{
	mpc_ast_t* ast = parse("max 1 2 3 4 (min 5 6 7 8)");
	assert(5 == eval(ast).data.lng);
	mpc_ast_delete(ast);
}

void test_eval_maxmin_dbl()
{
	mpc_ast_t* ast = parse("max 1 2 3.3 4.4 (min 5.5 6 7 8)");
	assert(5.5 == eval(ast).data.dbl);
	mpc_ast_delete(ast);
}

int main(void)
{
	logfp = fopen(LOGFILE, "w+");
	if (NULL == logfp)
	{
		log_err("freopen failed on %s", LOGFILE);
		return 1;
	}

	errfp = freopen(ERRFILE, "w+", stderr);
	if (NULL == errfp)
	{
		log_err("freopen failed on %s", ERRFILE);
		return 1;
	}

	init_parser();
	run_test(test_lval_long);
	run_test(test_lval_err);
	run_test(test_ast_type);
	run_test(test_ast_failure);
	run_test(test_eval_arithmetic);
	run_test(test_eval_arithmetic_dbl);
	run_test(test_eval_pow);
	run_test(test_eval_pow_dbl);
	run_test(test_eval_maxmin);
	run_test(test_eval_maxmin_dbl);
	printf("Done\n");

	fclose(logfp);
	fclose(errfp);
	return 0;
}



