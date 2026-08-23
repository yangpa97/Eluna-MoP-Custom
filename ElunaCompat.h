/*
 * Copyright (C) 2010 - 2024 Eluna Lua Engine <https://elunaluaengine.github.io/>
 * This program is free software licensed under GPL version 3
 * Please see the included DOCS/LICENSE.md for more information
 */

#ifndef ELUNACOMPAT_H
#define ELUNACOMPAT_H

extern "C"
{
#include "lua.h"
#include "lauxlib.h"
#if __has_include(<luajit.h>)
#include <luajit.h>
#endif
};

/* Compatibility layer for compiling with Lua 5.1 or LuaJIT */
#if LUA_VERSION_NUM == 501
    int luaL_getsubtable(lua_State* L, int i, const char* name);
    const char* luaL_tolstring(lua_State* L, int idx, size_t* len);
    int lua_absindex(lua_State* L, int i);
    #define lua_pushglobaltable(L) \
        lua_pushvalue((L), LUA_GLOBALSINDEX)
    #define lua_rawlen(L, idx) \
        lua_objlen(L, idx)
    #define lua_pushunsigned(L, u) \
        lua_pushinteger(L, u)
    #define lua_load(L, buf_read, dec_buf, str, NULL) \
        lua_load(L, buf_read, dec_buf, str)

#if !defined LUAJIT_VERSION
    void* luaL_testudata(lua_State* L, int index, const char* tname);
    void luaL_setmetatable(lua_State* L, const char* tname);
    #define luaL_setfuncs(L, l, n) luaL_register(L, NULL, l)
#endif
#endif

#if LUA_VERSION_NUM > 502
    #define lua_dump(L, writer, data) \
        lua_dump(L, writer, data, 0)
    #define lua_pushunsigned(L, u) \
        lua_pushinteger(L, u)
#endif

// ─── PORTABILIDAD SKYFIRE (2026-08-22) ───────────────────────────────────────
// El motor se probo contra un checkout LIMPIO de ProjectSkyfire/SkyFire_548 y
// fallo por cuatro cosas que nuestro fork tiene y upstream no. Tres se resuelven
// aqui detectandolas en tiempo de compilacion, sin macros que el usuario tenga
// que poner bien:
//
//   WorldSession::IsBot()            viene del modulo playerbots
//   ResultSet::GetFieldMetadata()    nombres/tipos de columna en la capa DB
//   urand()                          upstream lo quito en 2025
//
// Con SFINAE el mismo fuente compila en los dos cores y hace lo correcto en cada
// uno: sin playerbots nadie es bot, sin metadatos GetRow devuelve claves por
// indice, y urand es nuestro.
#include <cstdint>
#include <random>
#include <string>
#include <type_traits>

namespace ElunaCompat {

// --- bots ---
template <typename S>
auto SessionIsBot(S const* s, int) -> decltype(s->IsBot()) { return s->IsBot(); }
template <typename S>
bool SessionIsBot(S const*, long) { return false; }

// --- metadatos de columna ---
template <typename R>
auto FieldAlias(R const* r, uint32_t i, int) -> decltype(std::string(r->GetFieldMetadata(i).Alias)) {
  return r->GetFieldMetadata(i).Alias;
}
template <typename R>
std::string FieldAlias(R const*, uint32_t, long) { return std::string(); }

// --- especializacion (MoP): Player::LearnSpecialization no existe en todos los
// SkyFire; los anteriores a ago-2026 no la tienen. Sin ella SetSpecialization
// no hace nada en vez de romper la compilacion. ---
template <typename P>
auto LearnSpec(P *p, uint32_t id, int) -> decltype(p->LearnSpecialization(id), bool()) {
  p->LearnSpecialization(id);
  return true;
}
template <typename P>
bool LearnSpec(P *, uint32_t, long) { return false; }

// --- urand ---
inline uint32_t urand(uint32_t min, uint32_t max) {
  static thread_local std::mt19937 gen{std::random_device{}()};
  if (max < min) std::swap(min, max);
  return std::uniform_int_distribution<uint32_t>(min, max)(gen);
}

} // namespace ElunaCompat

#endif
