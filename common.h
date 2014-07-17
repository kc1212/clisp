#ifndef COMMON_H_
#define COMMON_H_

#include <stdio.h>
#include "mpc/mpc.h"

#define LOGFILE "logs/logs.txt"

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

// error macros, taken from zed shaw
#define clean_errno() (errno == 0 ? "None" : strerror(errno))
#define log_err(M, ...) fprintf(stderr, "[ERROR] (%s:%d: errno: %s) " M "\n", __FILE__ , __LINE__ , clean_errno() , __VA_ARGS__)
#define log_warn(M, ...) fprintf(stderr, "[WARN] (%s:%d: errno: %s) " M "\n", __FILE__, __LINE__, clean_errno(), __VA_ARGS__)
#define log_info(M, ...) fprintf(stderr, "[INFO] (%s:%d) " M "\n", __FILE__, __LINE__, __VA_ARGS__)
#define debug(M, ...) fprintf(stderr, "DEBUG %s:%d: " M "\n", __FILE__ , __LINE__ , __VA_ARGS__)

enum {LVAL_NUM, LVAL_ERR};
enum {LERR_DIV_ZERO, LERR_BAD_OP, LERR_BAD_NUM, LERR_OTHER};

typedef struct
{
	int type;
	int err;
	long num;
// 	union
// 	{
// 		long l;
// 		double f;
// 	} data;
} lval;

lval lval_num(long x);
lval lval_err(int x);
void lval_print(lval v);
void lval_println(lval v);

#endif


