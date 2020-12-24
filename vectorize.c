#include <lauxlib.h>
#include <lua.h>
#include <stddef.h>
#include <stdlib.h>

// TODO decide if it's going to be 0- or 1-based indexing...
// leaning towards 1-based to be consistent with the rest of lua
// maybe let user configure it? Might make it too confusing

const char vector_mt_name[] = "_vectorize_vector_mt";
const char vector_lib_mt_name[] = "_vectorize_lib_mt";

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

int vec_add(lua_State *L) {
  lua_pushglobaltable(L);
  return 0;
}

int vec_lib__call(lua_State *L) {
  // TODO check if this actually works
  // this is a macro for a function called rotate, this might make things
  // more complicated than they seem
  // UPDATE ON THE TODO:
  // the positive indices for parameters may be the so called pseudo-indices
  // it depends on if "upvalues" also include parameters to C functions
  lua_remove(L, 1);
  return vec_new(L);
}

static const struct luaL_Reg functions[] = {{"new", &vec_new}, {NULL, NULL}};

void create_vector_metatable(lua_State *L, int libstackidx) {
  luaL_newmetatable(L, vector_mt_name);

  lua_pushvalue(L, libstackidx);
  lua_setfield(L, -2, "__index"); // TODO make __index a function so people can use v[1]

  lua_pushcfunction(L, &vec_add);
  lua_setfield(L, -2, "__add");

  lua_pop(L, 1);
}

void create_lib_metatable(lua_State *L) {
  luaL_newmetatable(L, vector_lib_mt_name);
  lua_pushcfunction(L, &vec_lib__call);
  lua_setfield(L, -3, "__call");
  lua_pop(L, 1);
}

int luaopen_vectorize(lua_State *L) {
  create_lib_metatable(L);

  luaL_newlib(L, functions);
  luaL_setmetatable(L, vector_lib_mt_name);

  create_vector_metatable(L, -1);

  return 1;
}
