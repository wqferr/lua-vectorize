#include <lauxlib.h>
#include <lua.h>
#include <math.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

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

int vec_dup(lua_State *L) {
  Vector *self = luaL_checkudata(L, 1, vector_mt_name);
  Vector *new = _vec_push_new(L, self->len);
  memcpy(new->values, self->values, self->len * sizeof(lua_Number));
  return 1;
}

int vec_dup_into(lua_State *L) {
  Vector *self = luaL_checkudata(L, 1, vector_mt_name);
  Vector *new = luaL_checkudata(L, 2, vector_mt_name);
  _vec_check_same_len(L, self, new);

  memcpy(new->values, self->values, self->len * sizeof(lua_Number));
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

void _vec_broadcast_add_into(
  lua_State *L, Vector *v, lua_Number scalar, Vector *out) {
  _vec_check_same_len(L, v, out);
  for (int i = 0; i < out->len; i++) {
    out->values[i] = v->values[i] + scalar;
  }
}

void _vec_broadcast_pow_rev_into(
  lua_State *L, lua_Number base, const Vector *v, Vector *out) {
  _vec_check_same_len(L, v, out);
  for (int i = 0; i < out->len; i++) {
    out->values[i] = pow(base, v->values[i]);
  }
}

void _vec_broadcast_pow_into(
  lua_State *L, const Vector *v, lua_Number e, Vector *out) {
  _vec_check_same_len(L, v, out);
  for (int i = 0; i < out->len; i++) {
    out->values[i] = pow(v->values[i], e);
  }
}

void _vec_pow_into(
  lua_State *L, const Vector *b, const Vector *e, Vector *out) {
  _vec_check_same_len(L, b, e);
  _vec_check_same_len(L, b, out);
  for (int i = 0; i < out->len; i++) {
    out->values[i] = pow(b->values[i], e->values[i]);
  }
}

void _vec_xpsy_into(
  lua_State *L, const Vector *x, lua_Number s, const Vector *y, Vector *out) {
  _vec_check_same_len(L, x, y);
  _vec_check_same_len(L, x, out);
  for (int i = 0; i < out->len; i++) {
    out->values[i] = x->values[i] + s * y->values[i];
  }
}

void _vec_hadamard_product_into(
  lua_State *L, const Vector *x, const Vector *y, Vector *out) {
  _vec_check_same_len(L, x, y);
  _vec_check_same_len(L, x, out);
  for (int i = 0; i < out->len; i++) {
    out->values[i] = x->values[i] * y->values[i];
  }
}

void _vec_scale_into(lua_State *L, const Vector *v, lua_Number s, Vector *out) {
  _vec_check_same_len(L, v, out);
  for (int i = 0; i < out->len; i++) {
    out->values[i] = v->values[i] * s;
  }
}

void _vec_scale_reciproc_into(
  lua_State *L, const Vector *v, lua_Number s, Vector *out) {
  _vec_check_same_len(L, v, out);
  for (int i = 0; i < out->len; i++) {
    out->values[i] = v->values[i] / s;
  }
}

void _vec_elmwise_div_into(
  lua_State *L, const Vector *x, const Vector *y, Vector *out) {
  _vec_check_same_len(L, x, y);
  _vec_check_same_len(L, x, out);
  for (int i = 0; i < out->len; i++) {
    out->values[i] = x->values[i] / y->values[i];
  }
}

void _vec_elmwise_div_scalar_into(
  lua_State *L, lua_Number scalar, const Vector *x, Vector *out) {
  _vec_check_same_len(L, x, out);
  for (int i = 0; i < out->len; i++) {
    out->values[i] = scalar / x->values[i];
  }
}

int vec_psy_into(lua_State *L) {
  Vector *self = luaL_checkudata(L, 1, vector_mt_name);
  lua_Number scalar = luaL_checknumber(L, 2);
  Vector *other = luaL_checkudata(L, 3, vector_mt_name);
  Vector *out;

  if (lua_gettop(L) > 3) {
    out = luaL_checkudata(L, 4, vector_mt_name);
  } else {
    out = self;
    lua_pushvalue(L, 1);
  }
  _vec_xpsy_into(L, self, scalar, other, out);
  return 1;
}

int vec_psy(lua_State *L) {
  Vector *self = luaL_checkudata(L, 1, vector_mt_name);
  lua_Number scalar = luaL_checknumber(L, 2);
  Vector *other = luaL_checkudata(L, 3, vector_mt_name);
  Vector *new = _vec_push_new(L, self->len);
  _vec_xpsy_into(L, self, scalar, other, new);
  return 1;
}

int vec_hadamard_product_into(lua_State *L) {
  Vector *self = luaL_checkudata(L, 1, vector_mt_name);
  Vector *other = luaL_checkudata(L, 2, vector_mt_name);
  Vector *out;

  if (lua_gettop(L) > 2) {
    out = luaL_checkudata(L, 3, vector_mt_name);
    _vec_check_same_len(L, self, out);
  } else {
    out = self;
    lua_pushvalue(L, 1);
  }

  _vec_hadamard_product_into(L, self, other, out);
  return 1;
}

int vec_hadamard_product(lua_State *L) {
  Vector *self = luaL_checkudata(L, 1, vector_mt_name);
  Vector *other = luaL_checkudata(L, 2, vector_mt_name);
  Vector *new = _vec_push_new(L, self->len);
  _vec_hadamard_product_into(L, self, other, new);
  return 1;
}

int vec_scale_into(lua_State *L) {
  Vector *self = luaL_checkudata(L, 1, vector_mt_name);
  lua_Number scalar = luaL_checknumber(L, 2);
  Vector *out;

  if (lua_gettop(L) > 2) {
    out = luaL_checkudata(L, 3, vector_mt_name);
    _vec_check_same_len(L, self, out);
  } else {
    out = self;
    lua_pushvalue(L, 1);
  }

  _vec_scale_into(L, self, scalar, out);
  return 1;
}

int vec_scale(lua_State *L) {
  Vector *self = luaL_checkudata(L, 1, vector_mt_name);
  lua_Number scalar = luaL_checknumber(L, 2);
  Vector *new = _vec_push_new(L, self->len);
  _vec_scale_into(L, self, scalar, new);
  return 1;
}

#define def_vec_op(name, expr)                                                 \
  int vec_##name##_into(lua_State *L) {                                        \
    Vector *self = luaL_checkudata(L, 1, vector_mt_name);                      \
    Vector *out;                                                               \
    if (lua_gettop(L) > 1) {                                                   \
      out = luaL_checkudata(L, 2, vector_mt_name);                             \
      _vec_check_same_len(L, self, out);                                       \
    } else {                                                                   \
      out = self;                                                              \
    }                                                                          \
                                                                               \
    for (int i = 0; i < self->len; i++) {                                      \
      out->values[i] = (expr);                                                 \
    }                                                                          \
    return 1; /* out is already on the top of the stack */                     \
  }                                                                            \
                                                                               \
  int vec_##name(lua_State *L) {                                               \
    Vector *self = luaL_checkudata(L, 1, vector_mt_name);                      \
    Vector *out = _vec_push_new(L, self->len);                                 \
    for (int i = 0; i < out->len; i++) {                                       \
      out->values[i] = (expr);                                                 \
    }                                                                          \
    return 1;                                                                  \
  }

#define def_vec_op_func(fname) def_vec_op(fname, fname(self->values[i]))

def_vec_op(sq, self->values[i] * self->values[i]);
def_vec_op(sqrt, self->values[i] * self->values[i]);
def_vec_op(cb, self->values[i] * self->values[i] * self->values[i]);
def_vec_op(cbrt, self->values[i] * self->values[i] * self->values[i]);
def_vec_op(ln, log(self->values[i]));
def_vec_op(ln1p, log(1 + self->values[i]));
def_vec_op(reciproc, 1.0 / (self->values[i]));
def_vec_op_func(exp);

def_vec_op_func(sin);
def_vec_op_func(sinh);
def_vec_op_func(asin);
def_vec_op_func(asinh);
def_vec_op_func(cos);
def_vec_op_func(cosh);
def_vec_op_func(acos);
def_vec_op_func(acosh);
def_vec_op_func(tan);
def_vec_op_func(tanh);
def_vec_op_func(atan);
def_vec_op_func(atanh);

int _vec_iter_closure(lua_State *L) {
  Vector *v = lua_touserdata(L, 1);          // immutable iter state
  lua_Integer cur_idx = lua_tointeger(L, 2); // mutable iter state

  if (cur_idx >= v->len) {
    return 0;
  }

  lua_pushinteger(L, cur_idx + 1);       // mutable iter state
  lua_pushnumber(L, v->values[cur_idx]); // extra values
  return 2;
}

int vec_iter(lua_State *L) {
  luaL_checkudata(L, 1, vector_mt_name);

  lua_pushcfunction(L, &_vec_iter_closure); // iter function
  lua_pushvalue(L, 1); // immutable state (vector)
  lua_pushinteger(L, 0); // mutable state (index)
  return 3;
}

int vec_add_into(lua_State *L) {
  Vector *out = NULL;
  if (lua_gettop(L) > 2) {
    out = luaL_checkudata(L, 3, vector_mt_name);
  }

  if (lua_isnumber(L, 1)) {
    lua_Number scalar = lua_tonumber(L, 1);
    Vector *v = luaL_checkudata(L, 2, vector_mt_name);
    if (out == NULL) {
      out = v;
    }
    _vec_broadcast_add_into(L, v, scalar, out);
    return 1; // out is on top of the stack no matter the branch path

  } else if (lua_isnumber(L, 2)) {
    lua_Number scalar = lua_tonumber(L, 2);
    Vector *v = luaL_checkudata(L, 1, vector_mt_name);
    if (out == NULL) {
      out = v;
      lua_pushvalue(L, 1);
    }
    _vec_broadcast_add_into(L, v, scalar, out);
    return 1; // out is on top of the stack no matter the branch path

  } else {
    Vector *v1 = luaL_checkudata(L, 1, vector_mt_name);
    Vector *v2 = luaL_checkudata(L, 2, vector_mt_name);
    if (out == NULL) {
      out = v1;
      lua_pushvalue(L, 1);
    } // else out on top of stack already
    _vec_xpsy_into(L, v1, 1, v2, out);
    return 1;
  }
}

int vec_add(lua_State *L) {
  if (lua_isnumber(L, 1)) {
    lua_Number scalar = lua_tonumber(L, 1);
    Vector *v = luaL_checkudata(L, 2, vector_mt_name);
    Vector *out = _vec_push_new(L, v->len);
    _vec_broadcast_add_into(L, v, scalar, out);
    return 1;

  } else if (lua_isnumber(L, 2)) {
    lua_Number scalar = lua_tonumber(L, 2);
    Vector *v = luaL_checkudata(L, 1, vector_mt_name);
    Vector *out = _vec_push_new(L, v->len);
    _vec_broadcast_add_into(L, v, scalar, out);
    return 1;

  } else {
    Vector *v1 = luaL_checkudata(L, 1, vector_mt_name);
    Vector *v2 = luaL_checkudata(L, 2, vector_mt_name);
    Vector *out = _vec_push_new(L, v1->len);
    _vec_xpsy_into(L, v1, 1, v2, out);
    return 1;
  }
}

int vec_sub_into(lua_State *L) {
  Vector *out = NULL;
  if (lua_gettop(L) > 2) {
    out = luaL_checkudata(L, 3, vector_mt_name);
  }

  if (lua_isnumber(L, 1)) {
    lua_Number scalar = lua_tonumber(L, 1);
    Vector *v = luaL_checkudata(L, 2, vector_mt_name);
    if (out == NULL) {
      out = v;
    }
    _vec_broadcast_add_into(L, v, -scalar, out);
    return 1; // out is on top of the stack no matter the branch path

  } else if (lua_isnumber(L, 2)) {
    lua_Number scalar = lua_tonumber(L, 2);
    Vector *v = luaL_checkudata(L, 1, vector_mt_name);
    if (out == NULL) {
      out = v;
      lua_pushvalue(L, 1);
    }
    _vec_broadcast_add_into(L, v, -scalar, out);
    return 1; // out is on top of the stack no matter the branch path

  } else {
    Vector *v1 = luaL_checkudata(L, 1, vector_mt_name);
    Vector *v2 = luaL_checkudata(L, 2, vector_mt_name);
    if (out == NULL) {
      out = v1;
      lua_pushvalue(L, 1);
    } // else out on top of stack already
    _vec_xpsy_into(L, v1, -1, v2, out);
    return 1;
  }
}

int vec_sub(lua_State *L) {
  if (lua_isnumber(L, 1)) {
    lua_Number scalar = lua_tonumber(L, 1);
    Vector *v = luaL_checkudata(L, 2, vector_mt_name);
    Vector *out = _vec_push_new(L, v->len);
    _vec_broadcast_add_into(L, v, -scalar, out);
    return 1;

  } else if (lua_isnumber(L, 2)) {
    lua_Number scalar = lua_tonumber(L, 2);
    Vector *v = luaL_checkudata(L, 1, vector_mt_name);
    Vector *out = _vec_push_new(L, v->len);
    _vec_broadcast_add_into(L, v, -scalar, out);
    return 1;

  } else {
    Vector *v1 = luaL_checkudata(L, 1, vector_mt_name);
    Vector *v2 = luaL_checkudata(L, 2, vector_mt_name);
    Vector *out = _vec_push_new(L, v1->len);
    _vec_xpsy_into(L, v1, -1, v2, out);
    return 1;
  }
}

int vec_mul_into(lua_State *L) {
  Vector *out = NULL;
  if (lua_gettop(L) > 2) {
    out = luaL_checkudata(L, 3, vector_mt_name);
  }

  if (lua_isnumber(L, 1)) {
    lua_Number scalar = lua_tonumber(L, 1);
    Vector *v = luaL_checkudata(L, 2, vector_mt_name);
    if (out == NULL) {
      out = v;
    }
    _vec_scale_into(L, v, scalar, out);
    return 1; // out is on top of the stack no matter the branch path

  } else if (lua_isnumber(L, 2)) {
    lua_Number scalar = lua_tonumber(L, 2);
    Vector *v = luaL_checkudata(L, 1, vector_mt_name);
    if (out == NULL) {
      out = v;
      lua_pushvalue(L, 1);
    }
    _vec_scale_into(L, v, scalar, out);
    return 1; // out is on top of the stack no matter the branch path

  } else {
    Vector *v1 = luaL_checkudata(L, 1, vector_mt_name);
    Vector *v2 = luaL_checkudata(L, 2, vector_mt_name);
    if (out == NULL) {
      out = v1;
      lua_pushvalue(L, 1);
    } // else out on top of stack already
    _vec_hadamard_product_into(L, v1, v2, out);
    return 1;
  }
}

int vec_mul(lua_State *L) {
  if (lua_isnumber(L, 1)) {
    lua_Number scalar = lua_tonumber(L, 1);
    Vector *v = luaL_checkudata(L, 2, vector_mt_name);
    Vector *out = _vec_push_new(L, v->len);
    _vec_scale_into(L, v, scalar, out);
    return 1;

  } else if (lua_isnumber(L, 2)) {
    lua_Number scalar = lua_tonumber(L, 2);
    Vector *v = luaL_checkudata(L, 1, vector_mt_name);
    Vector *out = _vec_push_new(L, v->len);
    _vec_scale_into(L, v, scalar, out);
    return 1;

  } else {
    Vector *v1 = luaL_checkudata(L, 1, vector_mt_name);
    Vector *v2 = luaL_checkudata(L, 2, vector_mt_name);
    Vector *out = _vec_push_new(L, v1->len);
    _vec_hadamard_product_into(L, v1, v2, out);
    return 1;
  }
}

int vec_div_into(lua_State *L) {
  Vector *out = NULL;
  if (lua_gettop(L) > 2) {
    out = luaL_checkudata(L, 3, vector_mt_name);
  }

  if (lua_isnumber(L, 1)) {
    lua_Number scalar = lua_tonumber(L, 1);
    Vector *v = luaL_checkudata(L, 2, vector_mt_name);
    if (out == NULL) {
      out = v;
    }
    _vec_elmwise_div_scalar_into(L, scalar, v, out);
    return 1; // out is on top of the stack no matter the branch path

  } else if (lua_isnumber(L, 2)) {
    lua_Number scalar = lua_tonumber(L, 2);
    Vector *v = luaL_checkudata(L, 1, vector_mt_name);
    if (out == NULL) {
      out = v;
      lua_pushvalue(L, 1);
    }
    _vec_scale_reciproc_into(L, v, scalar, out);
    return 1; // out is on top of the stack no matter the branch path

  } else {
    Vector *v1 = luaL_checkudata(L, 1, vector_mt_name);
    Vector *v2 = luaL_checkudata(L, 2, vector_mt_name);
    if (out == NULL) {
      out = v1;
      lua_pushvalue(L, 1);
    } // else out on top of stack already
    _vec_elmwise_div_into(L, v1, v2, out);
    return 1;
  }
}

int vec_div(lua_State *L) {
  if (lua_isnumber(L, 1)) {
    lua_Number scalar = lua_tonumber(L, 1);
    const Vector *v = luaL_checkudata(L, 2, vector_mt_name);
    Vector *out = _vec_push_new(L, v->len);
    _vec_elmwise_div_scalar_into(L, scalar, v, out);
    return 1;

  } else if (lua_isnumber(L, 2)) {
    lua_Number scalar = lua_tonumber(L, 2);
    const Vector *v = luaL_checkudata(L, 1, vector_mt_name);
    Vector *out = _vec_push_new(L, v->len);
    _vec_scale_reciproc_into(L, v, scalar, out);
    return 1;

  } else {
    const Vector *v1 = luaL_checkudata(L, 1, vector_mt_name);
    const Vector *v2 = luaL_checkudata(L, 2, vector_mt_name);
    Vector *out = _vec_push_new(L, v1->len);
    _vec_elmwise_div_into(L, v1, v2, out);
    return 1;
  }
}

int vec_pow_into(lua_State *L) {
  Vector *out = NULL;
  if (lua_gettop(L) > 2) {
    out = luaL_checkudata(L, 3, vector_mt_name);
  }

  if (lua_isnumber(L, 1)) {
    lua_Number scalar = lua_tonumber(L, 1);
    Vector *v = luaL_checkudata(L, 2, vector_mt_name);
    if (out == NULL) {
      out = v;
    }
    _vec_broadcast_pow_rev_into(L, scalar, v, out);
    return 1; // out is on top of the stack no matter the branch path

  } else if (lua_isnumber(L, 2)) {
    lua_Number scalar = lua_tonumber(L, 2);
    Vector *v = luaL_checkudata(L, 1, vector_mt_name);
    if (out == NULL) {
      out = v;
      lua_pushvalue(L, 1);
    }
    _vec_broadcast_pow_into(L, v, scalar, out);
    return 1; // out is on top of the stack no matter the branch path

  } else {
    Vector *v1 = luaL_checkudata(L, 1, vector_mt_name);
    Vector *v2 = luaL_checkudata(L, 2, vector_mt_name);
    if (out == NULL) {
      out = v1;
      lua_pushvalue(L, 1);
    } // else out on top of stack already
    _vec_pow_into(L, v1, v2, out);
    return 1;
  }
}

int vec_pow(lua_State *L) {
  if (lua_isnumber(L, 1)) {
    lua_Number scalar = lua_tonumber(L, 1);
    const Vector *v = luaL_checkudata(L, 2, vector_mt_name);
    Vector *out = _vec_push_new(L, v->len);
    _vec_broadcast_pow_rev_into(L, scalar, v, out);
    return 1;

  } else if (lua_isnumber(L, 2)) {
    lua_Number scalar = lua_tonumber(L, 2);
    const Vector *v = luaL_checkudata(L, 1, vector_mt_name);
    Vector *out = _vec_push_new(L, v->len);
    _vec_broadcast_pow_into(L, v, scalar, out);
    return 1;

  } else {
    const Vector *v1 = luaL_checkudata(L, 1, vector_mt_name);
    const Vector *v2 = luaL_checkudata(L, 2, vector_mt_name);
    Vector *out = _vec_push_new(L, v1->len);
    _vec_pow_into(L, v1, v2, out);
    return 1;
  }
}

int vec_neg(lua_State *L) {
  lua_pushcfunction(L, &vec_scale);
  lua_pushvalue(L, 1);
  lua_pushnumber(L, -1);
  lua_call(L, 2, 1);
  return 1;
}

int vec_neg_into(lua_State *L) {
  int nargs = lua_gettop(L);
  lua_pushcfunction(L, &vec_scale_into);
  lua_pushvalue(L, 1);
  lua_pushnumber(L, -1);
  if (nargs > 1) {
    lua_pushvalue(L, 2);
  }
  lua_call(L, nargs + 1, 1);
  return 1;
}

int vec__gc(lua_State *L) {
  Vector *v = luaL_checkudata(L, 1, vector_mt_name);
  free(v->values);
  return 0;
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
  {"dup", &vec_dup},
  {"dup_into", &vec_dup_into},

  {"add", &vec_add},
  {"add_into", &vec_add_into},
  {"sub", &vec_sub},
  {"sub_into", &vec_sub_into},
  {"mul", &vec_mul},
  {"mul_into", &vec_mul_into},
  {"div", &vec_div},
  {"div_into", &vec_div_into},
  {"pow", &vec_pow},
  {"pow_into", &vec_pow_into},
  {"neg", &vec_neg},
  {"neg_into", &vec_neg_into},
  {"reciproc", &vec_reciproc},
  {"reciproc_into", &vec_reciproc_into},

  {"at", &vec_at},
  {"iter", &vec_iter},
  {"psy", &vec_psy},
  {"psy_into", &vec_psy_into},
  {"scale", &vec_scale},
  {"scale_into", &vec_scale_into},
  {"hadamard", &vec_hadamard_product},
  {"hadamard_into", &vec_hadamard_product},

  {"sq", &vec_sq},
  {"sq_into", &vec_sq_into},
  {"square", &vec_sq},
  {"square_into", &vec_sq_into},
  {"cb", &vec_cb},
  {"cb_into", &vec_cb_into},
  {"cube", &vec_cb},
  {"cube_into", &vec_cb_into},

  {"sqrt", &vec_sqrt},
  {"sqrt_into", &vec_sqrt_into},
  {"cbrt", &vec_cbrt},
  {"cbrt_into", &vec_cbrt_into},

  {"exp", &vec_exp},
  {"exp_into", &vec_exp_into},
  {"ln", &vec_ln},
  {"ln_into", &vec_ln_into},
  {"ln1p", &vec_ln1p},
  {"ln1p_into", &vec_ln1p_into},

  {"sin", &vec_sin},
  {"sin_into", &vec_sin_into},
  {"sinh", &vec_sinh},
  {"sinh_into", &vec_sinh_into},
  {"asin", &vec_asin},
  {"asin_into", &vec_asin_into},
  {"asinh", &vec_asinh},
  {"asinh_into", &vec_asinh_into},

  {"cos", &vec_cos},
  {"cos_into", &vec_cos_into},
  {"cosh", &vec_cosh},
  {"cosh_into", &vec_cosh_into},
  {"acos", &vec_acos},
  {"acos_into", &vec_acos_into},
  {"acosh", &vec_acosh},
  {"acosh_into", &vec_acosh_into},

  {"tan", &vec_tan},
  {"tan_into", &vec_tan_into},
  {"tanh", &vec_tanh},
  {"tanh_into", &vec_tanh_into},
  {"atan", &vec_atan},
  {"atan_into", &vec_atan_into},
  {"atanh", &vec_atanh},
  {"atanh_into", &vec_atanh_into},

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
