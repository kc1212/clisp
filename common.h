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
#define debug_to(fd, M, ...) fprintf(fd, "[DEBUG] (%s:%d %s): " M "\n", __FILE__ , __LINE__ , __func__, __VA_ARGS__)

#define log_err(M, ...) log_err_to(stderr, M, __VA_ARGS__)
#define log_warn(M, ...) log_warn_to(stderr, M, __VA_ARGS__)
#define log_info(M, ...) log_info_to(stderr, M, __VA_ARGS__)
#define debug(M, ...) debug_to(stderr, M, __VA_ARGS__)

#define GENERATE_ENUM(ENUM) ENUM,
#define GENERATE_STRING(STRING) #STRING,

// generate lval types and strings
#define FOREACH_LVAL_TYPE(TYPE) \
	TYPE(LVAL_LNG) \
	TYPE(LVAL_DBL) \
	TYPE(LVAL_SYM) \
	TYPE(LVAL_FUN) \
	TYPE(LVAL_SEXPR) \
	TYPE(LVAL_QEXPR) \
	TYPE(LVAL_ERR) \

enum LVAL_TYPES { FOREACH_LVAL_TYPE(GENERATE_ENUM) };
// static const char* LVAL_TYPE_STRINGS[] = { FOREACH_LVAL_TYPE(GENERATE_STRING) };

// generate lval errors and strings
#define FOREACH_LVAL_ERR(TYPE) \
	TYPE(LERR_DIV_ZERO) \
	TYPE(LERR_BAD_OP) \
	TYPE(LERR_BAD_NUM) \
	TYPE(LERR_BAD_SEXPR_START) \
	TYPE(LERR_BAD_FUNCTION) \
	TYPE(LERR_BAD_SYMBOL) \
	TYPE(LERR_TOO_MANY_ARGS) \
	TYPE(LERR_BAD_ARGS_COUNT) \
	TYPE(LERR_BAD_TYPE) \
	TYPE(LERR_EMPTY) \
	TYPE(LERR_OTHER) \

enum LVAL_ERRS { FOREACH_LVAL_ERR(GENERATE_ENUM) };
// static const char* LVAL_ERR_STRINGS[] = { FOREACH_LVAL_ERR(GENERATE_STRING) };

// TODO, need better way to output function names in error messages
static const char* const LVAL_ERR_DESCRIPTIONS[] =
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
	lval** cell;

	lbuiltin builtin;
	lenv* env;
	lval* formals;
	lval* body;
};

struct lenv
{
	int count;
	char** syms;
	lval** vals;
	lenv* par; // parent
	int debug;
};

// globals variables
FILE* logfp;
FILE* errfp;

// lval global functions
void lval_del(lval* v);
void lval_println(lval* v);
lval* lval_copy(lval* v);
lval* lval_err(enum LVAL_ERRS e);

// lenv global functions
lenv* lenv_new(void);
void lenv_del(lenv* e);
lval* lenv_get(lenv* e, lval* k);
int lenv_put(lenv* e, lval* k, lval* v);
int lenv_def(lenv* e, lval* k, lval* v);
lenv* lenv_copy(lenv* e);
int lenv_print(lenv* e);

// others
int colon_commands(const char* input, lenv* e);

// TODO: this function sometimes returns -1 if the output is truncated,
// this need to be fixed so be consistent with snprintf
int lval_snprintln(lval *v, char* str, const long n);

#endif


