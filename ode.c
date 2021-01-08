#include "lauxlib.h"
#include "lua.h"
#include <errno.h>
#include <math.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "ode.h"

/*
args:
  integration method: callable (uncheckable at creation)
  integrand: callable (uncheckable at creation)
  initstate: number | {number} | Vector
  stepsize: number
  */
int ode_custom_init(lua_State *L) {
  if (lua_isnumber(L, 3)) {
    // Get vectorize lib
    lua_getglobal(L, "require");
    lua_pushstring(L, "vectorize");
    lua_call(L, 1, 1);

    // Create single-element vector
    lua_pushinteger(L, 1);
    lua_call(L, 1, 1);

    // Set its element to 3rd arg
    lua_pushvalue(L, 3);
    lua_seti(L, -2, 1);

    // Make sure it worked
    if (!lua_isuserdata(L, -1)) {
      return luaL_error(L, "Could not create vector (unknown error)");
    }

    // Put converted value back in its place
    lua_replace(L, 3);

  } else if (lua_istable(L, 3)) {
    // Get vectorize lib
    lua_getglobal(L, "require");
    lua_pushstring(L, "vectorize");
    lua_call(L, 1, 1);

    // Create vector from table arg
    lua_pushvalue(L, 3);
    lua_call(L, 1, 1);

    // Make sure it worked
    if (!lua_isuserdata(L, -1)) {
      return luaL_error(L, "Could not create vector (unknown error)");
    }

    // Put converted value back in its place
    lua_replace(L, 3);

  } else if (!luaL_testudata(L, 3, vector_mt_name)) {
    return luaL_error(
      L,
      "Expected a vector, a number, or an array-like table, got %s",
      lua_typename(L, lua_type(L, 3)));
  }

  lua_Number stepsize = luaL_checknumber(L, 4);

  ODE *ode = lua_newuserdatauv(L, sizeof(ODE), 3);
  luaL_setmetatable(L, ode_mt_name);
  ode->curiter = 0;
  ode->stepsize = stepsize;

  lua_pushvalue(L, 1);
  lua_setiuservalue(L, -2, VECTORIZE_ODE_INTMETHOD_UV);

  lua_pushvalue(L, 2);
  lua_setiuservalue(L, -2, VECTORIZE_ODE_INTEGRAND_UV);

  lua_pushvalue(L, 3);
  lua_setiuservalue(L, -2, VECTORIZE_ODE_STATE_UV);

  return 1;
}

/* int ode_rk4_step(lua_State *L); */
/* int ode_rk4_init(lua_State *L); */
/*  */
/* int ode_euler_step(lua_State *L); */
/* int ode_euler_init(lua_State *L); */
/*  */
/* int ode_heun_step(lua_State *L); */
/* int ode_heun_init(lua_State *L); */
/*  */
/* int ode_iterstep(lua_State *L); */
/* int ode_state(lua_State *L); */
/*  */
/* int ode__gc(lua_State *L); */

/* int ode_rk4_init(lua_State *L) { */
/* } */

int ode_state(lua_State *L) {
  luaL_checkudata(L, 1, ode_mt_name);
  lua_getiuservalue(L, -1, VECTORIZE_ODE_STATE_UV);
  return 1;
}

const struct luaL_Reg ode_functions[] = {
  {"custom", &ode_custom_init}, {"state", &ode_state}, {NULL, NULL}};

static void create_ode_metatable(lua_State *L, int libstackidx) {
  libstackidx = lua_absindex(L, libstackidx);

  luaL_newmetatable(L, ode_mt_name);

  lua_pushvalue(L, libstackidx);
  lua_setfield(L, -2, "__index");

  lua_pop(L, 1);
}

extern int luaopen_vectorize_ode(lua_State *L) {
  /* create_lib_metatable(L); */
  // TODO use require here for vector instead of linking vectorize.o

  luaL_newlib(L, ode_functions);
  create_ode_metatable(L, -1);

  return 1;
}
