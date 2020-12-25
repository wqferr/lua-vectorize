#include <lauxlib.h>
#include <lua.h>
#include <math.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>

const char vector_mt_name[] = "vector";
const char vector_lib_mt_name[] = "liblua-vectorize";

typedef struct Vector {
  lua_Number *values;
  int len;
} Vector;

void _vec_check_oob(lua_State *L, int idx, int len) {
  // idx is the 0-based index!
  if (idx < 0) {
    luaL_error(L, "Expected positive integer, got %d", idx + 1);
  } else if (idx >= len) {
    luaL_error(
      L, "Index out of bounds: %d (vector has length %d)", idx + 1, len);
  }
}

void _vec_check_same_len(lua_State *L, const Vector *x, const Vector *y) {
  if (x->len != y->len) {
    luaL_error(
      L, "Vectors must have the same length (%d != %d)", x->len, y->len);
  }
}

int vec_new(lua_State *L) {
  int len;
  lua_Number *values;
  Vector *v;

  len = luaL_checkinteger(L, 1);
  lua_pop(L, 1);
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

int vec_from(lua_State *L) {
  int len = luaL_len(L, 1);
  Vector *v = _vec_push_new(L, len);
  for (int i = 1; i <= len; i++) {
    lua_pushinteger(L, i);
    lua_gettable(L, 1);
    if (!lua_isnumber(L, -1)) {
      luaL_error(
        L,
        "Tried to create a vector, but element at position %d is a %s instead "
        "of a number",
        i,
        luaL_typename(L, -1));
    }
    v->values[i - 1] = lua_tonumber(L, -1);
    lua_pop(L, 1);
  }
  return 1;
}

int vec_linspace(lua_State *L) {
  lua_Number start = luaL_checknumber(L, 1);
  lua_Number end = luaL_checknumber(L, 2);
  lua_Integer len = luaL_checkinteger(L, 3);
  lua_Number step_num = end - start;
  lua_Number step_den = len - 1;

  Vector *new = _vec_push_new(L, len);

  for (int i = 0; i < len; i++) {
    new->values[i] = start + ((i * step_num) / step_den);
  }

  return 1;
}

int vec_ones(lua_State *L) {
  vec_new(L);
  Vector *new = lua_touserdata(L, -1);
  for (int i = 0; i < new->len; i++) {
    new->values[i] = 1;
  }
  return 1;
}

int vec_basis(lua_State *L) {
  int len = luaL_checkinteger(L, 1);
  int onepos = luaL_checkinteger(L, 2) - 1;
  lua_pop(L, 2);
  _vec_check_oob(L, onepos, len);

  Vector *new = _vec_push_new(L, len);
  new->values[onepos] = 1;

  return 1;
}

int vec_at(lua_State *L) {
  Vector *v = luaL_checkudata(L, 1, vector_mt_name);
  int idx = lua_tointeger(L, 2) - 1;
  _vec_check_oob(L, idx, v->len);
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
  _vec_check_oob(L, idx, v->len);

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

int vec__len(lua_State *L) {
  Vector *v = luaL_checkudata(L, 1, vector_mt_name);
  lua_pushinteger(L, v->len);
  return 1;
}

int _vec_broadcast_add(lua_State *L, const Vector *v, lua_Number scalar) {
  Vector *new = _vec_push_new(L, v->len);
  for (int i = 0; i < new->len; i++) {
    new->values[i] = v->values[i] + scalar;
  }
  return 1;
}

int _vec_broadcast_pow(lua_State *L, const Vector *v, lua_Number e) {
  Vector *new = _vec_push_new(L, v->len);
  for (int i = 0; i < new->len; i++) {
    new->values[i] = pow(v->values[i], e);
  }
  return 1;
}

int _vec_broadcast_pow_rev(lua_State *L, lua_Number base, const Vector *v) {
  Vector *new = _vec_push_new(L, v->len);
  for (int i = 0; i < new->len; i++) {
    new->values[i] = pow(base, v->values[i]);
  }
  return 1;
}

int _vec_pow(lua_State *L, const Vector *b, const Vector *e) {
  _vec_check_same_len(L, b, e);
  Vector *new = _vec_push_new(L, b->len);
  for (int i = 0; i < new->len; i++) {
    new->values[i] = pow(b->values[i], e->values[i]);
  }
  return 1;
}

int _vec_xpsy(lua_State *L, const Vector *x, lua_Number s, const Vector *y) {
  _vec_check_same_len(L, x, y);
  Vector *new = _vec_push_new(L, x->len);
  for (int i = 0; i < new->len; i++) {
    new->values[i] = x->values[i] + s * y->values[i];
  }
  return 1;
}

int _vec_hadamard_product(lua_State *L, const Vector *x, const Vector *y) {
  _vec_check_same_len(L, x, y);
  Vector *new = _vec_push_new(L, x->len);
  for (int i = 0; i < new->len; i++) {
    new->values[i] = x->values[i] * y->values[i];
  }
  return 1;
}

int _vec_scale(lua_State *L, const Vector *v, lua_Number s) {
  Vector *new = _vec_push_new(L, v->len);
  for (int i = 0; i < new->len; i++) {
    new->values[i] = v->values[i] * s;
  }
  return 1;
}

int _vec_elmwise_div(lua_State *L, const Vector *x, const Vector *y) {
  _vec_check_same_len(L, x, y);
  Vector *new = _vec_push_new(L, x->len);
  for (int i = 0; i < new->len; i++) {
    new->values[i] = x->values[i] / y->values[i];
  }
  return 1;
}

int _vec_elmwise_div_scalar(lua_State *L, lua_Number scalar, const Vector *x) {
  Vector *new = _vec_push_new(L, x->len);
  for (int i = 0; i < new->len; i++) {
    new->values[i] = scalar / x->values[i];
  }
  return 1;
}

int vec_psy(lua_State *L) {
  Vector *self = luaL_checkudata(L, 1, vector_mt_name);
  lua_Number scalar = luaL_checknumber(L, 2);
  Vector *other = luaL_checkudata(L, 3, vector_mt_name);
  return _vec_xpsy(L, self, scalar, other);
}

int vec_hadamard_product(lua_State *L) {
  Vector *self = luaL_checkudata(L, 1, vector_mt_name);
  Vector *other = luaL_checkudata(L, 2, vector_mt_name);
  return _vec_hadamard_product(L, self, other);
}

int vec_scale(lua_State *L) {
  Vector *self = luaL_checkudata(L, 1, vector_mt_name);
  lua_Number scalar = luaL_checknumber(L, 2);
  return _vec_scale(L, self, scalar);
}

int vec_sq(lua_State *L) {
  Vector *self = luaL_checkudata(L, 1, vector_mt_name);
  Vector *new = _vec_push_new(L, self->len);
  for (int i = 0; i < new->len; i++) {
    new->values[i] = self->values[i] * self->values[i];
  }
  return 1;
}

int vec_sqrt(lua_State *L) {
  Vector *self = luaL_checkudata(L, 1, vector_mt_name);
  Vector *new = _vec_push_new(L, self->len);
  for (int i = 0; i < new->len; i++) {
    new->values[i] = sqrt(self->values[i]);
  }
  return 1;
}

int vec_cb(lua_State *L) {
  Vector *self = luaL_checkudata(L, 1, vector_mt_name);
  Vector *new = _vec_push_new(L, self->len);
  for (int i = 0; i < new->len; i++) {
    new->values[i] = self->values[i] * self->values[i] * self->values[i];
  }
  return 1;
}

int vec_cbrt(lua_State *L) {
  Vector *self = luaL_checkudata(L, 1, vector_mt_name);
  Vector *new = _vec_push_new(L, self->len);
  for (int i = 0; i < new->len; i++) {
    new->values[i] = cbrt(self->values[i]);
  }
  return 1;
}
int vec_exp(lua_State *L) {
  Vector *self = luaL_checkudata(L, 1, vector_mt_name);
  Vector *new = _vec_push_new(L, self->len);
  for (int i = 0; i < new->len; i++) {
    new->values[i] = exp(self->values[i]);
  }
  return 1;
}

int vec_ln(lua_State *L) {
  Vector *self = luaL_checkudata(L, 1, vector_mt_name);
  Vector *new = _vec_push_new(L, self->len);
  for (int i = 0; i < new->len; i++) {
    new->values[i] = log(self->values[i]);
  }
  return 1;
}

int vec_ln1p(lua_State *L) {
  Vector *self = luaL_checkudata(L, 1, vector_mt_name);
  Vector *new = _vec_push_new(L, self->len);
  for (int i = 0; i < new->len; i++) {
    new->values[i] = log(1 + self->values[i]);
  }
  return 1;
}

int vec_sin(lua_State *L) {
  Vector *self = luaL_checkudata(L, 1, vector_mt_name);
  Vector *new = _vec_push_new(L, self->len);
  for (int i = 0; i < new->len; i++) {
    new->values[i] = sin(self->values[i]);
  }
  return 1;
}

int vec_cos(lua_State *L) {
  Vector *self = luaL_checkudata(L, 1, vector_mt_name);
  Vector *new = _vec_push_new(L, self->len);
  for (int i = 0; i < new->len; i++) {
    new->values[i] = cos(self->values[i]);
  }
  return 1;
}

int vec_tan(lua_State *L) {
  Vector *self = luaL_checkudata(L, 1, vector_mt_name);
  Vector *new = _vec_push_new(L, self->len);
  for (int i = 0; i < new->len; i++) {
    new->values[i] = tan(self->values[i]);
  }
  return 1;
}

int vec_sinh(lua_State *L) {
  Vector *self = luaL_checkudata(L, 1, vector_mt_name);
  Vector *new = _vec_push_new(L, self->len);
  for (int i = 0; i < new->len; i++) {
    new->values[i] = sinh(self->values[i]);
  }
  return 1;
}

int vec_cosh(lua_State *L) {
  Vector *self = luaL_checkudata(L, 1, vector_mt_name);
  Vector *new = _vec_push_new(L, self->len);
  for (int i = 0; i < new->len; i++) {
    new->values[i] = cosh(self->values[i]);
  }
  return 1;
}

int vec_tanh(lua_State *L) {
  Vector *self = luaL_checkudata(L, 1, vector_mt_name);
  Vector *new = _vec_push_new(L, self->len);
  for (int i = 0; i < new->len; i++) {
    new->values[i] = tanh(self->values[i]);
  }
  return 1;
}

int vec_asin(lua_State *L) {
  Vector *self = luaL_checkudata(L, 1, vector_mt_name);
  Vector *new = _vec_push_new(L, self->len);
  for (int i = 0; i < new->len; i++) {
    new->values[i] = asin(self->values[i]);
  }
  return 1;
}

int vec_acos(lua_State *L) {
  Vector *self = luaL_checkudata(L, 1, vector_mt_name);
  Vector *new = _vec_push_new(L, self->len);
  for (int i = 0; i < new->len; i++) {
    new->values[i] = acos(self->values[i]);
  }
  return 1;
}

int vec_atan(lua_State *L) {
  Vector *self = luaL_checkudata(L, 1, vector_mt_name);
  Vector *new = _vec_push_new(L, self->len);
  for (int i = 0; i < new->len; i++) {
    new->values[i] = atan(self->values[i]);
  }
  return 1;
}

int vec_asinh(lua_State *L) {
  Vector *self = luaL_checkudata(L, 1, vector_mt_name);
  Vector *new = _vec_push_new(L, self->len);
  for (int i = 0; i < new->len; i++) {
    new->values[i] = asinh(self->values[i]);
  }
  return 1;
}

int vec_acosh(lua_State *L) {
  Vector *self = luaL_checkudata(L, 1, vector_mt_name);
  Vector *new = _vec_push_new(L, self->len);
  for (int i = 0; i < new->len; i++) {
    new->values[i] = acosh(self->values[i]);
  }
  return 1;
}

int vec_atanh(lua_State *L) {
  Vector *self = luaL_checkudata(L, 1, vector_mt_name);
  Vector *new = _vec_push_new(L, self->len);
  for (int i = 0; i < new->len; i++) {
    new->values[i] = atanh(self->values[i]);
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
    return _vec_xpsy(L, v1, 1, v2);
  }
}

int vec__sub(lua_State *L) {
  if (lua_isnumber(L, 1)) {
    lua_Number scalar = lua_tonumber(L, 1);
    const Vector *v = luaL_checkudata(L, 2, vector_mt_name);
    return _vec_broadcast_add(L, v, -scalar);
  } else if (lua_isnumber(L, 2)) {
    lua_Number scalar = lua_tonumber(L, 2);
    const Vector *v = luaL_checkudata(L, 1, vector_mt_name);
    return _vec_broadcast_add(L, v, -scalar);
  } else {
    const Vector *v1 = luaL_checkudata(L, 1, vector_mt_name);
    const Vector *v2 = luaL_checkudata(L, 2, vector_mt_name);
    return _vec_xpsy(L, v1, -1, v2);
  }
}

int vec__mul(lua_State *L) {
  if (lua_isnumber(L, 1)) {
    lua_Number scalar = lua_tonumber(L, 1);
    const Vector *v = luaL_checkudata(L, 2, vector_mt_name);
    return _vec_scale(L, v, scalar);
  } else if (lua_isnumber(L, 2)) {
    lua_Number scalar = lua_tonumber(L, 2);
    const Vector *v = luaL_checkudata(L, 1, vector_mt_name);
    return _vec_scale(L, v, scalar);
  } else {
    const Vector *v1 = luaL_checkudata(L, 1, vector_mt_name);
    const Vector *v2 = luaL_checkudata(L, 2, vector_mt_name);
    return _vec_hadamard_product(L, v1, v2);
  }
}

int vec__div(lua_State *L) {
  if (lua_isnumber(L, 1)) {
    lua_Number scalar = lua_tonumber(L, 1);
    const Vector *v = luaL_checkudata(L, 2, vector_mt_name);
    return _vec_elmwise_div_scalar(L, scalar, v);
  } else if (lua_isnumber(L, 2)) {
    lua_Number scalar = lua_tonumber(L, 2);
    const Vector *v = luaL_checkudata(L, 1, vector_mt_name);
    return _vec_scale(L, v, 1 / scalar);
  } else {
    const Vector *v1 = luaL_checkudata(L, 1, vector_mt_name);
    const Vector *v2 = luaL_checkudata(L, 2, vector_mt_name);
    return _vec_elmwise_div(L, v1, v2);
  }
}

int vec__pow(lua_State *L) {
  if (lua_isnumber(L, 1)) {
    lua_Number scalar = lua_tonumber(L, 1);
    const Vector *v = luaL_checkudata(L, 2, vector_mt_name);
    return _vec_broadcast_pow_rev(L, scalar, v);
  } else if (lua_isnumber(L, 2)) {
    lua_Number scalar = lua_tonumber(L, 2);
    const Vector *v = luaL_checkudata(L, 1, vector_mt_name);
    return _vec_broadcast_pow(L, v, scalar);
  } else {
    const Vector *v1 = luaL_checkudata(L, 1, vector_mt_name);
    const Vector *v2 = luaL_checkudata(L, 2, vector_mt_name);
    return _vec_pow(L, v1, v2);
  }
}

int vec__gc(lua_State *L) {
  Vector *v = luaL_checkudata(L, 1, vector_mt_name);
  free(v->values);
  return 0;
}

int vec__unm(lua_State *L) {
  lua_pushcfunction(L, &vec_scale);
  lua_pushvalue(L, 1);
  lua_pushnumber(L, -1);
  lua_call(L, 2, 1);
  return 1;
}

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
  {"add", &vec__add},
  {"sub", &vec__sub},
  {"mul", &vec__mul},
  {"div", &vec__div},
  {"pow", &vec__pow},
  {"neg", &vec__unm},
  {"sq", &vec_sq},
  {"square", &vec_sq},
  {"sqrt", &vec_sqrt},
  {"cb", &vec_cb},
  {"cube", &vec_cb},
  {"cbrt", &vec_cbrt},
  {"exp", &vec_exp},
  {"ln", &vec_ln},
  {"ln1p", &vec_ln1p},
  {"sin", &vec_sin},
  {"cos", &vec_cos},
  {"tan", &vec_tan},
  {"sinh", &vec_sinh},
  {"cosh", &vec_cosh},
  {"tanh", &vec_tanh},
  {"asin", &vec_asin},
  {"acos", &vec_acos},
  {"atan", &vec_atan},
  {"asinh", &vec_asinh},
  {"acosh", &vec_acosh},
  {"atanh", &vec_atanh},
  {"at", &vec_at},
  {"psy", &vec_psy},
  {"scale", &vec_scale},
  {"hadamard", &vec_hadamard_product},
  {NULL, NULL}};

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

  lua_pushcfunction(L, &vec__len);
  lua_setfield(L, -2, "__len");

  lua_pushcfunction(L, &vec__add);
  lua_setfield(L, -2, "__add");

  lua_pushcfunction(L, &vec__sub);
  lua_setfield(L, -2, "__sub");

  lua_pushcfunction(L, &vec__mul);
  lua_setfield(L, -2, "__mul");

  lua_pushcfunction(L, &vec__div);
  lua_setfield(L, -2, "__div");

  lua_pushcfunction(L, &vec__pow);
  lua_setfield(L, -2, "__pow");

  lua_pushcfunction(L, &vec__unm);
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
