#ifndef _VECTORIZE_VEC_H
#define _VECTORIZE_VEC_H 1

#include "lua.h"

const char vector_mt_name[] = "vector";

typedef struct Vector {
  lua_Number *values;
  lua_Integer len;
} Vector;

int vec_new(lua_State *L);
int vec_from(lua_State *L);

#endif /* ifndef _VECTORIZE_VEC_H */
