#include "lauxlib.h"
#include "lua.h"
#include <errno.h>
#include <math.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "vector.h"

/* const char ode_lib_mt_name[] = "liblua-vectorize-ode"; */
/* const char ode_mt_name[] = "ODE"; */

/* static void create_lib_metatable(lua_State *L) { */
/*   luaL_newmetatable(L, ode_lib_mt_name); */
/*   lua_pop(L, 1); */
/* } */

int ode_test(lua_State *L) {
  lua_pushnumber(L, 1);
  return 1;
}

const struct luaL_Reg ode_functions[] = {{"test", &ode_test}, {NULL, NULL}};

extern int luaopen_vectorize_ode(lua_State *L) {
  /* create_lib_metatable(L); */

  luaL_newlib(L, ode_functions);
  return 1;
}
