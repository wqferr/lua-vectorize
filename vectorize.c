#include "lua.h"
#include "lauxlib.h"

#define MESSAGE "hello world!"

int return_hello(struct lua_State *L) {
    lua_pushstring(L, MESSAGE);
    return 1;
}

static const struct luaL_Reg functions[] = {
    {"return_hello", return_hello},
    {NULL, NULL}
};

int luaopen_vectorize(lua_State *L) {
    luaL_newlib(L, functions);
    return 1;
}
