#ifndef _VECTORIZE_ODE_H
#define _VECTORIZE_ODE_H 1

#include "lua.h"

#include "vector.h"

#define VECTORIZE_ODE_INTMETHOD_UV (1)
#define VECTORIZE_ODE_INTEGRAND_UV (2)
#define VECTORIZE_ODE_STATE_UV (3)

extern const char ode_mt_name[];

typedef struct ODE {
  lua_Integer curiter;
  lua_Number stepsize;
} ODE;

#endif /* ifndef _VECTORIZE_ODE_H */
