/*
 * Copyright (C) 2010 - 2024 Eluna Lua Engine
 * <https://elunaluaengine.github.io/> This program is free software licensed
 * under GPL version 3 Please see the included DOCS/LICENSE.md for more
 * information
 */

#include "BindingMap.h"
#include "ElunaIncludes.h"
#include "ElunaLoader.h"
#include "ElunaTemplate.h"
#include "HookHelpers.h"
#include "Hooks.h"
#include "LuaEngine.h"
#include <algorithm> // std::transform
#include <cstdlib>   // strtol

using namespace Hooks;



#define START_HOOK(EVENT)                                                      \
  if (!HasBindings(REGTYPE_PLAYER)) return;                                    \
  auto binding = GetBinding<EventKey<PlayerEvents>>(REGTYPE_PLAYER);           \
  auto key = EventKey<PlayerEvents>(EVENT);                                    \
  if (!binding->HasBindingsFor(key))                                           \
    return;

#define START_HOOK_WITH_RETVAL(EVENT, RETVAL)                                  \
  if (!HasBindings(REGTYPE_PLAYER)) return RETVAL;                             \
  auto binding = GetBinding<EventKey<PlayerEvents>>(REGTYPE_PLAYER);           \
  auto key = EventKey<PlayerEvents>(EVENT);                                    \
  if (!binding->HasBindingsFor(key))                                           \
    return RETVAL;

void Eluna::OnLearnTalents(Player *pPlayer, uint32 talentId, uint32 talentRank,
                           uint32 spellid) {
  START_HOOK(PLAYER_EVENT_ON_LEARN_TALENTS);
  if (ElunaSkipBot(pPlayer)) return;
  HookPush(pPlayer);
  HookPush(talentId);
  HookPush(talentRank);
  HookPush(spellid);
  CallAllFunctions(binding, key);
}

void Eluna::OnSkillChange(Player *pPlayer, uint32 skillId, uint32 skillValue) {
  START_HOOK(PLAYER_EVENT_ON_SKILL_CHANGE);
  if (ElunaSkipBot(pPlayer)) return;
  HookPush(pPlayer);
  HookPush(skillId);
  HookPush(skillValue);
  int valueIndex = lua_gettop(L) - 1;
  int n = SetupStack(binding, key, 3);

  while (n > 0) {
    int r = CallOneFunction(n--, 3, 1);

    if (lua_isnumber(L, r)) {
      skillValue = CHECKVAL<uint32>(r);
      // Update the stack for subsequent calls.
      ReplaceArgument(skillValue, valueIndex);
    }

    lua_pop(L, 1);
  }

  CleanUpStack(3);
}

void Eluna::OnLearnSpell(Player *pPlayer, uint32 spellId) {
  START_HOOK(PLAYER_EVENT_ON_LEARN_SPELL);
  if (ElunaSkipBot(pPlayer)) return;
  HookPush(pPlayer);
  HookPush(spellId);
  CallAllFunctions(binding, key);
}

bool Eluna::OnCommand(Player *player, const char *text) {
  // If from console, player is NULL
  if (sElunaConfig->IsReloadCommandEnabled() &&
      (!player || player->GetSession()->GetSecurity() >=
                      sElunaConfig->GetReloadSecurityLevel())) {
    std::string reload = text;
    std::transform(reload.begin(), reload.end(), reload.begin(), ::tolower);
    // SKYFIRE: ChatHandler::ParseCommands pasa el texto ANTES de quitar el
    // prefijo '.'/'!' — sin esto ".reload eluna" jamas matchea (find()==0).
    std::string::size_type cmdStart = reload.find_first_not_of(" \t.!");
    if (cmdStart == std::string::npos)
      cmdStart = reload.length();
    const std::string reload_command = "reload eluna";
    if (reload.compare(cmdStart, reload_command.length(), reload_command) ==
        0) {
      int mapId = RELOAD_ALL_STATES;
      std::string args = reload.substr(cmdStart + reload_command.length());
      if (!args.empty())
        mapId = strtol(args.c_str(), nullptr, 10);

      sElunaLoader->ReloadElunaForMap(mapId);

      // SKYFIRE: feedback visible — sin esto la recarga es silenciosa y parece
      // que el comando no funciona (la recarga real es asincrona, al terminar
      // de recompilar la cache del disco).
      if (player)
        ChatHandler(player->GetSession())
            .SendSysMessage("|cff00ff00[Eluna]|r Recarga de scripts Lua "
                            "iniciada (se aplica en ~1s).");
      else
        SF_LOG_INFO("eluna", "[Eluna] Recarga de scripts solicitada (consola).");

      return false;
    }
  }

  START_HOOK_WITH_RETVAL(PLAYER_EVENT_ON_COMMAND, true);
  if (ElunaSkipBot(player)) return true;
  HookPush(player);
  HookPush(text);
  return CallAllFunctionsBool(binding, key, true);
}

void Eluna::OnLootItem(Player *pPlayer, Item *pItem, uint32 count,
                       ObjectGuid guid) {
  START_HOOK(PLAYER_EVENT_ON_LOOT_ITEM);
  if (ElunaSkipBot(pPlayer)) return;
  HookPush(pPlayer);
  HookPush(pItem);
  HookPush(count);
  HookPush(guid);
  CallAllFunctions(binding, key);
}

void Eluna::OnLootMoney(Player *pPlayer, uint32 amount) {
  START_HOOK(PLAYER_EVENT_ON_LOOT_MONEY);
  if (ElunaSkipBot(pPlayer)) return;
  HookPush(pPlayer);
  HookPush(amount);
  CallAllFunctions(binding, key);
}

void Eluna::OnFirstLogin(Player *pPlayer) {
  START_HOOK(PLAYER_EVENT_ON_FIRST_LOGIN);
  if (ElunaSkipBot(pPlayer)) return;
  HookPush(pPlayer);
  CallAllFunctions(binding, key);
}

void Eluna::OnRepop(Player *pPlayer) {
  START_HOOK(PLAYER_EVENT_ON_REPOP);
  if (ElunaSkipBot(pPlayer)) return;
  HookPush(pPlayer);
  CallAllFunctions(binding, key);
}

void Eluna::OnResurrect(Player *pPlayer) {
  START_HOOK(PLAYER_EVENT_ON_RESURRECT);
  if (ElunaSkipBot(pPlayer)) return;
  HookPush(pPlayer);
  CallAllFunctions(binding, key);
}

void Eluna::OnQuestAbandon(Player *pPlayer, uint32 questId) {
  START_HOOK(PLAYER_EVENT_ON_QUEST_ABANDON);
  if (ElunaSkipBot(pPlayer)) return;
  HookPush(pPlayer);
  HookPush(questId);
  CallAllFunctions(binding, key);
}

void Eluna::OnQuestStatusChanged(Player *pPlayer, uint32 questId,
                                 uint8 status) {
  START_HOOK(PLAYER_EVENT_ON_QUEST_STATUS_CHANGED);
  if (ElunaSkipBot(pPlayer)) return;
  HookPush(pPlayer);
  HookPush(questId);
  HookPush(status);
  CallAllFunctions(binding, key);
}

void Eluna::OnEquip(Player *pPlayer, Item *pItem, uint8 bag, uint8 slot) {
  START_HOOK(PLAYER_EVENT_ON_EQUIP);
  if (ElunaSkipBot(pPlayer)) return;
  HookPush(pPlayer);
  HookPush(pItem);
  HookPush(bag);
  HookPush(slot);
  CallAllFunctions(binding, key);
}

InventoryResult Eluna::OnCanUseItem(const Player *pPlayer, uint32 itemEntry) {
  START_HOOK_WITH_RETVAL(PLAYER_EVENT_ON_CAN_USE_ITEM, EQUIP_ERR_OK);
  if (ElunaSkipBot(pPlayer)) return EQUIP_ERR_OK;
  InventoryResult result = EQUIP_ERR_OK;
  HookPush(pPlayer);
  HookPush(itemEntry);
  int n = SetupStack(binding, key, 2);

  while (n > 0) {
    int r = CallOneFunction(n--, 2, 1);

    if (lua_isnumber(L, r))
      result = (InventoryResult)CHECKVAL<uint32>(r);

    lua_pop(L, 1);
  }

  CleanUpStack(2);
  return result;
}
void Eluna::OnPlayerEnterCombat(Player *pPlayer, Unit *pEnemy) {
  START_HOOK(PLAYER_EVENT_ON_ENTER_COMBAT);
  if (ElunaSkipBot(pPlayer)) return;
  HookPush(pPlayer);
  HookPush(pEnemy);
  CallAllFunctions(binding, key);
}

void Eluna::OnPlayerLeaveCombat(Player *pPlayer) {
  START_HOOK(PLAYER_EVENT_ON_LEAVE_COMBAT);
  if (ElunaSkipBot(pPlayer)) return;
  HookPush(pPlayer);
  CallAllFunctions(binding, key);
}

void Eluna::OnPVPKill(Player *pKiller, Player *pKilled) {
  START_HOOK(PLAYER_EVENT_ON_KILL_PLAYER);
  if (ElunaSkipBot(pKiller)) return;
  HookPush(pKiller);
  HookPush(pKilled);
  CallAllFunctions(binding, key);
}

void Eluna::OnCreatureKill(Player *pKiller, Creature *pKilled) {
  START_HOOK(PLAYER_EVENT_ON_KILL_CREATURE);
  if (ElunaSkipBot(pKiller)) return;
  HookPush(pKiller);
  HookPush(pKilled);
  CallAllFunctions(binding, key);
}

void Eluna::OnPlayerKilledByCreature(Creature *pKiller, Player *pKilled) {
  START_HOOK(PLAYER_EVENT_ON_KILLED_BY_CREATURE);
  if (ElunaSkipBot(pKilled)) return;
  HookPush(pKiller);
  HookPush(pKilled);
  CallAllFunctions(binding, key);
}

void Eluna::OnPlayerKilledByEnvironment(Player *pKilled, uint8 damageType) {
  START_HOOK(PLAYER_EVENT_ON_ENVIRONMENTAL_DEATH);
  if (ElunaSkipBot(pKilled)) return;
  HookPush(pKilled);
  HookPush(damageType);
  CallAllFunctions(binding, key);
}

void Eluna::OnLevelChanged(Player *pPlayer, uint8 oldLevel) {
  START_HOOK(PLAYER_EVENT_ON_LEVEL_CHANGE);
  if (ElunaSkipBot(pPlayer)) return;
  HookPush(pPlayer);
  HookPush(oldLevel);
  CallAllFunctions(binding, key);
}

void Eluna::OnFreeTalentPointsChanged(Player *pPlayer, uint32 newPoints) {
  START_HOOK(PLAYER_EVENT_ON_TALENTS_CHANGE);
  if (ElunaSkipBot(pPlayer)) return;
  HookPush(pPlayer);
  HookPush(newPoints);
  CallAllFunctions(binding, key);
}

void Eluna::OnTalentsReset(Player *pPlayer, bool noCost) {
  START_HOOK(PLAYER_EVENT_ON_TALENTS_RESET);
  if (ElunaSkipBot(pPlayer)) return;
  HookPush(pPlayer);
  HookPush(noCost);
  CallAllFunctions(binding, key);
}

void Eluna::OnMoneyChanged(Player *pPlayer, int32 &amount) {
  START_HOOK(PLAYER_EVENT_ON_MONEY_CHANGE);
  if (ElunaSkipBot(pPlayer)) return;
  HookPush(pPlayer);
  HookPush(amount);
  int amountIndex = lua_gettop(L);
  int n = SetupStack(binding, key, 2);

  while (n > 0) {
    int r = CallOneFunction(n--, 2, 1);

    if (lua_isnumber(L, r)) {
      amount = CHECKVAL<int32>(r);
      // Update the stack for subsequent calls.
      ReplaceArgument(amount, amountIndex);
    }

    lua_pop(L, 1);
  }

  CleanUpStack(2);
}

#if ELUNA_EXPANSION >= EXP_CATA
void Eluna::OnMoneyChanged(Player *pPlayer, int64 &amount) {
  START_HOOK(PLAYER_EVENT_ON_MONEY_CHANGE);
  if (ElunaSkipBot(pPlayer)) return;
  HookPush(pPlayer);
  HookPush(amount);
  int amountIndex = lua_gettop(L);
  int n = SetupStack(binding, key, 2);

  while (n > 0) {
    int r = CallOneFunction(n--, 2, 1);

    if (lua_isnumber(L, r)) {
      amount = CHECKVAL<int32>(r);
      // Update the stack for subsequent calls.
      ReplaceArgument(amount, amountIndex);
    }

    lua_pop(L, 1);
  }

  CleanUpStack(2);
}
#endif

void Eluna::OnGiveXP(Player *pPlayer, uint32 &amount, Unit *pVictim) {
  START_HOOK(PLAYER_EVENT_ON_GIVE_XP);
  if (ElunaSkipBot(pPlayer)) return;
  HookPush(pPlayer);
  HookPush(amount);
  HookPush(pVictim);
  int amountIndex = lua_gettop(L) - 1;
  int n = SetupStack(binding, key, 3);

  while (n > 0) {
    int r = CallOneFunction(n--, 3, 1);

    if (lua_isnumber(L, r)) {
      amount = CHECKVAL<uint32>(r);
      // Update the stack for subsequent calls.
      ReplaceArgument(amount, amountIndex);
    }

    lua_pop(L, 1);
  }

  CleanUpStack(3);
}

void Eluna::OnReputationChange(Player *pPlayer, uint32 factionID,
                               int32 &standing, bool incremental) {
  START_HOOK(PLAYER_EVENT_ON_REPUTATION_CHANGE);
  if (ElunaSkipBot(pPlayer)) return;
  HookPush(pPlayer);
  HookPush(factionID);
  HookPush(standing);
  HookPush(incremental);
  int standingIndex = lua_gettop(L) - 1;
  int n = SetupStack(binding, key, 4);

  while (n > 0) {
    int r = CallOneFunction(n--, 4, 1);

    if (lua_isnumber(L, r)) {
      standing = CHECKVAL<int32>(r);
      // Update the stack for subsequent calls.
      ReplaceArgument(standing, standingIndex);
    }

    lua_pop(L, 1);
  }

  CleanUpStack(4);
}

void Eluna::OnDuelRequest(Player *pTarget, Player *pChallenger) {
  START_HOOK(PLAYER_EVENT_ON_DUEL_REQUEST);
  if (ElunaSkipBot(pTarget)) return;
  HookPush(pTarget);
  HookPush(pChallenger);
  CallAllFunctions(binding, key);
}

void Eluna::OnDuelStart(Player *pStarter, Player *pChallenger) {
  START_HOOK(PLAYER_EVENT_ON_DUEL_START);
  if (ElunaSkipBot(pStarter)) return;
  HookPush(pStarter);
  HookPush(pChallenger);
  CallAllFunctions(binding, key);
}

void Eluna::OnDuelEnd(Player *pWinner, Player *pLoser, DuelCompleteType type) {
  START_HOOK(PLAYER_EVENT_ON_DUEL_END);
  if (ElunaSkipBot(pWinner)) return;
  HookPush(pWinner);
  HookPush(pLoser);
  HookPush(static_cast<uint32>(type));
  CallAllFunctions(binding, key);
}

void Eluna::OnEmote(Player *pPlayer, uint32 emote) {
  START_HOOK(PLAYER_EVENT_ON_EMOTE);
  if (ElunaSkipBot(pPlayer)) return;
  HookPush(pPlayer);
  HookPush(emote);
  CallAllFunctions(binding, key);
}

void Eluna::OnTextEmote(Player *pPlayer, uint32 textEmote, uint32 emoteNum,
                        ObjectGuid guid) {
  START_HOOK(PLAYER_EVENT_ON_TEXT_EMOTE);
  if (ElunaSkipBot(pPlayer)) return;
  HookPush(pPlayer);
  HookPush(textEmote);
  HookPush(emoteNum);
  HookPush(guid);
  CallAllFunctions(binding, key);
}

void Eluna::OnSpellCast(Player *pPlayer, Spell *pSpell, bool skipCheck) {
  START_HOOK(PLAYER_EVENT_ON_SPELL_CAST);
  if (ElunaSkipBot(pPlayer)) return;
  HookPush(pPlayer);
  HookPush(pSpell);
  HookPush(skipCheck);
  CallAllFunctions(binding, key);
}

void Eluna::OnLogin(Player *pPlayer) {
  START_HOOK(PLAYER_EVENT_ON_LOGIN);
  if (ElunaSkipBot(pPlayer)) return;
  HookPush(pPlayer);
  CallAllFunctions(binding, key);
}

void Eluna::OnLogout(Player *pPlayer) {
  START_HOOK(PLAYER_EVENT_ON_LOGOUT);
  if (ElunaSkipBot(pPlayer)) return;
  HookPush(pPlayer);
  CallAllFunctions(binding, key);
}

void Eluna::OnCreate(Player *pPlayer) {
  START_HOOK(PLAYER_EVENT_ON_CHARACTER_CREATE);
  if (ElunaSkipBot(pPlayer)) return;
  HookPush(pPlayer);
  CallAllFunctions(binding, key);
}

void Eluna::OnDelete(uint32 guidlow) {
  START_HOOK(PLAYER_EVENT_ON_CHARACTER_DELETE);
  HookPush(guidlow);
  CallAllFunctions(binding, key);
}

void Eluna::OnSave(Player *pPlayer) {
  START_HOOK(PLAYER_EVENT_ON_SAVE);
  if (ElunaSkipBot(pPlayer)) return;
  HookPush(pPlayer);
  CallAllFunctions(binding, key);
}

void Eluna::OnBindToInstance(Player *pPlayer, Difficulty difficulty,
                             uint32 mapid, bool permanent) {
  START_HOOK(PLAYER_EVENT_ON_BIND_TO_INSTANCE);
  if (ElunaSkipBot(pPlayer)) return;
  HookPush(pPlayer);
  HookPush(difficulty);
  HookPush(mapid);
  HookPush(permanent);
  CallAllFunctions(binding, key);
}

void Eluna::OnUpdateZone(Player *pPlayer, uint32 newZone, uint32 newArea) {
  START_HOOK(PLAYER_EVENT_ON_UPDATE_ZONE);
  if (ElunaSkipBot(pPlayer)) return;
  HookPush(pPlayer);
  HookPush(newZone);
  HookPush(newArea);
  CallAllFunctions(binding, key);
}

void Eluna::OnUpdateArea(Player *pPlayer, uint32 oldArea, uint32 newArea) {
  START_HOOK(PLAYER_EVENT_ON_UPDATE_AREA);
  if (ElunaSkipBot(pPlayer)) return;
  HookPush(pPlayer);
  HookPush(oldArea);
  HookPush(newArea);
  CallAllFunctions(binding, key);
}

void Eluna::OnMapChanged(Player *player) {
  START_HOOK(PLAYER_EVENT_ON_MAP_CHANGE);
  if (ElunaSkipBot(player)) return;
  HookPush(player);
  CallAllFunctions(binding, key);
}

void Eluna::OnAchievementComplete(Player *player, uint32 achievementId) {
  START_HOOK(PLAYER_EVENT_ON_ACHIEVEMENT_COMPLETE);
  if (ElunaSkipBot(player)) return;
  HookPush(player);
  HookPush(achievementId);
  CallAllFunctions(binding, key);
}

bool Eluna::OnTradeInit(Player *trader, Player *tradee) {
  START_HOOK_WITH_RETVAL(PLAYER_EVENT_ON_TRADE_INIT, true);
  if (ElunaSkipBot(trader)) return true;
  HookPush(trader);
  HookPush(tradee);
  return CallAllFunctionsBool(binding, key, true);
}

bool Eluna::OnTradeAccept(Player *trader, Player *tradee) {
  START_HOOK_WITH_RETVAL(PLAYER_EVENT_ON_TRADE_ACCEPT, true);
  if (ElunaSkipBot(trader)) return true;
  HookPush(trader);
  HookPush(tradee);
  return CallAllFunctionsBool(binding, key, true);
}

bool Eluna::OnSendMail(Player *sender, ObjectGuid recipientGuid) {
  START_HOOK_WITH_RETVAL(PLAYER_EVENT_ON_SEND_MAIL, true);
  if (ElunaSkipBot(sender)) return true;
  HookPush(sender);
  HookPush(recipientGuid);
  return CallAllFunctionsBool(binding, key, true);
}

void Eluna::OnDiscoverArea(Player *player, uint32 area) {
  START_HOOK(PLAYER_EVENT_ON_DISCOVER_AREA);
  if (ElunaSkipBot(player)) return;
  HookPush(player);
  HookPush(area);
  CallAllFunctions(binding, key);
}

bool Eluna::OnChat(Player *pPlayer, uint32 type, uint32 lang,
                   std::string &msg) {
  if (ElunaSkipBot(pPlayer)) return true;
  if (lang == static_cast<uint32>(Language::LANG_ADDON))
    return OnAddonMessage(pPlayer, type, msg, NULL, NULL, NULL, NULL);

  START_HOOK_WITH_RETVAL(PLAYER_EVENT_ON_CHAT, true);
  bool result = true;
  HookPush(pPlayer);
  HookPush(msg);
  HookPush(type);
  HookPush(lang);
  int n = SetupStack(binding, key, 4);

  while (n > 0) {
    int r = CallOneFunction(n--, 4, 2);

    if (lua_isboolean(L, r + 0) && !lua_toboolean(L, r + 0))
      result = false;

    if (lua_isstring(L, r + 1))
      msg = std::string(lua_tostring(L, r + 1));

    lua_pop(L, 2);
  }

  CleanUpStack(4);
  return result;
}

bool Eluna::OnChat(Player *pPlayer, uint32 type, uint32 lang, std::string &msg,
                   Group *pGroup) {
  if (ElunaSkipBot(pPlayer)) return true;
  if (lang == static_cast<uint32>(Language::LANG_ADDON))
    return OnAddonMessage(pPlayer, type, msg, NULL, NULL, pGroup, NULL);

  START_HOOK_WITH_RETVAL(PLAYER_EVENT_ON_GROUP_CHAT, true);
  bool result = true;
  HookPush(pPlayer);
  HookPush(msg);
  HookPush(type);
  HookPush(lang);
  HookPush(pGroup);
  int n = SetupStack(binding, key, 5);

  while (n > 0) {
    int r = CallOneFunction(n--, 5, 2);

    if (lua_isboolean(L, r + 0) && !lua_toboolean(L, r + 0))
      result = false;

    if (lua_isstring(L, r + 1))
      msg = std::string(lua_tostring(L, r + 1));

    lua_pop(L, 2);
  }

  CleanUpStack(5);
  return result;
}

bool Eluna::OnChat(Player *pPlayer, uint32 type, uint32 lang, std::string &msg,
                   Guild *pGuild) {
  if (ElunaSkipBot(pPlayer)) return true;
  if (lang == static_cast<uint32>(Language::LANG_ADDON))
    return OnAddonMessage(pPlayer, type, msg, NULL, pGuild, NULL, NULL);

  START_HOOK_WITH_RETVAL(PLAYER_EVENT_ON_GUILD_CHAT, true);
  bool result = true;
  HookPush(pPlayer);
  HookPush(msg);
  HookPush(type);
  HookPush(lang);
  HookPush(pGuild);
  int n = SetupStack(binding, key, 5);

  while (n > 0) {
    int r = CallOneFunction(n--, 5, 2);

    if (lua_isboolean(L, r + 0) && !lua_toboolean(L, r + 0))
      result = false;

    if (lua_isstring(L, r + 1))
      msg = std::string(lua_tostring(L, r + 1));

    lua_pop(L, 2);
  }

  CleanUpStack(5);
  return result;
}

bool Eluna::OnChat(Player *pPlayer, uint32 type, uint32 lang, std::string &msg,
                   Channel *pChannel) {
  if (ElunaSkipBot(pPlayer)) return true;
  if (lang == static_cast<uint32>(Language::LANG_ADDON))
    return OnAddonMessage(pPlayer, type, msg, NULL, NULL, NULL, pChannel);

  START_HOOK_WITH_RETVAL(PLAYER_EVENT_ON_CHANNEL_CHAT, true);
  bool result = true;
  HookPush(pPlayer);
  HookPush(msg);
  HookPush(type);
  HookPush(lang);
  HookPush(pChannel->GetChannelId());
  int n = SetupStack(binding, key, 5);

  while (n > 0) {
    int r = CallOneFunction(n--, 5, 2);

    if (lua_isboolean(L, r + 0) && !lua_toboolean(L, r + 0))
      result = false;

    if (lua_isstring(L, r + 1))
      msg = std::string(lua_tostring(L, r + 1));

    lua_pop(L, 2);
  }

  CleanUpStack(5);
  return result;
}

bool Eluna::OnChat(Player *pPlayer, uint32 type, uint32 lang, std::string &msg,
                   Player *pReceiver) {
  if (ElunaSkipBot(pPlayer)) return true;
  if (lang == static_cast<uint32>(Language::LANG_ADDON))
    return OnAddonMessage(pPlayer, type, msg, pReceiver, NULL, NULL, NULL);

  START_HOOK_WITH_RETVAL(PLAYER_EVENT_ON_WHISPER, true);
  bool result = true;
  HookPush(pPlayer);
  HookPush(msg);
  HookPush(type);
  HookPush(lang);
  HookPush(pReceiver);
  int n = SetupStack(binding, key, 5);

  while (n > 0) {
    int r = CallOneFunction(n--, 5, 2);

    if (lua_isboolean(L, r + 0) && !lua_toboolean(L, r + 0))
      result = false;

    if (lua_isstring(L, r + 1))
      msg = std::string(lua_tostring(L, r + 1));

    lua_pop(L, 2);
  }

  CleanUpStack(5);
  return result;
}
