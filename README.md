### [![Eluna](docs/Eluna.png)](https://github.com/ElunaLuaEngine/Eluna)

**English** · [Español](README.es.md)

# Eluna for SkyFire — Mists of Pandaria 5.4.8

A port of the modern [Eluna Lua Engine](https://github.com/ElunaLuaEngine/Eluna) to
[SkyFire_548](https://github.com/ProjectSkyfire/SkyFire_548), the 5.4.8 emulator.

Eluna has supported MaNGOS, cMaNGOS and TrinityCore for years, but never Mists of
Pandaria: the only MoP-era Eluna in the wild is the 2010–2013 tree bundled with some
`pandaria_5.4.8` forks, which predates most of the current API. This repository is the
modern engine — current hooks, current method set — running on 5.4.8.

**Status:** in production on a private server since June 2026. 149 of the 150 hooks the
engine implements are wired to real call-sites in the core, and 18 automated in-game
scenarios pass against a live world. The one hook left is
`PLAYER_EVENT_ON_FREE_TALENT_POINTS_CHANGED`: in MoP free talent points only exist for
pets, and the hook's signature takes a `Player*`, so there is nothing to wire it to.

---

## Why you might want this

* **Scripting on 5.4.8 without touching C++.** Bosses, events, custom systems, web-shop
  integration — 900+ Lua methods and 150 hooks.
* **It is installable.** A single script applies 209 wiring blocks to a clean SkyFire
  checkout. No hand-merging, no reformatting noise (see [Installation](#installation)).
* **The claims are tested, not asserted.** See [Verification](#verification).

---

## Beyond upstream Eluna

This port is not a straight copy. Some of it is MoP-specific; some of it applies to any
core.

### New events

| Event | Signature | Notes |
|---|---|---|
| `UNIT_EVENT_ON_DAMAGE` (36) | `(attacker, victim, damage)` | **Return a number to replace the damage.** Not exposed by upstream Eluna |
| `UNIT_EVENT_ON_HEAL` (37) | `(healer, receiver, amount)` | Same, for healing |
| `PLAYER_EVENT_ON_SPEC_CHANGED` (55) | `(player, new, old)` | Covers all three paths: talent change, reset, and dual-spec toggle |
| `MAP_EVENT_ON_GRID_LOAD` (19) | `(map, gx, gy)` | Declared in Eluna for years, never implemented |
| `MAP_EVENT_ON_GRID_UNLOAD` (20) | `(map, gx, gy)` | As above. Fires on the *real* unload, not just the reload path |

`MAP_EVENT_ON_CREATE` was declared but never wired upstream either; it is wired here.

### New methods

* **MoP specialisation and currencies** — `Player:GetSpecialization([spec])` (the real
  `ChrSpecialization` id, not the index), `SetSpecialization(id)`, `GetCurrency(id)`,
  `ModifyCurrency(id, n)`, `HasCurrency(id, n)`.
* **`Player:InjectPacket(packet)`** *(unsafe)* — queues a client packet into a session as
  if the client had sent it, so a script can drive mail, trade, auctions, gossip or chat
  through the server's real handlers. Built for automated testing.
* **Packet bit-writing** — `WriteBit`, `WriteBits`, `FlushBits`, `WriteGuidMask`,
  `WriteGuidBytes`, `WriteBytes`. MoP packets pack fields into bit runs and split GUIDs
  into a mask plus a byte sequence; you cannot build one without these.
* **`GetOpcodeByName(name)`** — SkyFire's opcode enum is alphabetical and auto-numbered,
  so numeric opcodes cannot be hardcoded.
* **`RegisterVehicleEvent`** — missing from upstream Eluna, so the five vehicle hooks
  could not be registered at all.
* **`Map:SetWeather(zone, type, grade)`** — was an empty stub.

### Upstream bugs fixed here

* **`AUCTION_EVENT_ON_REMOVE` never fired.** Not on this core, and — from reading the
  hook and the call order — not on the other Eluna targets either. The hook resolves
  the auctioned `Item` through `GetAItem()`, but all three call-sites remove the item from
  the map *before* removing the auction, which is where the hook lived. So it always bailed
  out at its own `if (!owner || !item) return;` guard — silently, 100% of the time. Fixed
  by firing at each call-site while the item is still resolvable.
* **`ElunaQuery:GetRow()` collapsed every column into one key** on this core, because the
  `ResultSet` metadata returned `"unknown"` as the alias for all of them.

---

## Requirements and limits

Read this section before opening an issue.

* **SkyFire_548, current `main`.** Verified against upstream `0e0cfa60dd` and installed
  cleanly onto `936493b` (newer). Upstream itself now needs **g++ 14 / MSVC with C++23**
  and **Boost 1.91**.
* **Single Lua state.** One `lua_State` for the whole world, no locks. `MapUpdate.Threads`
  **must be 1**; the engine forces it to 1 and logs an error if Eluna is enabled with
  more, because the alternative is random crashes with no obvious cause.
* **Lua 5.2 / LuaJIT**, as upstream.
* **Object handles are scoped to the callback that produced them.** Store GUIDs and
  re-resolve at the top of each callback (`GetPlayerByGUID`, `map:GetWorldObject`). This is
  upstream behaviour, not something this port introduces, but it is the single most common
  source of `calling 'X' on bad self` and it bites everyone once.

---

## Installation

The wiring lives in 209 small blocks spread across 46 core files. A plain `.patch` is
useless here — entire files in this core have been run through `clang-format`, so a diff
buries the hooks under tens of thousands of reformatted lines. Instead, each block is
described by an **anchor**: a short sequence of neighbouring lines that exists verbatim in
upstream and is unique there. The installer finds each anchor in *your* checkout and
inserts the block beside it, whatever your formatting looks like.

```bash
git clone https://github.com/ProjectSkyfire/SkyFire_548.git
git clone https://github.com/yangpa97/Eluna-MoP-Custom.git \
          SkyFire_548/src/server/scripts/LuaEngine

cd SkyFire_548
python3 src/server/scripts/LuaEngine/patches/install_core_hooks.py --core "$PWD"
# → manifest built against upstream 0e0cfa60dd; 195 anchored blocks, 14 manual
#   blocks: 195 applied, 0 without an anchor   manual: 14
```

The 14 blocks the installer reports as "manual" are the ones with no usable anchor — a
whole new function, an initialiser list, a new file. The script writes those too; nothing
is left for you to paste by hand.

Then build as usual. The installer also adds `-DELUNA_SKYFIRE -DELUNA_EXPANSION=4` to
`src/server/game/CMakeLists.txt` and registers the engine in `src/server/scripts`.

It is **idempotent** — a block whose lines are already present is skipped — so you can
re-run it after pulling upstream. `--dry-run` reports what it would do without writing
anything, and `--lang es` switches the output to Spanish (it follows `LANG` otherwise).

---

## Configuration

Append to `worldserver.conf`:

```ini
Eluna.Enabled              = 1
Eluna.ScriptPath           = "lua_scripts"
Eluna.TraceBack            = 1     # full Lua traceback on error
Eluna.ReloadCommand        = 1     # enable ".reload eluna"
Eluna.ReloadSecurityLevel  = 3     # GM level required for it
Eluna.UseUnsafeMethods     = 1     # required for InjectPacket, raw DB queries
Eluna.OnlyOnMaps           = ""    # empty = all maps
Eluna.BotsFireHooks        = 0     # see below (the legacy key
                                   # Eluna.BotsDisparanHooks still works)
```

`Eluna.BotsFireHooks = 0` stops player-subject hooks from firing for playerbot sessions.
The rule is deliberate: a bot does **not** fire hooks whose subject is the player (chat,
save, item, gossip, vehicle boarding), but it **does** fire hooks whose subject is another
entity — a Lua boss must still see a bot hitting it. Set it to 1 if you have no bots.

---

## What a script looks like

```lua
-- Halve all damage taken by players below level 10, and log big hits.
-- ON_DAMAGE is global, so it lives in the server event family: RegisterServerEvent,
-- not RegisterUnitEvent.
RegisterServerEvent(36, function(event, attacker, victim, damage)
    local player = victim:ToPlayer()
    if player and player:GetLevel() < 10 then
        return math.floor(damage / 2)      -- returning a number replaces the damage
    end
    if damage > 100000 then
        print(("%s hit %s for %d"):format(attacker:GetName(), victim:GetName(), damage))
    end
end)

-- Give Valor Points when a player changes specialisation.
RegisterPlayerEvent(55, function(event, player, newSpec, oldSpec)
    player:ModifyCurrency(396, 100)        -- 396 = Valor Points
    player:SendBroadcastMessage(("Spec %d → %d"):format(oldSpec, newSpec))
end)
```

The [upstream API documentation](https://elunaluaengine.github.io/) applies to everything
inherited from Eluna; the tables above cover what is new here. If you are porting Eluna to
another 5.x core, [docs/PORTING_NOTES.md](docs/PORTING_NOTES.md) is the map of what had to
change and why.

---

## Verification

Every claim in this README corresponds to a check that was run.

| Check | Result |
|---|---|
| Build, Windows x64 (MSVC 2026, C++23) | 0 errors, both on this fork and on a clean upstream checkout |
| Build, Linux x86-64 (GCC 13, Ubuntu 24.04) | 0 errors — against a SkyFire tree that predates upstream's C++23 switch. Current upstream `main` needs g++ 14 and Boost 1.91, neither of which is packaged for Ubuntu 24.04 yet; that is upstream's requirement, not this engine's |
| Installer on a clean upstream checkout | 195/195 anchored blocks + 14 manual, 0 anchors missed — including on a *newer* upstream commit than the manifest targets |
| Line-level check of installed vs. source tree | every line of engine **code** lands in the same context; the only differences are in the comments |
| Server boot from that clean tree | `World initialized`, 34 scripts loaded, clean shutdown; also with `Eluna.Enabled=0` and with `MapUpdate.Threads=4` |
| In-game scenarios | **18/18** |

The in-game suite is a Lua harness that drives two playerbots and **checks game state, not
just that a hook fired**: the damage hook forces every hit to 1 and the wolf's health must
drop by exactly the number of hits; the auction test posts and cancels a real auction and
the item must leave the bag; the duel test deals 5,000,000 damage and the target must end
up at 1 HP, still alive. It covers damage, healing, groups, instances and grid loading, MoP
methods, database rows, guilds, weather, game events, server control, the player hook
family, chat/emote/addon, mail, gossip without an NPC, trade, duels, auctions and death.

Four of the fixes listed in this README were found by that harness, not by reading code.

---

## Credits and license

This is a fork of the [Eluna Lua Engine](https://github.com/ElunaLuaEngine/Eluna) by the
Eluna team and its contributors. The design and the overwhelming majority of the code are
theirs; this repository ports it to SkyFire 5.4.8 and extends it. It is **not affiliated
with or endorsed by** the Eluna project — please do not take issues you hit here to their
support channels.

Licensed under **GPLv3**, as upstream. See [LICENSE](LICENSE).

Bug reports and pull requests are welcome. If you put this on a live realm, tell me what
breaks: one server is not a sample. Issues are far more useful with your core commit and
the Lua that reproduces the problem.
