# Porting notes: TrinityCore → SkyFire 5.4.8

*Notas del port: TrinityCore → SkyFire 5.4.8. English below is the reference; the
Spanish summary at the end says the same thing.*

Eluna's TrinityCore backend is the closest thing to a SkyFire backend, so the port
started from it. These are the API differences that actually required a change in this
tree — each entry corresponds to code you can find in the repository, not to a guess.
If you are porting Eluna to another 5.x core, this is the map.

Everything below is guarded by `ELUNA_SKYFIRE`, so the TrinityCore, MaNGOS, cMaNGOS and
AzerothCore backends are untouched.

## Naming: same concept, different spelling

| TrinityCore | SkyFire 5.4.8 | Where |
|---|---|---|
| `ThreatManager::GetThreatList()` | `getThreatList()` | `CreatureMethods.h` |
| `SetRegenerateHealth()` | `setRegeneratingHealth()` | `CreatureMethods.h` |
| `GetWanderDistance()` | `GetRespawnRadius()` | `CreatureMethods.h` |
| `GetCurrentWaypointInfo()` | `GetCurrentWaypointID()` | `CreatureMethods.h` — returns the id alone, so the caller no longer unpacks a pair |
| `Player::LearnSpell()` | `Player::learnSpell()` | lowercase on this core |

## Types and containers

* **`ElunaQuery` is `QueryResult`**, an `AutoPtr`, not `PreparedQueryResult`
  (`ElunaUtility.h`). Anything that takes a query result by value has to respect that.
* **`ObjectGuid` is a struct over a `uint64`** (`ByteBuffer.h`) and `Object::GetGUID()`
  returns the raw `uint64`. Eluna pushes that as a boxed 64-bit value, which is why
  `CHECKVAL<ObjectGuid>` here accepts an `ObjectGuid`, a `uint64` **or** a plain number —
  without that, `GetPlayerByGUID(p:GetGUID())` fails, and it failed silently for months.
* **`GetTemplate` needs no macro.** Other cores alias it to `GetProto`; SkyFire has
  `GetTemplate()` natively, so the `#define` is disabled for this backend
  (`ElunaUtility.h`).

## Cooldowns

SkyFire has no `SpellHistory`. Creature cooldowns go through the creature itself:
`HasSpellCooldown(spellId)` and `GetCreatureSpellCooldownDelay(spellId)`
(`CreatureMethods.h`).

## Opcodes

The `Opcodes` enum is **alphabetical and auto-numbered**, so a numeric opcode means
nothing across builds and cannot be hardcoded in a script. That is what
`GetOpcodeByName("CMSG_...")` is for.

Two related traps, both real:

* Several opcodes are registered with number `0x0000` and `STATUS_UNHANDLED` — the
  dispatcher drops them before the handler runs. Of the six addon-chat opcodes, only
  `CMSG_MESSAGECHAT_ADDON_WHISPER` is live.
* MoP packets pack fields into bit runs and split GUIDs into a bit mask plus a byte
  sequence, so building one needs `WriteBits` / `WriteGuidMask` / `WriteGuidBytes`
  rather than plain writes.

## Core APIs that are not in every fork

Four APIs the engine used were not available everywhere, and are now detected at compile
time in `ElunaCompat.h` instead of being required:

| API | Missing where | Fallback |
|---|---|---|
| `WorldSession::IsBot()` | any core without a playerbot module | nobody is a bot |
| `ResultSet::GetFieldMetadata()` | upstream SkyFire | `GetRow()` keys rows by 1-based index instead of column name; the type still comes from `Field::GetType()` |
| `urand()` | removed from upstream in 2025 | `ElunaCompat::urand`, `<random>` |
| `Player::LearnSpecialization()` | SkyFire trees older than August 2026 | `SetSpecialization()` returns `false` |

The lesson, if you take one thing from this file: **build against a clean checkout of the
core you claim to support before claiming it.** Testing only against your own fork hides
every API your fork happens to have — and it hides how *old* a supported core may be.

## Threading

The engine runs a single `lua_State` with no locks, so `MapUpdate.Threads` must be 1.
Rather than documenting that and hoping, the core now forces it to 1 and logs an error
when Eluna is enabled with more.

---

## Resumen en español

Este documento recoge las diferencias de API que hubo que salvar para llevar Eluna de
TrinityCore a SkyFire 5.4.8. Cada entrada corresponde a un cambio real en el código, no a
una suposición; todo va detrás de `ELUNA_SKYFIRE`, así que los demás backends quedan
intactos.

* **Nombres distintos para lo mismo**: `getThreatList`, `setRegeneratingHealth`,
  `GetRespawnRadius`, `GetCurrentWaypointID`, `learnSpell` en minúscula.
* **Tipos**: `ElunaQuery` es `QueryResult` (un `AutoPtr`), `ObjectGuid` es una estructura
  sobre `uint64` y `GetGUID()` devuelve el entero crudo — por eso `CHECKVAL<ObjectGuid>`
  acepta las tres formas. `GetTemplate` es nativo y no necesita macro.
* **Enfriamientos**: no hay `SpellHistory`; se consultan en la propia criatura.
* **Opcodes**: el enum es alfabético y autonumerado, así que los números no se pueden
  fijar a mano (`GetOpcodeByName`). Algunos están registrados con número `0x0000` y estado
  «sin manejar»: el despachador los descarta antes del manejador.
* **APIs que no están en todos los forks**: `IsBot`, `GetFieldMetadata`, `urand` y
  `LearnSpecialization` se detectan en tiempo de compilación (`ElunaCompat.h`) en vez de
  exigirse.

La lección, si solo te llevas una: **compila contra un checkout limpio del core que dices
soportar antes de decirlo.** Probar solo contra tu propio fork esconde todas las APIs que
tu fork tiene por casualidad, y esconde también lo *antiguo* que puede ser un core al que
dices dar soporte.
