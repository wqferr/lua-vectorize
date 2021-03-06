#include "lauxlib.h"
#include "lua.h"
#include <errno.h>
#include <math.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "vector.h"

#include "vectorize_compat.h"

const char vector_lib_mt_name[] = "liblua-vectorize";

const uint8_t intsize = sizeof(lua_Integer);
const uint8_t numbersize = sizeof(lua_Number);

static inline void _vec_check_oob(lua_State *L, int idx, lua_Integer len) {
  // idx is the 0-based index!
  if (idx < 0) {
    luaL_error(L, "Expected positive integer, got %d", idx + 1);
  } else if (idx >= len) {
    luaL_error(
      L, "Index out of bounds: %d (vector has length %d)", idx + 1, len);
  }
}

static inline void
_vec_check_same_len(lua_State *L, const Vector *x, const Vector *y) {
  if (x->len != y->len) {
    luaL_error(
      L, "Vectors must have the same length (%d != %d)", x->len, y->len);
  }
}

int vec_new(lua_State *L) {
  lua_Integer len;
  lua_Number *values;
  Vector *v;

  len = luaL_checkinteger(L, 1);
  lua_pop(L, 1);
  if (len <= 0) {
    return luaL_error(L, "Expected positive integer for size, got %d", len);
  }

  values = calloc(len, sizeof(*values));
  if (values == NULL) {
    return luaL_error(L, "Could not allocate vector");
  }

  v = newudata(L, sizeof(*v));

  setmetatable(L, vector_mt_name);

  v->len = len;
  v->values = values;
  return 1;
}

static inline Vector *_vec_push_new(lua_State *L, lua_Integer len) {
  lua_pushcfunction(L, &vec_new);
  lua_pushinteger(L, len);
  lua_call(L, 1, 1);

  return lua_touserdata(L, -1);
}

int vec_from(lua_State *L) {
  lua_Integer len = luaL_len(L, 1);
  Vector *v = _vec_push_new(L, len);
  for (lua_Integer i = 1; i <= len; i++) {
    lua_pushinteger(L, i);
    lua_gettable(L, 1);
    if (!lua_isnumber(L, -1)) {
      return luaL_error(
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

  for (lua_Integer i = 0; i < len; i++) {
    new->values[i] = start + ((i * step_num) / step_den);
  }

  return 1;
}

int vec_ones(lua_State *L) {
  vec_new(L);
  Vector *new = lua_touserdata(L, -1);
  for (lua_Integer i = 0; i < new->len; i++) {
    new->values[i] = 1;
  }
  return 1;
}

int vec_basis(lua_State *L) {
  lua_Integer len = luaL_checkinteger(L, 1);
  lua_Integer onepos = luaL_checkinteger(L, 2) - 1;
  lua_pop(L, 2);
  _vec_check_oob(L, onepos, len);

  Vector *new = _vec_push_new(L, len);
  new->values[onepos] = 1;

  return 1;
}

int vec_save(lua_State *L) {
  Vector *self = luaL_checkudata(L, 1, vector_mt_name);
  const char *filename = luaL_checkstring(L, 2);
  FILE *fp = fopen(filename, "wb+");

  if (fp == NULL) {
    return luaL_error(L, "Could not open file %s for writing.", filename);
  }

  // save architecture info (size of a lua integer)
  if (fwrite(&intsize, sizeof(intsize), 1, fp) == 0) {
    fclose(fp);
    return luaL_error(
      L,
      "Could not write architecture information to file (lua_Integer size).\n"
      "errno: %d\n"
      "%s",
      errno,
      strerror(errno));
  }

  // save lua compiled info (size of lua number)
  if (fwrite(&numbersize, sizeof(numbersize), 1, fp) == 0) {
    fclose(fp);
    return luaL_error(
      L,
      "Could not write architecture information to file (lua_Number size).\n"
      "errno: %d\n"
      "%s",
      errno,
      strerror(errno));
  }

  // save vector length
  if (fwrite(&self->len, sizeof(self->len), 1, fp) == 0) {
    fclose(fp);
    return luaL_error(
      L,
      "Could not write vector length to file.\n"
      "errno: %d\n"
      "%s",
      errno,
      strerror(errno));
  }

  // save vector data
  if (fwrite(self->values, sizeof(*self->values), self->len, fp) == 0) {
    fclose(fp);
    return luaL_error(
      L,
      "Could not write whole vector contents to file.\n"
      "errno: %d\n"
      "%s",
      errno,
      strerror(errno));
  }

  fclose(fp);

  return 0;
}

// TODO vec_savetxt

int vec_load(lua_State *L) {
  const char *filename = luaL_checkstring(L, 1);
  FILE *fp = fopen(filename, "rb");
  if (fp == NULL) {
    return luaL_error(L, "Could not open file %s for reading.", filename);
  }

  uint8_t load_intsize;
  if (fread(&load_intsize, sizeof(load_intsize), 1, fp) == 0) {
    fclose(fp);
    return luaL_error(
      L,
      "Corrupted file: could not read architecture information (lua_Integer "
      "size).\n"
      "errno: %d\n"
      "%s",
      errno,
      strerror(errno));
  }
  if (load_intsize != intsize) {
    fclose(fp);
    return luaL_error(
      L,
      "Incompatible architectures: vector was saved in a machine with "
      "lua_Integer size "
      "%d, this machine has lua_Integer size %d.",
      load_intsize,
      intsize);
  }
  uint8_t load_numbersize;
  if (fread(&load_numbersize, sizeof(load_numbersize), 1, fp) == 0) {
    fclose(fp);
    return luaL_error(
      L,
      "Corrupted file: could not read architecture information (lua_Number "
      "size).\n"
      "errno: %d\n"
      "%s",
      errno,
      strerror(errno));
  }
  if (load_numbersize != numbersize) {
    fclose(fp);
    return luaL_error(
      L,
      "Incompatible architectures: vector was saved in a machine with "
      "lua_Number size "
      "%d, this machine has lua_Number size %d.",
      load_numbersize,
      numbersize);
  }

  // By this point, we know that the architectures match

  lua_Integer len;
  if (fread(&len, sizeof(len), 1, fp) == 0) {
    fclose(fp);
    return luaL_error(
      L,
      "Could not read vector length from file.\n"
      "errno: %d\n"
      "%s",
      errno,
      strerror(errno));
  }

  Vector *new = _vec_push_new(L, len);
  if (((lua_Integer)fread(new->values, numbersize, len, fp)) < len) {
    fclose(fp);
    return luaL_error(
      L,
      "Could not read whole vector. Was the file truncated?\n"
      "errno: %d\n"
      "%s",
      errno,
      strerror(errno));
  }

  char dummy;
  if (fread(&dummy, sizeof(dummy), 1, fp) != 0) {
    fclose(fp);
    return luaL_error(
      L,
      "File has additional data after end of vector. Is this really a vector "
      "file?");
  }

  fclose(fp);
  return 1;
}

// TODO vec_loadtxt

int vec_reset(lua_State *L) {
  Vector *self = luaL_checkudata(L, 1, vector_mt_name);
  memset(self->values, 0, self->len * sizeof(lua_Number));
  lua_settop(L, 1);
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
  lua_settop(L, 2);
  return 1;
}

int vec_at(lua_State *L) {
  Vector *v = luaL_checkudata(L, 1, vector_mt_name);
  lua_Integer idx = lua_tointeger(L, 2) - 1;
  _vec_check_oob(L, idx, v->len);
  lua_pushnumber(L, v->values[idx]);

  return 1;
}

int vec_sum(lua_State *L) {
  Vector *self = luaL_checkudata(L, 1, vector_mt_name);
  lua_Number total = 0;
  for (lua_Integer i = 0; i < self->len; i++) {
    total += self->values[i];
  }

  lua_pushnumber(L, total);
  return 1;
}

int vec_norm(lua_State *L) {
  Vector *self = luaL_checkudata(L, 1, vector_mt_name);
  lua_Number total = 0;
  for (lua_Integer i = 0; i < self->len; i++) {
    total += self->values[i] * self->values[i];
  }

  lua_pushnumber(L, sqrt(total));
  return 1;
}

int vec_norm2(lua_State *L) {
  Vector *self = luaL_checkudata(L, 1, vector_mt_name);
  lua_Number total = 0;
  for (lua_Integer i = 0; i < self->len; i++) {
    total += self->values[i] * self->values[i];
  }

  lua_pushnumber(L, total);
  return 1;
}

int vec_normalize_into(lua_State *L) {
  Vector *self = luaL_checkudata(L, 1, vector_mt_name);
  lua_Number norm = 0;
  Vector *out;

  if (lua_gettop(L) > 1) {
    out = luaL_checkudata(L, 2, vector_mt_name);
    _vec_check_same_len(L, self, out);
    lua_settop(L, 2);
  } else {
    out = self;
  }

  for (lua_Integer i = 0; i < self->len; i++) {
    norm += self->values[i] * self->values[i];
  }
  norm = sqrt(norm);
  for (lua_Integer i = 0; i < out->len; i++) {
    out->values[i] = self->values[i] / norm;
  }

  return 1;
}

int vec_normalize(lua_State *L) {
  Vector *self = luaL_checkudata(L, 1, vector_mt_name);
  Vector *new = _vec_push_new(L, self->len);
  lua_Number norm = 0;
  for (lua_Integer i = 0; i < self->len; i++) {
    norm += self->values[i] * self->values[i];
  }
  norm = sqrt(norm);
  for (lua_Integer i = 0; i < new->len; i++) {
    new->values[i] = self->values[i] / norm;
  }
  return 1;
}

int vec_trapz(lua_State *L) {
  Vector *y = luaL_checkudata(L, 1, vector_mt_name);
  Vector *x = luaL_checkudata(L, 2, vector_mt_name);
  lua_Number total = 0;
  _vec_check_same_len(L, y, x);

  for (lua_Integer i = 1; i < y->len; i++) {
    lua_Number dx = (x->values[i] - x->values[i - 1]);
    total += ((y->values[i] + y->values[i - 1]) * dx) / 2;
  }
  lua_pushnumber(L, total);
  return 1;
}

int vec__index(lua_State *L) {
  if (lua_isinteger(L, 2)) {
    // integer indexing
    return vec_at(L);
  } else {
    // anything else falls back to the lib
    lua_pushvalue(L, 2);
    lua_rawget(L, lua_upvalueindex(1));
    return 1;
  }
}

int vec__newindex(lua_State *L) {
  Vector *v = luaL_checkudata(L, 1, vector_mt_name);
  lua_Integer idx = luaL_checkinteger(L, 2) - 1;
  _vec_check_oob(L, idx, v->len);

  v->values[idx] = luaL_checknumber(L, 3);
  return 0;
}

int vec__tostring(lua_State *L) {
  Vector *v = luaL_checkudata(L, 1, vector_mt_name);
  luaL_Buffer b;
  luaL_buffinit(L, &b);

  luaL_addstring(&b, "[");
  for (lua_Integer i = 0; i < v->len; i++) {
    lua_pushnumber(L, v->values[i]);
    luaL_addvalue(&b);
    if (i < v->len - 1) {
      luaL_addstring(&b, ", ");
    } else {
      luaL_addstring(&b, "]");
    }
  }
  luaL_pushresult(&b);
  return 1;
}

int vec__len(lua_State *L) {
  Vector *v = luaL_checkudata(L, 1, vector_mt_name);
  lua_pushinteger(L, v->len);
  return 1;
}

static inline void _vec_broadcast_add_into(
  lua_State *L, Vector *v, lua_Number scalar, Vector *out) {
  _vec_check_same_len(L, v, out);
  for (lua_Integer i = 0; i < out->len; i++) {
    out->values[i] = v->values[i] + scalar;
  }
}

static inline void _vec_broadcast_rev_sub_into(
  lua_State *L, lua_Number scalar, Vector *v, Vector *out) {
  _vec_check_same_len(L, v, out);
  for (lua_Integer i = 0; i < out->len; i++) {
    out->values[i] = scalar - v->values[i];
  }
}

static inline void _vec_broadcast_pow_rev_into(
  lua_State *L, lua_Number base, const Vector *v, Vector *out) {
  _vec_check_same_len(L, v, out);
  for (lua_Integer i = 0; i < out->len; i++) {
    out->values[i] = pow(base, v->values[i]);
  }
}

static inline void _vec_broadcast_pow_into(
  lua_State *L, const Vector *v, lua_Number e, Vector *out) {
  _vec_check_same_len(L, v, out);
  for (lua_Integer i = 0; i < out->len; i++) {
    out->values[i] = pow(v->values[i], e);
  }
}

static inline void
_vec_pow_into(lua_State *L, const Vector *b, const Vector *e, Vector *out) {
  _vec_check_same_len(L, b, e);
  _vec_check_same_len(L, b, out);
  for (lua_Integer i = 0; i < out->len; i++) {
    out->values[i] = pow(b->values[i], e->values[i]);
  }
}

static inline void _vec_xpsy_into(
  lua_State *L, const Vector *x, lua_Number s, const Vector *y, Vector *out) {
  _vec_check_same_len(L, x, y);
  _vec_check_same_len(L, x, out);
  for (lua_Integer i = 0; i < out->len; i++) {
    out->values[i] = x->values[i] + s * y->values[i];
  }
}

static inline void _vec_hadamard_product_into(
  lua_State *L, const Vector *x, const Vector *y, Vector *out) {
  _vec_check_same_len(L, x, y);
  _vec_check_same_len(L, x, out);
  for (lua_Integer i = 0; i < out->len; i++) {
    out->values[i] = x->values[i] * y->values[i];
  }
}

static inline void
_vec_scale_into(lua_State *L, const Vector *v, lua_Number s, Vector *out) {
  _vec_check_same_len(L, v, out);
  for (lua_Integer i = 0; i < out->len; i++) {
    out->values[i] = v->values[i] * s;
  }
}

static inline void _vec_scale_reciproc_into(
  lua_State *L, const Vector *v, lua_Number s, Vector *out) {
  _vec_check_same_len(L, v, out);
  for (lua_Integer i = 0; i < out->len; i++) {
    out->values[i] = v->values[i] / s;
  }
}

static inline void _vec_elmwise_div_into(
  lua_State *L, const Vector *x, const Vector *y, Vector *out) {
  _vec_check_same_len(L, x, y);
  _vec_check_same_len(L, x, out);
  for (lua_Integer i = 0; i < out->len; i++) {
    out->values[i] = x->values[i] / y->values[i];
  }
}

static inline void _vec_elmwise_div_scalar_into(
  lua_State *L, lua_Number scalar, const Vector *x, Vector *out) {
  _vec_check_same_len(L, x, out);
  for (lua_Integer i = 0; i < out->len; i++) {
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
    _vec_check_same_len(L, self, out);
    lua_settop(L, 4);
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
    lua_settop(L, 3);
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

int vec_inner(lua_State *L) {
  Vector *a = luaL_checkudata(L, 1, vector_mt_name);
  Vector *b = luaL_checkudata(L, 2, vector_mt_name);
  lua_Number total = 0;
  _vec_check_same_len(L, a, b);

  for (lua_Integer i = 0; i < a->len; i++) {
    total += a->values[i] * b->values[i];
  }
  lua_pushnumber(L, total);
  return 1;
}

int vec_project_into(lua_State *L) {
  Vector *a = luaL_checkudata(L, 1, vector_mt_name);
  Vector *b = luaL_checkudata(L, 2, vector_mt_name);
  Vector *out;
  _vec_check_same_len(L, a, b);

  if (lua_gettop(L) > 2) {
    out = luaL_checkudata(L, 3, vector_mt_name);
    _vec_check_same_len(L, a, out);
    lua_settop(L, 3);
  } else {
    out = a;
    lua_pushvalue(L, 1);
  }

  lua_Number norm2b = 0;
  lua_Number ainnerb = 0;
  for (lua_Integer i = 0; i < a->len; i++) {
    ainnerb += a->values[i] * b->values[i];
    norm2b += b->values[i] * b->values[i];
  }

  for (lua_Integer i = 0; i < out->len; i++) {
    out->values[i] = (b->values[i] * ainnerb) / norm2b;
  }

  return 1;
}

int vec_project(lua_State *L) {
  Vector *a = luaL_checkudata(L, 1, vector_mt_name);
  Vector *b = luaL_checkudata(L, 2, vector_mt_name);
  _vec_check_same_len(L, a, b);

  Vector *new = _vec_push_new(L, a->len);

  lua_Number norm2b = 0;
  lua_Number ainnerb = 0;
  for (lua_Integer i = 0; i < a->len; i++) {
    ainnerb += a->values[i] * b->values[i];
    norm2b += b->values[i] * b->values[i];
  }

  for (lua_Integer i = 0; i < new->len; i++) {
    new->values[i] = (b->values[i] * ainnerb) / norm2b;
  }

  return 1;
}

int vec_cosine_similarity(lua_State *L) {
  Vector *a = luaL_checkudata(L, 1, vector_mt_name);
  Vector *b = luaL_checkudata(L, 2, vector_mt_name);
  _vec_check_same_len(L, a, b);
  lua_Number norm2a = 0, norm2b = 0;
  lua_Number ainnerb = 0;

  for (lua_Integer i = 0; i < a->len; i++) {
    norm2a += a->values[i] * a->values[i];
    norm2b += b->values[i] * b->values[i];
    ainnerb += a->values[i] * b->values[i];
  }

  // product inside sqrt to avoid loss of precision
  lua_pushnumber(L, ainnerb / (sqrt(norm2a * norm2b)));
  return 1;
}

int vec_scale_into(lua_State *L) {
  Vector *self = luaL_checkudata(L, 1, vector_mt_name);
  lua_Number scalar = luaL_checknumber(L, 2);
  Vector *out;

  if (lua_gettop(L) > 2) {
    out = luaL_checkudata(L, 3, vector_mt_name);
    _vec_check_same_len(L, self, out);
    lua_settop(L, 3);
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
      lua_settop(L, 2);                                                        \
    } else {                                                                   \
      out = self;                                                              \
    }                                                                          \
                                                                               \
    for (lua_Integer i = 0; i < self->len; i++) {                              \
      out->values[i] = (expr);                                                 \
    }                                                                          \
    return 1; /* out is already on the top of the stack */                     \
  }                                                                            \
                                                                               \
  int vec_##name(lua_State *L) {                                               \
    Vector *self = luaL_checkudata(L, 1, vector_mt_name);                      \
    Vector *out = _vec_push_new(L, self->len);                                 \
    for (lua_Integer i = 0; i < out->len; i++) {                               \
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

static inline int _vec_iter_closure(lua_State *L) {
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
  lua_pushvalue(L, 1);                      // immutable state (vector)
  lua_pushinteger(L, 0);                    // mutable state (index)
  return 3;
}

#define def_vec_binop_arith_into(name, exp_lscalar, exp_rscalar, exp_ewise)    \
  int vec_##name##_into(lua_State *L) {                                        \
    Vector *out = NULL;                                                        \
    if (lua_gettop(L) > 2) {                                                   \
      out = luaL_checkudata(L, 3, vector_mt_name);                             \
      lua_settop(L, 3);                                                        \
    }                                                                          \
    if (lua_isnumber(L, 1)) {                                                  \
      lua_Number scalar = lua_tonumber(L, 1);                                  \
      Vector *v = luaL_checkudata(L, 2, vector_mt_name);                       \
      if (out == NULL)                                                         \
        out = v;                                                               \
      exp_lscalar;                                                             \
      return 1;                                                                \
    } else if (lua_isnumber(L, 2)) {                                           \
      Vector *v = luaL_checkudata(L, 1, vector_mt_name);                       \
      lua_Number scalar = lua_tonumber(L, 2);                                  \
      if (out == NULL) {                                                       \
        out = v;                                                               \
        lua_pushvalue(L, 1);                                                   \
      }                                                                        \
      exp_rscalar;                                                             \
      return 1;                                                                \
    } else {                                                                   \
      Vector *v1 = luaL_checkudata(L, 1, vector_mt_name);                      \
      Vector *v2 = luaL_checkudata(L, 2, vector_mt_name);                      \
      if (out == NULL) {                                                       \
        out = v1;                                                              \
        lua_pushvalue(L, 1);                                                   \
      }                                                                        \
      exp_ewise;                                                               \
      return 1;                                                                \
    }                                                                          \
  }

#define def_vec_binop_arith_noninto(name)                                      \
  int vec_##name(lua_State *L) {                                               \
    Vector *v;                                                                 \
    lua_settop(L, 2);                                                          \
    if (lua_isnumber(L, 1)) {                                                  \
      v = luaL_checkudata(L, 2, vector_mt_name);                               \
    } else {                                                                   \
      v = luaL_checkudata(L, 1, vector_mt_name);                               \
    }                                                                          \
    _vec_push_new(L, v->len);                                                  \
    return vec_##name##_into(L);                                               \
  }

def_vec_binop_arith_into(
  add,
  _vec_broadcast_add_into(L, v, scalar, out),
  _vec_broadcast_add_into(L, v, scalar, out),
  _vec_xpsy_into(L, v1, 1, v2, out));

def_vec_binop_arith_into(
  sub,
  _vec_broadcast_rev_sub_into(L, scalar, v, out),
  _vec_broadcast_add_into(L, v, -scalar, out),
  _vec_xpsy_into(L, v1, -1, v2, out));

def_vec_binop_arith_into(
  mul,
  _vec_scale_into(L, v, scalar, out),
  _vec_scale_into(L, v, scalar, out),
  _vec_hadamard_product_into(L, v1, v2, out));

def_vec_binop_arith_into(
  div,
  _vec_elmwise_div_scalar_into(L, scalar, v, out),
  _vec_scale_reciproc_into(L, v, scalar, out),
  _vec_elmwise_div_into(L, v1, v2, out));

def_vec_binop_arith_into(
  pow,
  _vec_broadcast_pow_rev_into(L, scalar, v, out),
  _vec_broadcast_pow_into(L, v, scalar, out),
  _vec_pow_into(L, v1, v2, out));

int vec_neg_into(lua_State *L) {
  Vector *self = luaL_checkudata(L, 1, vector_mt_name);
  Vector *out;

  if (lua_gettop(L) > 1) {
    out = luaL_checkudata(L, 2, vector_mt_name);
    _vec_check_same_len(L, self, out);
    lua_settop(L, 2);
  } else {
    out = self;
    lua_pushvalue(L, 1);
  }

  for (lua_Integer i = 0; i < self->len; i++) {
    out->values[i] = -self->values[i];
  }

  return 1;
}

def_vec_binop_arith_noninto(add);
def_vec_binop_arith_noninto(sub);
def_vec_binop_arith_noninto(mul);
def_vec_binop_arith_noninto(div);
def_vec_binop_arith_noninto(pow);

int vec_neg(lua_State *L) {
  Vector *self = luaL_checkudata(L, 1, vector_mt_name);
  lua_settop(L, 1);
  _vec_push_new(L, self->len);
  return vec_neg_into(L);
}

int vec__gc(lua_State *L) {
  Vector *v = luaL_checkudata(L, 1, vector_mt_name);
  free(v->values);
  return 0;
}

static const luaL_Reg vec_mt_funcs[] = {
  {"__index", &vec__index},
  {"__newindex", &vec__newindex},
  {"__tostring", &vec__tostring},
  {"__gc", &vec__gc},
  {"__len", &vec__len},
  {"__add", &vec_add},
  {"__sub", &vec_sub},
  {"__mul", &vec_mul},
  {"__div", &vec_div},
  {"__pow", &vec_pow},
  {"__unm", &vec_neg},
  {NULL, NULL}};

void create_vector_metatable(lua_State *L) {
  int libstackidx = lua_gettop(L);
  luaL_newmetatable(L, vector_mt_name);
  lua_pushvalue(L, libstackidx);
  luaL_setfuncs(L, vec_mt_funcs, 1);
  lua_pop(L, 1);
}

const struct luaL_Reg vec_functions[] = {
  {"new", &vec_new},
  {"from", &vec_from},
  {"ones", &vec_ones},
  {"basis", &vec_basis},
  {"linspace", &vec_linspace},
  {"dup", &vec_dup},
  {"dup_", &vec_dup_into},
  {"save", &vec_save},
  {"load", &vec_load},
  {"reset", &vec_reset},

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
  {"dot", &vec_inner},
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

int vec_lib__call(lua_State *L) {
  lua_remove(L, 1); // remove "self" argument
  if (lua_isnumber(L, 1)) {
    return vec_new(L);
  } else {
    return vec_from(L);
  }
}

static void create_lib_metatable(lua_State *L) {
  luaL_newmetatable(L, vector_lib_mt_name);
  lua_pushcfunction(L, &vec_lib__call);
  lua_setfield(L, -2, "__call");
  lua_pop(L, 1);
}

extern int luaopen_vec(lua_State *L) {
  create_lib_metatable(L);

  luaL_newlib(L, vec_functions);
  setmetatable(L, vector_lib_mt_name);

  create_vector_metatable(L);

  return 1;
}
