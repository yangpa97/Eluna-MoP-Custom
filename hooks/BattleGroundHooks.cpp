/*
 * Copyright (C) 2010 - 2024 Eluna Lua Engine
 * <https://elunaluaengine.github.io/> This program is free software licensed
 * under GPL version 3 Please see the included DOCS/LICENSE.md for more
 * information
 */

#include "Battlegrounds/Battleground.h"
#include "BindingMap.h"
#include "ElunaTemplate.h"
#include "HookHelpers.h"
#include "Hooks.h"
#include "LuaEngine.h"

using namespace Hooks;

#define START_HOOK(EVENT)                                                      \
  auto binding = GetBinding<EventKey<BGEvents>>(REGTYPE_BG);                   \
  auto key = EventKey<BGEvents>(EVENT);                                        \
  if (!binding->HasBindingsFor(key))                                           \
    return;

void Eluna::OnBGStart(Battleground *bg, BattlegroundTypeId bgId,
                      uint32 instanceId) {
  START_HOOK(BG_EVENT_ON_START);
  HookPush(bg);
  HookPush(uint32(bgId));
  HookPush(instanceId);
  CallAllFunctions(binding, key);
}

void Eluna::OnBGEnd(Battleground *bg, BattlegroundTypeId bgId,
                    uint32 instanceId, TeamId winner) {
  START_HOOK(BG_EVENT_ON_END);
  HookPush(bg);
  HookPush(uint32(bgId));
  HookPush(instanceId);
  HookPush(winner);
  CallAllFunctions(binding, key);
}

void Eluna::OnBGCreate(Battleground *bg, BattlegroundTypeId bgId,
                       uint32 instanceId) {
  START_HOOK(BG_EVENT_ON_CREATE);
  HookPush(bg);
  HookPush(uint32(bgId));
  HookPush(instanceId);
  CallAllFunctions(binding, key);
}

void Eluna::OnBGDestroy(Battleground *bg, BattlegroundTypeId bgId,
                        uint32 instanceId) {
  START_HOOK(BG_EVENT_ON_PRE_DESTROY);
  HookPush(bg);
  HookPush(uint32(bgId));
  HookPush(instanceId);
  CallAllFunctions(binding, key);
}
