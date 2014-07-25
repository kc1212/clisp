
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

void test_ast_type()
{
	mpc_ast_t* ast = parse("+ 1.1 1");
	assert(6 == ast_size(ast));
	assert(strstr(ast->children[1]->tag, "symbol"));
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
	lval* v = lval_eval(lval_read(ast));
	assert(LVAL_LNG == v->type);
	assert(22 == v->data.lng);
	lval_del(v);
	mpc_ast_delete(ast);
}

void test_eval_arithmetic_dbl()
{
	mpc_ast_t* ast = parse("+ 1.5 1.5 (- 20. 23) (* 3. 7) (/ 9 (/ 6.0 2))");
	lval* v = lval_eval(lval_read(ast));
	assert(LVAL_DBL == v->type);
	assert(24. == v->data.dbl);
	lval_del(v);
	mpc_ast_delete(ast);
}

void test_eval_pow()
{
	mpc_ast_t* ast = parse("^ 2 2 2 2 2");
	lval* v = lval_eval(lval_read(ast));
	assert(LVAL_LNG == v->type);
	assert(65536 == v->data.lng);
	lval_del(v);
	mpc_ast_delete(ast);
}

void test_eval_pow_dbl()
{
	mpc_ast_t* ast = parse("^ 2 .5");
	lval* v = lval_eval(lval_read(ast));
	assert(LVAL_DBL == v->type);
	assert(sqrt(2.0) == v->data.dbl);
	lval_del(v);
	mpc_ast_delete(ast);
}

void test_eval_maxmin()
{
	mpc_ast_t* ast = parse("max 1 2 3 4 (min 5 6 7 8)");
	lval* v = lval_eval(lval_read(ast));
	assert(LVAL_LNG == v->type);
	assert(5 == v->data.lng);
	lval_del(v);
	mpc_ast_delete(ast);
}

void test_eval_maxmin_dbl()
{
	mpc_ast_t* ast = parse("max 1 2 3.3 4.4 (min 5.5 6 7 8)");
	lval* v = lval_eval(lval_read(ast));
	assert(LVAL_DBL == v->type);
	assert(5.5 == v->data.dbl);
	lval_del(v);
	mpc_ast_delete(ast);
}

void test_non_number()
{
	mpc_ast_t* ast = parse("( / ( ) )");
	lval* v = lval_eval(lval_read(ast));
	assert(LVAL_ERR == v->type);
	assert(LERR_BAD_NUM == v->err);
	lval_del(v);
	mpc_ast_delete(ast);
}

void test_bad_sexpr_start()
{
	mpc_ast_t* ast = parse("( 1 () )");
	lval* v = lval_eval(lval_read(ast));
	assert(LVAL_ERR == v->type);
	assert(LERR_BAD_SEXPR_START == v->err);
	lval_del(v);
	mpc_ast_delete(ast);
}

void test_div_zero()
{
	mpc_ast_t* ast = parse("(/ 1 0 )");
	lval* v = lval_eval(lval_read(ast));
	assert(LVAL_ERR == v->type);
	assert(LERR_DIV_ZERO == v->err);
	lval_del(v);
	mpc_ast_delete(ast);
}

void test_div_zero_dbl()
{
	mpc_ast_t* ast = parse("(/ 1 0.0000000000000001)");
	lval* v = lval_eval(lval_read(ast));
	assert(LVAL_ERR == v->type);
	assert(LERR_DIV_ZERO == v->err);
	lval_del(v);
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
	run_test(test_ast_type);
	run_test(test_ast_failure);
	run_test(test_eval_arithmetic);
	run_test(test_eval_arithmetic_dbl);
	run_test(test_eval_pow);
	run_test(test_eval_pow_dbl);
	run_test(test_eval_maxmin);
	run_test(test_eval_maxmin_dbl);
	run_test(test_non_number);
	run_test(test_bad_sexpr_start);
	run_test(test_div_zero);
	run_test(test_div_zero_dbl);
	printf("Done\n");

	fclose(logfp);
	fclose(errfp);
	return 0;
}



