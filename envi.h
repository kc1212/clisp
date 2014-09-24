#ifndef ENVI_H_
#define ENVI_H_

#include "common.h"

lenv* lenv_new(void);
void lenv_del(lenv* e);
lval* lenv_get(lenv* e, lval* k);
int lenv_put(lenv* e, lval* k, lval* v);

#endif



