
#include <assert.h>

#include "mpc/mpc.h"

#include "common.h"
#include "eval.h"
#include "parser.h"

// TODO logging is weird, need to be improved
#define RUN_TEST(fn_name)\
	printf("%s", #fn_name);\
	fprintf(logfp, "\n%s\n", #fn_name);\
	fprintf(stderr, "\n%s\n", #fn_name);\
	for (size_t i = 0; i < 26 - strlen(#fn_name); i++) printf(".");\
	if (0 == fn_name()) { printf("OK\n"); } \
	else { printf("FAIL!\n"); return 1; } \

#define TEST_ASSERT(expr) \
	if (!(expr)) { \
		printf("\n\tTEST_ASSERT failed on expression \"%s\"", #expr); \
		printf("\n\tin function \"%s\", on line %d\n", __func__, __LINE__); \
		return 1; \
	} \

#define STARTUP(AST, V, STR) \
	mpc_ast_t* AST = parse(STR); \
	lval* V = eval(ast_to_lval(AST))

#define TEARDOWN(AST, V) \
	lval_del(V); mpc_ast_delete(AST)


int ast_size(mpc_ast_t* ast)
{
	if (!ast) { return -1; }

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

// TODO, add tests for lval count and sexpr count

int test_ast_type()
{
	mpc_ast_t* ast = parse("+ 1.1 1");
	TEST_ASSERT(6 == ast_size(ast));
	TEST_ASSERT(strstr(ast->children[1]->tag, "symbol"));
	TEST_ASSERT(strstr(ast->children[2]->tag, "double"));
	TEST_ASSERT(strstr(ast->children[3]->tag, "long"));
	mpc_ast_delete(ast);
	return 0;
}

int test_ast_failure()
{
	mpc_ast_t* ast = parse("+ 4 (");
	TEST_ASSERT(NULL == ast);
	mpc_ast_delete(ast);
	return 0;
}

int test_eval_arithmetic()
{
	STARTUP(ast, v, "+ 1 2 (- 20 23) (* 3 7) (/ 9 (/ 14 2))");
	TEST_ASSERT(LVAL_LNG == v->type);
	TEST_ASSERT(22 == v->data.lng);
	TEARDOWN(ast, v);
	return 0;
}

int test_eval_arithmetic_dbl()
{
	STARTUP(ast, v, "+ 1.5 1.5 (- 20. 23) (* 3. 7) (/ 9 (/ 6.0 2))");
	TEST_ASSERT(LVAL_DBL == v->type);
	TEST_ASSERT(24. == v->data.dbl);
	TEARDOWN(ast, v);
	return 0;
}

int test_eval_pow()
{
	STARTUP(ast, v, "^ 2 2 2 2 2");
	TEST_ASSERT(LVAL_LNG == v->type);
	TEST_ASSERT(65536 == v->data.lng);
	TEARDOWN(ast, v);
	return 0;
}

int test_eval_pow_dbl()
{
	STARTUP(ast, v, "^ 2 .5");
	TEST_ASSERT(LVAL_DBL == v->type);
	TEST_ASSERT(sqrt(2.0) == v->data.dbl);
	TEARDOWN(ast, v);
	return 0;
}

int test_eval_maxmin()
{
	STARTUP(ast, v, "max 1 2 3 4 (min 5 6 7 8)");
	TEST_ASSERT(LVAL_LNG == v->type);
	TEST_ASSERT(5 == v->data.lng);
	TEARDOWN(ast, v);
	return 0;
}

int test_eval_maxmin_dbl()
{
	STARTUP(ast, v, "max 1 2 3.3 4.4 (min 5.5 6 7 8)");
	TEST_ASSERT(LVAL_DBL == v->type);
	TEST_ASSERT(5.5 == v->data.dbl);
	TEARDOWN(ast, v);
	return 0;
}

int test_non_number()
{
	STARTUP(ast, v, "( / ( ) )");
	TEST_ASSERT(LVAL_ERR == v->type);
	TEST_ASSERT(LERR_BAD_NUM == v->err);
	TEARDOWN(ast, v);
	return 0;
}

int test_bad_sexpr_start()
{
	STARTUP(ast, v, "( 1 () )");
	TEST_ASSERT(LVAL_ERR == v->type);
	TEST_ASSERT(LERR_BAD_SEXPR_START == v->err);
	TEARDOWN(ast, v);
	return 0;
}

int test_div_zero()
{
	STARTUP(ast, v, "(/ 1 0 )");
	TEST_ASSERT(LVAL_ERR == v->type);
	TEST_ASSERT(LERR_DIV_ZERO == v->err);
	TEARDOWN(ast, v);
	return 0;
}

int test_div_zero_dbl()
{
	STARTUP(ast, v, "(/ 1 0.0000000000000001)");
	TEST_ASSERT(LVAL_ERR == v->type);
	TEST_ASSERT(LERR_DIV_ZERO == v->err);
	TEARDOWN(ast, v);
	return 0;
}

int test_empty_input()
{
	mpc_ast_t* ast = parse("  ");
	TEST_ASSERT(NULL == ast);
	mpc_ast_delete(ast);
	return 0;
}

int test_snprint_exprs_good()
{
	const int N = 14;
	char output[N];

	mpc_ast_t* ast = parse(" { (+ 1 2 3 ) }");
	lval* v = ast_to_lval(ast);

	int ret = lval_snprintln(v, output, N);
	TEST_ASSERT(0 <= ret);
	TEST_ASSERT(0 == strcmp("({(+ 1 2 3)})", output));
	TEST_ASSERT('\0' == output[N-1]);

	lval_del(v);
	mpc_ast_delete(ast);
	return 0;

}

int test_snprint_exprs_bad()
{
	const int N = 13; // 12 characters + '\0'
	char output[N+1];
	memset(output, 'z', sizeof(output));

	mpc_ast_t* ast = parse(" { (+ 1 2 3 ) }");
	lval* v = ast_to_lval(ast);

	int ret = lval_snprintln(v, output, N);
	TEST_ASSERT(-1 == ret);
	TEST_ASSERT('z' == output[N]); // make sure last bit is not set

	TEARDOWN(ast, v);

	return 0;
}

int test_qexprs()
{
	const int N = 16;
	char output[N];

	STARTUP(ast, v, "list 1 2 3 4");
	TEST_ASSERT(lval_snprintln(v, output, N));
	TEST_ASSERT(0 == strncmp("{1 2 3 4}", output, N));
	TEARDOWN(ast, v);
	return 0;
}

int run_tests(void)
{
	RUN_TEST(test_ast_type);
	RUN_TEST(test_ast_failure);
	RUN_TEST(test_eval_arithmetic);
	RUN_TEST(test_eval_arithmetic_dbl);
	RUN_TEST(test_eval_pow);
	RUN_TEST(test_eval_pow_dbl);
	RUN_TEST(test_eval_maxmin);
	RUN_TEST(test_eval_maxmin_dbl);
	RUN_TEST(test_non_number);
	RUN_TEST(test_bad_sexpr_start);
	RUN_TEST(test_div_zero);
	RUN_TEST(test_div_zero_dbl);
	RUN_TEST(test_empty_input);
	RUN_TEST(test_snprint_exprs_good);
	RUN_TEST(test_snprint_exprs_bad);
	RUN_TEST(test_qexprs);
	printf("Done\n");
	return 0;
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

	int ret = run_tests();

	fclose(logfp);
	fclose(errfp);
	mpc_cleanup(7, Long, Double, Symbol, Qexpr, Sexpr, Expr, Lisp);
	return ret;
}



