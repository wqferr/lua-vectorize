#include <lauxlib.h>
#include <lua.h>
#include <stddef.h>
#include <stdlib.h>

const char vector_mt_name[] = "_vectorize_vector_mt";

typedef struct FloatVector {
  lua_Number *values;
  size_t len;
} FloatVector;

int vec_new(lua_State *L) {
  size_t len;
  lua_Number *values;
  FloatVector *v;

  len = luaL_checkinteger(L, 1);
  if (len <= 0) {
    luaL_error(L, "Expected positive integer for size, got %d", len);
  }

  values = malloc(len * sizeof(*values));
  if (values == NULL) {
    luaL_error(L, "Could not allocate vector");
  }

  v = lua_newuserdata(L, sizeof(*v));
  luaL_setmetatable(L, vector_mt_name);
  v->len = len;
  v->values = values;
  return 1;
}

int vec_lib__call(lua_State *L) {
  // TODO manipulate stack to remove the additional parameter
  // and call vec_new
  return 0;
}

static const struct luaL_Reg functions[] = {{"new", &vec_new}, {NULL, NULL}};

int luaopen_vectorize(lua_State *L) {
  luaL_newmetatable(L, vector_mt_name);
  luaL_newlib(L, functions);

  lua_pushvalue(L, -1);
  lua_setfield(L, -3, "__index");

  lua_pushcfunction(L, &vec_lib__call);
  lua_setfield(L, -3, "__call");
  return 1;
}
