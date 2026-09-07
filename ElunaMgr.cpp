/*
* Copyright (C) 2010 - 2025 Eluna Lua Engine <https://elunaluaengine.github.io/>
* This program is free software licensed under GPL version 3
* Please see the included DOCS/LICENSE.md for more information
*/

#include "ElunaMgr.h"
#include "LuaEngine.h"

bool ElunaMgr::_destruido = false;

ElunaMgr::ElunaMgr()
{
}

ElunaMgr* ElunaMgr::instance()
{
    static ElunaMgr instance;
    return &instance;
}

ElunaMgr::~ElunaMgr()
{
    // La bandera PRIMERO: a partir de aqui cualquier ~ElunaInfo tardio que
    // llame a Destroy() se convierte en un no-op en vez de tocar un mapa
    // destruido. Luego se sueltan los Eluna con el objeto aun entero, en vez
    // de dejarlo para la destruccion implicita del miembro.
    _destruido = true;
    _elunaMap.clear();
}

void ElunaMgr::Create(Map* map, ElunaInfo const& info)
{
    if (_destruido)
        return;

    // If already exists, do nothing
    bool keyExists = info.IsValid() && (_elunaMap.find(info.key) != _elunaMap.end());
    if (keyExists)
        return;

    _elunaMap.emplace(info.key, std::make_unique<Eluna>(map));
}

Eluna* ElunaMgr::Get(ElunaInfoKey key) const
{
    if (_destruido)
        return nullptr;

    auto it = _elunaMap.find(key);
    if (it != _elunaMap.end())
        return it->second.get();

    return nullptr;
}

Eluna* ElunaMgr::Get(ElunaInfo const& info) const
{
    return Get(info.key);
}

void ElunaMgr::Destroy(ElunaInfoKey key)
{
    if (_destruido)
        return;

    _elunaMap.erase(key);
}

void ElunaMgr::Destroy(ElunaInfo const& info)
{
    Destroy(info.key);
}

ElunaInfo::~ElunaInfo()
{
    if (IsValid() && sElunaMgr)
        sElunaMgr->Destroy(key);
}

bool ElunaInfo::IsValid() const
{
    return key.IsValid();
}

bool ElunaInfo::IsGlobal() const
{
    return key.IsGlobal();
}

uint32 ElunaInfo::GetMapId() const
{
    return key.GetMapId();
}

uint32 ElunaInfo::GetInstanceId() const
{
    return key.GetInstanceId();
}

Eluna* ElunaInfo::GetEluna() const
{
    if (IsValid() && sElunaMgr)
        return sElunaMgr->Get(key);

    return nullptr;
}
