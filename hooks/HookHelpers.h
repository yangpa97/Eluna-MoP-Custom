/*
 * Copyright (C) 2010 - 2024 Eluna Lua Engine <https://elunaluaengine.github.io/>
 * This program is free software licensed under GPL version 3
 * Please see the included DOCS/LICENSE.md for more information
 */

#ifndef _HOOK_HELPERS_H
#define _HOOK_HELPERS_H

#include "LuaEngine.h"
#include "ElunaUtility.h"

#include "ElunaConfig.h"
#include "Player.h"
#include "WorldSession.h"

/*
 * REGLA DE LOS BOTS (auditoria 2026-08-21, hallazgo H2).
 *
 * Los bots del modulo playerbots son jugadores con sesion sin socket. Los
 * scripts Lua (saludos, watchers de zona, #pet, menus) estan escritos para
 * humanos, y con 40 bots llenarian el mundo de ruido -- o peor: 40 bots
 * hablando en el chat le dan trabajo al daemon de Ollama.
 *
 * Un bot NO dispara los hooks cuyo SUJETO es el jugador: las familias Player,
 * Item, Gossip, el uso y las misiones de gameobject, aceptar grupo, los de
 * miembro de hermandad y subir/bajar de vehiculo.
 *
 * Un bot SI dispara los hooks cuyo sujeto es OTRA entidad aunque el bot sea
 * parte del evento: criatura (un boss en Lua tiene que ver que un bot le pega),
 * instancia, mapa, mundo, BG, y los de grupo/hermandad que van por guid (el
 * script vigila al grupo, no al bot). Filtrar ahi romperia esos scripts.
 *
 * Eluna.BotsDisparanHooks = 1 desactiva el filtro entero.
 */
static inline bool ElunaSkipBot(Player const* p) {
  return p && p->GetSession() && ElunaCompat::SessionIsBot(p->GetSession(), 0) &&
         !sElunaConfig->BotsFireHooks();
}
static inline bool ElunaSkipBot(Unit const* u) {
  return u && u->GetTypeId() == TypeID::TYPEID_PLAYER &&
         ElunaSkipBot(static_cast<Player const*>(u));
}


/*
 * Sets up the stack so that event handlers can be called.
 *
 * Returns the number of functions that were pushed onto the stack.
 */
template<typename K1, typename K2>
int Eluna::SetupStack(BindingMap<K1>* bindings1, BindingMap<K2>* bindings2, const K1& key1, const K2& key2, int number_of_arguments)
{
    ASSERT(number_of_arguments == this->push_counter);
    ASSERT(key1.event_id == key2.event_id);
    // Stack: [arguments]

    HookPush(key1.event_id);
    this->push_counter = 0;
    ++number_of_arguments;
    // Stack: [arguments], event_id

    int arguments_top = lua_gettop(L);
    int first_argument_index = arguments_top - number_of_arguments + 1;
    ASSERT(arguments_top >= number_of_arguments);

    lua_insert(L, first_argument_index);
    // Stack: event_id, [arguments]

    bindings1->PushRefsFor(key1);
    if (bindings2)
        bindings2->PushRefsFor(key2);
    // Stack: event_id, [arguments], [functions]

    int number_of_functions = lua_gettop(L) - arguments_top;
    return number_of_functions;
}

/*
 * Replace one of the arguments pushed before `SetupStack` with a new value.
 */
template<typename T>
void Eluna::ReplaceArgument(T value, uint8 index)
{
    ASSERT(index < lua_gettop(L) && index > 0);
    // Stack: event_id, [arguments], [functions], [results]

    Push(value);
    // Stack: event_id, [arguments], [functions], [results], value

    lua_replace(L, index + 1);
    // Stack: event_id, [arguments and value], [functions], [results]
}

/*
 * Call all event handlers registered to the event ID/entry combination and ignore any results.
 */
template<typename K1, typename K2>
void Eluna::CallAllFunctions(BindingMap<K1>* bindings1, BindingMap<K2>* bindings2, const K1& key1, const K2& key2)
{
    int number_of_arguments = this->push_counter;
    // Stack: [arguments]

    int number_of_functions = SetupStack(bindings1, bindings2, key1, key2, number_of_arguments);
    // Stack: event_id, [arguments], [functions]

    while (number_of_functions > 0)
    {
        CallOneFunction(number_of_functions, number_of_arguments, 0);
        --number_of_functions;
        // Stack: event_id, [arguments], [functions - 1]
    }
    // Stack: event_id, [arguments]

    CleanUpStack(number_of_arguments);
    // Stack: (empty)
}

/*
 * Call all event handlers registered to the event ID/entry combination,
 *   and returns `default_value` if ALL event handlers returned `default_value`,
 *   otherwise returns the opposite of `default_value`.
 */
template<typename K1, typename K2>
bool Eluna::CallAllFunctionsBool(BindingMap<K1>* bindings1, BindingMap<K2>* bindings2, const K1& key1, const K2& key2, bool default_value/* = false*/)
{
    bool result = default_value;
    // Note: number_of_arguments here does not count in eventID, which is pushed in SetupStack
    int number_of_arguments = this->push_counter;
    // Stack: [arguments]

    int number_of_functions = SetupStack(bindings1, bindings2, key1, key2, number_of_arguments);
    // Stack: event_id, [arguments], [functions]

    while (number_of_functions > 0)
    {
        int r = CallOneFunction(number_of_functions, number_of_arguments, 1);
        --number_of_functions;
        // Stack: event_id, [arguments], [functions - 1], result

        if (lua_isboolean(L, r) && (lua_toboolean(L, r) == 1) != default_value)
            result = !default_value;

        lua_pop(L, 1);
        // Stack: event_id, [arguments], [functions - 1]
    }
    // Stack: event_id, [arguments]

    CleanUpStack(number_of_arguments);
    // Stack: (empty)
    return result;
}

#endif // _HOOK_HELPERS_H
