#ifndef _VECTORIZE_ODE_H
#define _VECTORIZE_ODE_H 1

typedef struct ODE {
  Vector *state;
  lua_CFunction deriv;

  lua_Integer curiter;
  lua_Number stepsize;
} ODE;

int ode_new(lua_State *L);
int ode_step(lua_State *L);
int ode_iterstep(lua_State *L);
int ode_state(lua_State *L);

int ode__gc(lua_State *L);

void create_ode_metatable(lua_State *L, int libstackidx);

#endif /* ifndef _VECTORIZE_ODE_H */
