/*
 * Copyright (C) 2010 - 2024 Eluna Lua Engine
 * <https://elunaluaengine.github.io/> This program is free software licensed
 * under GPL version 3 Please see the included DOCS/LICENSE.md for more
 * information
 */

// Eluna
#include "ElunaEventMgr.h"
#include "ElunaIncludes.h"
#include "ElunaTemplate.h"
#include "ElunaUtility.h"
#include "LuaEngine.h"

// Method includes
#include "CustomMethodsInterface.h"
#include "TrinityCore/AuraMethods.h"
#include "TrinityCore/BattleGroundMethods.h"
#include "TrinityCore/BigIntMethods.h"
#include "TrinityCore/CorpseMethods.h"
#include "TrinityCore/CreatureMethods.h"
#include "TrinityCore/ElunaQueryMethods.h"
#include "TrinityCore/GameObjectMethods.h"
#include "TrinityCore/GlobalMethods.h"
#include "TrinityCore/GroupMethods.h"
#include "TrinityCore/GuildMethods.h"
#include "TrinityCore/ItemMethods.h"
#include "TrinityCore/MapMethods.h"
#include "TrinityCore/ObjectMethods.h"
#include "TrinityCore/PlayerMethods.h"
#include "TrinityCore/QuestMethods.h"
#include "TrinityCore/SpellMethods.h"
#include "TrinityCore/UnitMethods.h"
#include "TrinityCore/VehicleMethods.h"
#include "TrinityCore/WorldObjectMethods.h"
#include "TrinityCore/WorldPacketMethods.h"

#define ELUNA_SET_METHODS(CLASS, TABLE)                                        \
  ElunaTemplate<CLASS>::SetMethods(E, TABLE, sizeof(TABLE) / sizeof(TABLE[0]))

void RegisterMethods(Eluna *E) {
  ELUNA_SET_METHODS(void, LuaGlobalFunctions::GlobalMethods);

  ElunaTemplate<Object>::Register(E, "Object");
  ELUNA_SET_METHODS(Object, LuaObject::ObjectMethods);

  ElunaTemplate<WorldObject>::Register(E, "WorldObject");
  ELUNA_SET_METHODS(WorldObject, LuaObject::ObjectMethods);
  ELUNA_SET_METHODS(WorldObject, LuaWorldObject::WorldObjectMethods);

  ElunaTemplate<Unit>::Register(E, "Unit");
  ELUNA_SET_METHODS(Unit, LuaObject::ObjectMethods);
  ELUNA_SET_METHODS(Unit, LuaWorldObject::WorldObjectMethods);
  ELUNA_SET_METHODS(Unit, LuaUnit::UnitMethods);

  ElunaTemplate<Player>::Register(E, "Player");
  ELUNA_SET_METHODS(Player, LuaObject::ObjectMethods);
  ELUNA_SET_METHODS(Player, LuaWorldObject::WorldObjectMethods);
  ELUNA_SET_METHODS(Player, LuaUnit::UnitMethods);
  ELUNA_SET_METHODS(Player, LuaPlayer::PlayerMethods);

  ElunaTemplate<Creature>::Register(E, "Creature");
  ELUNA_SET_METHODS(Creature, LuaObject::ObjectMethods);
  ELUNA_SET_METHODS(Creature, LuaWorldObject::WorldObjectMethods);
  ELUNA_SET_METHODS(Creature, LuaUnit::UnitMethods);
  ELUNA_SET_METHODS(Creature, LuaCreature::CreatureMethods);

  ElunaTemplate<GameObject>::Register(E, "GameObject");
  ELUNA_SET_METHODS(GameObject, LuaObject::ObjectMethods);
  ELUNA_SET_METHODS(GameObject, LuaWorldObject::WorldObjectMethods);
  ELUNA_SET_METHODS(GameObject, LuaGameObject::GameObjectMethods);

  ElunaTemplate<Corpse>::Register(E, "Corpse");
  ELUNA_SET_METHODS(Corpse, LuaObject::ObjectMethods);
  ELUNA_SET_METHODS(Corpse, LuaWorldObject::WorldObjectMethods);
  ELUNA_SET_METHODS(Corpse, LuaCorpse::CorpseMethods);

  ElunaTemplate<Item>::Register(E, "Item");
  ELUNA_SET_METHODS(Item, LuaObject::ObjectMethods);
  ELUNA_SET_METHODS(Item, LuaItem::ItemMethods);

#if ELUNA_EXPANSION >= EXP_WOTLK
  ElunaTemplate<Vehicle>::Register(E, "Vehicle");
  ELUNA_SET_METHODS(Vehicle, LuaVehicle::VehicleMethods);
#endif

  ElunaTemplate<Group>::Register(E, "Group");
  ELUNA_SET_METHODS(Group, LuaGroup::GroupMethods);

  ElunaTemplate<Guild>::Register(E, "Guild");
  ELUNA_SET_METHODS(Guild, LuaGuild::GuildMethods);

  ElunaTemplate<Aura>::Register(E, "Aura");
  ELUNA_SET_METHODS(Aura, LuaAura::AuraMethods);

  ElunaTemplate<Spell>::Register(E, "Spell");
  ELUNA_SET_METHODS(Spell, LuaSpell::SpellMethods);

  ElunaTemplate<Quest>::Register(E, "Quest");
  ELUNA_SET_METHODS(Quest, LuaQuest::QuestMethods);

  ElunaTemplate<Map>::Register(E, "Map");
  ELUNA_SET_METHODS(Map, LuaMap::MapMethods);

  ElunaTemplate<Battleground>::Register(E, "BattleGround");
  ELUNA_SET_METHODS(Battleground, LuaBattleGround::BattleGroundMethods);

  ElunaTemplate<WorldPacket>::Register(E, "WorldPacket");
  ELUNA_SET_METHODS(WorldPacket, LuaPacket::PacketMethods);

  ElunaTemplate<ElunaQuery>::Register(E, "ElunaQuery");
  ELUNA_SET_METHODS(ElunaQuery, LuaQuery::QueryMethods);

  ElunaTemplate<long long>::Register(E, "long long");
  ELUNA_SET_METHODS(long long, LuaBigInt::LongLongMethods);

  ElunaTemplate<unsigned long long>::Register(E, "unsigned long long");
  ELUNA_SET_METHODS(unsigned long long, LuaBigInt::ULongLongMethods);

  ElunaTemplate<ObjectGuid>::Register(E, "ObjectGuid");
  ELUNA_SET_METHODS(ObjectGuid, LuaBigInt::ObjectGuidMethods);

  LuaCustom::RegisterCustomMethods(E);

  LuaVal::Register(E->L);
}
