#ifndef _VECTORIZE_VEC_H
#define _VECTORIZE_VEC_H 1

typedef struct Vector {
  lua_Number *values;
  lua_Integer len;
} Vector;

#endif /* ifndef _VECTORIZE_VEC_H */