#ifndef _VECTORIZE_VEC_H
#define _VECTORIZE_VEC_H 1

#include "lua.h"

typedef struct Vector {
  lua_Number *values;
  lua_Integer len;
} Vector;

extern const char vector_mt_name[];

int vec_new(lua_State *L);
int vec_from(lua_State *L);
int vec_ones(lua_State *L);
int vec_basis(lua_State *L);
int vec_linspace(lua_State *L);
int vec_dup(lua_State *L);
int vec_dup_into(lua_State *L);
int vec_save(lua_State *L);
int vec_load(lua_State *L);

int vec_add(lua_State *L);
int vec_add_into(lua_State *L);
int vec_sub(lua_State *L);
int vec_sub_into(lua_State *L);
int vec_mul(lua_State *L);
int vec_mul_into(lua_State *L);
int vec_div(lua_State *L);
int vec_div_into(lua_State *L);
int vec_pow(lua_State *L);
int vec_pow_into(lua_State *L);
int vec_neg(lua_State *L);
int vec_neg_into(lua_State *L);
int vec_reciproc(lua_State *L);
int vec_reciproc_into(lua_State *L);
int vec_psy(lua_State *L);
int vec_psy_into(lua_State *L);
int vec_hadamard_product(lua_State *L);
int vec_scale(lua_State *L);
int vec_scale_into(lua_State *L);
int vec_project(lua_State *L);
int vec_project_into(lua_State *L);
int vec_normalize(lua_State *L);
int vec_normalize_into(lua_State *L);

int vec_inner(lua_State *L);
int vec_cosine_similarity(lua_State *L);

int vec_at(lua_State *L);
int vec_iter(lua_State *L);
int vec_sum(lua_State *L);

int vec_norm(lua_State *L);
int vec_norm2(lua_State *L);

int vec_trapz(lua_State *L);

int vec_sq(lua_State *L);
int vec_sq_into(lua_State *L);
int vec_cb(lua_State *L);
int vec_cb_into(lua_State *L);
int vec_sqrt(lua_State *L);
int vec_sqrt_into(lua_State *L);
int vec_cbrt(lua_State *L);
int vec_cbrt_into(lua_State *L);

int vec_exp(lua_State *L);
int vec_exp_into(lua_State *L);
int vec_ln(lua_State *L);
int vec_ln_into(lua_State *L);
int vec_ln1p(lua_State *L);
int vec_ln1p_into(lua_State *L);

int vec_sin(lua_State *L);
int vec_sin_into(lua_State *L);
int vec_asin(lua_State *L);
int vec_asin_into(lua_State *L);
int vec_sinh(lua_State *L);
int vec_sinh_into(lua_State *L);
int vec_asinh(lua_State *L);
int vec_asinh_into(lua_State *L);

int vec_cos(lua_State *L);
int vec_cos_into(lua_State *L);
int vec_acos(lua_State *L);
int vec_acos_into(lua_State *L);
int vec_cosh(lua_State *L);
int vec_cosh_into(lua_State *L);
int vec_acosh(lua_State *L);
int vec_acosh_into(lua_State *L);

int vec_tan(lua_State *L);
int vec_tan_into(lua_State *L);
int vec_atan(lua_State *L);
int vec_atan_into(lua_State *L);
int vec_tanh(lua_State *L);
int vec_tanh_into(lua_State *L);
int vec_atanh(lua_State *L);
int vec_atanh_into(lua_State *L);

int vec__index(lua_State *L);
int vec__newindex(lua_State *L);
int vec__tostring(lua_State *L);
int vec__len(lua_State *L);
int vec__gc(lua_State *L);

void create_vector_metatable(lua_State *L, int libstackidx);

// Can't define it here because it's considered to be included multiple times.
// Declare it with its size so it can be used in luaL_newlib in vectorize.c.
extern const struct luaL_Reg vec_functions[85];

#endif /* ifndef _VECTORIZE_VEC_H */
