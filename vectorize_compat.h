#ifndef VECTORIZE_COMPAT_H
#define VECTORIZE_COMPAT_H 1

#include "lauxlib.h"
#include "lua.h"

#if LUA_VERSION_NUM == 504
#define newudata(L, size) (lua_newuserdatauv(L, size, 0))

#else
#define newudata(L, size) (lua_newuserdata(L, size))

#endif

#if LUA_VERSION_NUM == 502
static inline bool lua_isinteger(lua_State *L, int idx) {
  int ok;
  lua_tointegerx(L, idx, &ok);
  return ok;
}

#elif LUA_VERSION_NUM == 501
#define lua_isinteger(L, idx) (lua_isnumber(L, idx))

#endif

static inline void setmetatable(lua_State *L, const char *mtname) {
#if LUA_VERSION_NUM == 501
  luaL_newmetatable(L, mtname);
  lua_setmetatable(L, -2);

#else
  luaL_setmetatable(L, mtname);

#endif
}

#if LUA_VERSION_NUM == 501
#define luaL_len(L, idx) (lua_objlen(L, idx))

#ifndef luaL_newlib

// moonjit defines this already, need to safeguard it
#define luaL_newlib(L, l) (luaL_newlib_(L, l, (sizeof(l) / sizeof((l)[0]) - 1)))

#endif

inline void luaL_setfuncs(lua_State *L, const luaL_Reg *l, int nup) {
  int tidx = lua_gettop(L) - nup;

  while (l->name != NULL) {
    for (int i = 1; i <= nup; i++) {
      lua_pushvalue(L, tidx + i);
    }
    lua_pushcclosure(L, l->func, nup);
    lua_setfield(L, tidx, l->name);
    l++;
  }
  lua_pop(L, nup);
}

static inline void luaL_newlib_(lua_State *L, const luaL_Reg *l, int size) {
  lua_createtable(L, 0, size);
  luaL_setfuncs(L, l, 0);
}

static inline int lua_absindex(lua_State *L, int i) {
  if (i > 0) {
    return i;
  } else {
    return lua_gettop(L) + i + 1;
  }
}
#endif

#endif
