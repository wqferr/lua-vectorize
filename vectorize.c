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

static const struct luaL_Reg functions[] = {
  {"new", &vec_new},
  {"from", &vec_from},
  {"ones", &vec_ones},
  {"basis", &vec_basis},
  {"linspace", &vec_linspace},
  {"dup", &vec_dup},
  {"dup_", &vec_dup_into},
  {"save", &vec_save},
  {"load", &vec_load},

  {"add", &vec_add},
  {"add_", &vec_add_into},
  {"sub", &vec_sub},
  {"sub_", &vec_sub_into},
  {"mul", &vec_mul},
  {"mul_", &vec_mul_into},
  {"div", &vec_div},
  {"div_", &vec_div_into},
  {"pow", &vec_pow},
  {"pow_", &vec_pow_into},
  {"neg", &vec_neg},
  {"neg_", &vec_neg_into},
  {"reciproc", &vec_reciproc},
  {"reciproc_", &vec_reciproc_into},
  {"psy", &vec_psy},
  {"psy_", &vec_psy_into},
  {"hadamard", &vec_hadamard_product},
  {"hadamard_", &vec_hadamard_product},
  {"scale", &vec_scale},
  {"scale_", &vec_scale_into},
  {"inner", &vec_inner},
  {"project", &vec_project},
  {"project_", &vec_project_into},
  {"cosine_similarity", &vec_cosine_similarity},

  {"at", &vec_at},
  {"len", &vec__len},
  {"iter", &vec_iter},
  {"sum", &vec_sum},

  {"norm", &vec_norm},
  {"norm2", &vec_norm2},
  {"normalize", &vec_normalize},
  {"normalize_", &vec_normalize_into},

  {"trapz", &vec_trapz},

  {"sq", &vec_sq},
  {"sq_", &vec_sq_into},
  {"square", &vec_sq},
  {"square_", &vec_sq_into},
  {"cb", &vec_cb},
  {"cb_", &vec_cb_into},
  {"cube", &vec_cb},
  {"cube_", &vec_cb_into},

  {"sqrt", &vec_sqrt},
  {"sqrt_", &vec_sqrt_into},
  {"cbrt", &vec_cbrt},
  {"cbrt_", &vec_cbrt_into},

  {"exp", &vec_exp},
  {"exp_", &vec_exp_into},
  {"ln", &vec_ln},
  {"ln_", &vec_ln_into},
  {"ln1p", &vec_ln1p},
  {"ln1p_", &vec_ln1p_into},

  {"sin", &vec_sin},
  {"sin_", &vec_sin_into},
  {"sinh", &vec_sinh},
  {"sinh_", &vec_sinh_into},
  {"asin", &vec_asin},
  {"asin_", &vec_asin_into},
  {"asinh", &vec_asinh},
  {"asinh_", &vec_asinh_into},

  {"cos", &vec_cos},
  {"cos_", &vec_cos_into},
  {"cosh", &vec_cosh},
  {"cosh_", &vec_cosh_into},
  {"acos", &vec_acos},
  {"acos_", &vec_acos_into},
  {"acosh", &vec_acosh},
  {"acosh_", &vec_acosh_into},

  {"tan", &vec_tan},
  {"tan_", &vec_tan_into},
  {"tanh", &vec_tanh},
  {"tanh_", &vec_tanh_into},
  {"atan", &vec_atan},
  {"atan_", &vec_atan_into},
  {"atanh", &vec_atanh},
  {"atanh_", &vec_atanh_into},

  {NULL, NULL}};

void create_lib_metatable(lua_State *L) {
  luaL_newmetatable(L, vector_lib_mt_name);
  lua_pushcfunction(L, &vec_lib__call);
  lua_setfield(L, -2, "__call");
  lua_pop(L, 1);
}

void create_vector_metatable(lua_State *L, int libstackidx) {
  luaL_newmetatable(L, vector_mt_name);
  if (libstackidx < 0) // idx is relative to the top
    libstackidx--;     // new value was pushed!

  lua_pushvalue(L, libstackidx);
  lua_setfield(L, -2, "__lib");

  lua_pushcfunction(L, &vec__index);
  lua_setfield(L, -2, "__index");

  lua_pushcfunction(L, &vec__newindex);
  lua_setfield(L, -2, "__newindex");

  lua_pushcfunction(L, &vec__tostring);
  lua_setfield(L, -2, "__tostring");

  lua_pushcfunction(L, &vec__len);
  lua_setfield(L, -2, "__len");

  lua_pushcfunction(L, &vec_add);
  lua_setfield(L, -2, "__add");

  lua_pushcfunction(L, &vec_sub);
  lua_setfield(L, -2, "__sub");

  lua_pushcfunction(L, &vec_mul);
  lua_setfield(L, -2, "__mul");

  lua_pushcfunction(L, &vec_div);
  lua_setfield(L, -2, "__div");

  lua_pushcfunction(L, &vec_pow);
  lua_setfield(L, -2, "__pow");

  lua_pushcfunction(L, &vec_neg);
  lua_setfield(L, -2, "__unm");

  lua_pushcfunction(L, &vec__gc);
  lua_setfield(L, -2, "__gc");

  lua_pop(L, 1);
}

int luaopen_vectorize(lua_State *L) {
  create_lib_metatable(L);

  luaL_newlib(L, functions);
  luaL_setmetatable(L, vector_lib_mt_name);

  create_vector_metatable(L, -1);

  return 1;
}
