#include <lauxlib.h>
#include <lua.h>
#include <stddef.h>
#include <stdlib.h>

// TODO decide if it's going to be 0- or 1-based indexing...
// leaning towards 1-based to be consistent with the rest of lua
// maybe let user configure it? Might make it too confusing

const char vector_mt_name[] = "vector";
const char vector_lib_mt_name[] = "liblua-vectorize";

typedef struct Vector {
  lua_Number *values;
  int len;
} Vector;

void _vec_check_oob(lua_State *L, Vector *v, int idx) {
  if (idx < 0) {
    luaL_error(L, "Expected positive integer, got %d", idx + 1);
  } else if (idx >= v->len) {
    luaL_error(L, "Index out of bounds: %d (vector %p has length %d)", idx + 1,
               v, v->len);
  }
}

int vec_new(lua_State *L) {
  int len;
  lua_Number *values;
  Vector *v;

  len = luaL_checkinteger(L, 1);
  if (len <= 0) {
    luaL_error(L, "Expected positive integer for size, got %d", len);
  }

  values = calloc(len, sizeof(*values));
  if (values == NULL) {
    luaL_error(L, "Could not allocate vector");
  }

  v = lua_newuserdata(L, sizeof(*v));
  luaL_setmetatable(L, vector_mt_name);
  v->len = len;
  v->values = values;
  return 1;
}

Vector *_vec_push_new(lua_State *L, int len) {
  lua_pushcfunction(L, &vec_new);
  lua_pushinteger(L, len);
  lua_call(L, 1, 1);

  return lua_touserdata(L, -1);
}

int vec_at(lua_State *L) {
  Vector *v = luaL_checkudata(L, 1, vector_mt_name);
  int idx = lua_tointeger(L, 2) - 1;
  _vec_check_oob(L, v, idx);
  lua_pushnumber(L, v->values[idx]);

  return 1;
}

int vec__index(lua_State *L) {
  if (lua_isinteger(L, 2)) {
    // integer indexing
    return vec_at(L);
  } else {
    // method lookup
    const char *fname = luaL_checkstring(L, 2);
    luaL_getmetafield(L, 1, "__lib");
    lua_getfield(L, -1, fname);
    return 1;
  }
}

int vec__newindex(lua_State *L) {
  Vector *v = luaL_checkudata(L, 1, vector_mt_name);
  int idx = luaL_checkinteger(L, 2) - 1;
  _vec_check_oob(L, v, idx);

  v->values[idx] = luaL_checknumber(L, 3);
  return 0;
}

int vec__tostring(lua_State *L) {
  Vector *v = luaL_checkudata(L, 1, vector_mt_name);
  int nterms = 0;

  lua_pushstring(L, "[");
  nterms++;

  for (int i = 0; i < v->len; i++) {
    lua_pushnumber(L, v->values[i]);
    nterms++;

    // Value is a number, tostring changes it in the stack
    // as well as returning it
    (void)lua_tostring(L, -1);

    lua_pushstring(L, ", ");
    nterms++;
  }

  // Replace last separator with a closing bracket
  lua_pop(L, 1);
  lua_pushstring(L, "]");

  lua_concat(L, nterms);
  return 1;
}

int _vec_broadcast_add(lua_State *L, const Vector *v, lua_Number scalar) {
  Vector *new = _vec_push_new(L, v->len);

  for (int i = 0; i < new->len; i++) {
    new->values[i] = v->values[i] + scalar;
  }

  return 1;
}

int _vec_add(lua_State *L, const Vector *v1, const Vector *v2) {
  if (v1->len != v2->len) {
    luaL_error(L, "Vectors must have the same length (%d != %d)", v1->len,
               v2->len);
  }

  Vector *new = _vec_push_new(L, v1->len);
  for (int i = 0; i < new->len; i++) {
    new->values[i] = v1->values[i] + v2->values[i];
  }

  return 1;
}

int vec__add(lua_State *L) {
  if (lua_isnumber(L, 1)) {
    lua_Number scalar = lua_tonumber(L, 1);
    const Vector *v = luaL_checkudata(L, 2, vector_mt_name);
    return _vec_broadcast_add(L, v, scalar);
  } else if (lua_isnumber(L, 2)) {
    lua_Number scalar = lua_tonumber(L, 2);
    const Vector *v = luaL_checkudata(L, 1, vector_mt_name);
    return _vec_broadcast_add(L, v, scalar);
  } else {
    const Vector *v1 = luaL_checkudata(L, 1, vector_mt_name);
    const Vector *v2 = luaL_checkudata(L, 2, vector_mt_name);
    return _vec_add(L, v1, v2);
  }
}

int vec_lib__call(lua_State *L) {
  lua_remove(L, 1);
  return vec_new(L);
}

static const struct luaL_Reg functions[] = {
    {"new", &vec_new}, {"at", &vec_at}, {NULL, NULL}};

void create_lib_metatable(lua_State *L) {
  luaL_newmetatable(L, vector_lib_mt_name);
  lua_pushcfunction(L, &vec_lib__call);
  lua_setfield(L, -2, "__call");
  lua_pop(L, 1);
}

void create_vector_metatable(lua_State *L, int libstackidx) {
  luaL_newmetatable(L, vector_mt_name);
  libstackidx--; // new value was pushed!

  lua_pushvalue(L, libstackidx);
  lua_setfield(L, -2, "__lib");

  lua_pushcfunction(L, &vec__index);
  lua_setfield(L, -2, "__index");

  lua_pushcfunction(L, &vec__newindex);
  lua_setfield(L, -2, "__newindex");

  lua_pushcfunction(L, &vec__tostring);
  lua_setfield(L, -2, "__tostring");

  lua_pushcfunction(L, &vec__add);
  lua_setfield(L, -2, "__add");

  lua_pop(L, 1);
}

int luaopen_vectorize(lua_State *L) {
  create_lib_metatable(L);

  luaL_newlib(L, functions);
  luaL_setmetatable(L, vector_lib_mt_name);

  create_vector_metatable(L, -1);

  return 1;
}
