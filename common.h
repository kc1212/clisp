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

enum {LVAL_LNG, LVAL_DBL, LVAL_ERR};
enum {LERR_DIV_ZERO, LERR_BAD_OP, LERR_BAD_NUM, LERR_OTHER};

typedef struct
{
	int type;
	int err;
	union
	{
		int64_t lng;
		double dbl;
	} data;
} lval;

FILE* logfp;
FILE* errfp;

lval lval_long(int64_t x);
lval lval_double(double x);
lval lval_err(int x);
void lval_print(lval v);
void lval_println(lval v);

#endif


