#include "lauxlib.h"
#include "lua.h"
#include <errno.h>
#include <math.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "vector.h"
#include "vectorize.h"

const char vector_lib_mt_name[] = "liblua-vectorize";

int vec_lib__call(lua_State *L) {
  lua_remove(L, 1); // remove "self" argument
  if (lua_isnumber(L, 1)) {
    return vec_new(L);
  } else {
    return vec_from(L);
  }
}

void create_lib_metatable(lua_State *L) {
  luaL_newmetatable(L, vector_lib_mt_name);
  lua_pushcfunction(L, &vec_lib__call);
  lua_setfield(L, -2, "__call");
  lua_pop(L, 1);
}

int luaopen_vectorize(lua_State *L) {
  create_lib_metatable(L);

  luaL_newlib(L, vec_functions);
  luaL_setmetatable(L, vector_lib_mt_name);

  create_vector_metatable(L, -1);

  return 1;
}
