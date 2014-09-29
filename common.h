#ifndef COMMON_H_
#define COMMON_H_

#include <stdio.h>
#include <stdint.h>
#include "mpc/mpc.h"

#define LOGFILE "logs/logs.txt"
#define ERRFILE "logs/logs.err.txt"

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

// error macros, taken from zed shaw
#define clean_errno() (errno == 0 ? "None" : strerror(errno))
#define log_err_to(fd, M, ...) fprintf(fd, "[ERROR] (%s:%d: errno: %s) " M "\n", __FILE__ , __LINE__ , clean_errno() , __VA_ARGS__)
#define log_warn_to(fd, M, ...) fprintf(fd, "[WARN] (%s:%d: errno: %s) " M "\n", __FILE__, __LINE__, clean_errno(), __VA_ARGS__)
#define log_info_to(fd, M, ...) fprintf(fd, "[INFO] (%s:%d) " M "\n", __FILE__, __LINE__, __VA_ARGS__)
#define debug_to(fd, M, ...) fprintf(fd, "DEBUG %s:%d: " M "\n", __FILE__ , __LINE__ , __VA_ARGS__)

#define log_err(M, ...) log_err_to(stderr, M, __VA_ARGS__)
#define log_warn(M, ...) log_warn_to(stderr, M, __VA_ARGS__)
#define log_info(M, ...) log_info_to(stderr, M, __VA_ARGS__)
#define debug(M, ...) debug_to(stderr, M, __VA_ARGS__)

// constants
enum LVAL_TYPES
{
	LVAL_LNG,
	LVAL_DBL,
	LVAL_SYM,
	LVAL_FUN,
	LVAL_SEXPR,
	LVAL_QEXPR,
	LVAL_ERR
};

enum LVAL_ERRS
{
	LERR_DIV_ZERO,
	LERR_BAD_OP,
	LERR_BAD_NUM,
	LERR_BAD_SEXPR_START,
	LERR_BAD_FUNCTION,
	LERR_BAD_SYMBOL,
	LERR_TOO_MANY_ARGS,
	LERR_BAD_ARGS_COUNT,
	LERR_BAD_TYPE,
	LERR_EMPTY,
	LERR_OTHER
};

// TODO, need better way to output function names in error messages
static const char* const err_strings[] =
{
	"Error: Division By Zero!\n",
	"Error: Invalid Operator!\n",
	"Error: Invalid Number!\n",
	"S-expression Does not start with symbol!\n",
	"Unknown Function!\n",
	"Unknown Symbol!\n",
	"Function passed too many arguments!\n",
	"Function passed wrong number of arguments!\n",
	"Function passed incorrect type!\n",
	"Function passed {}!\n",
	"Critical Error!\n"
};

enum COLON_COMMAND_ACTION
{
	COLON_BREAK,
	COLON_CONTINUE,
	COLON_OTHER
};

// forward declaration
struct lval;
struct lenv;
typedef struct lval lval;
typedef struct lenv lenv;

// type declarations
typedef lval*(*lbuiltin)(lenv*, lval*);

struct lval
{
	int type;
	int count; // of cells
	int err;
	union
	{
		int64_t lng;
		double dbl;
	} data;
	char* sym; // op
	struct lval** cell;
	lbuiltin fun;
};

struct lenv
{
	int count;
	char** syms;
	lval** vals;
	int debug;
};

// globals variables
FILE* logfp;
FILE* errfp;

// global functions
void lval_del(lval* v);
void lval_println(lval* v);
lval* lval_copy(lval* v);
lval* lval_err(enum LVAL_ERRS e);
int colon_commands(const char* input, lenv* e);

// TODO: this function sometimes returns -1 if the output is truncated,
// this need to be fixed so be consistent with snprintf
int lval_snprintln(lval *v, char* str, const long n);

#endif


