# Adaptaciones de Eluna para SkyFire MoP 5.4.8

## Cambios en ObjectGuid
- TrinityCore: `guid.GetEntry()`
- SkyFire: `guid.GetCounter()` (Check usage, SkyFire 5.4.8 often splits GUIDs differently)
- TrinityCore: `guid.GetHigh()`
- SkyFire: `guid.GetHigh()`
- **New**: `ObjectGuid` appears to be a `uint64` typedef in some contexts or lacks `Empty` static method. Use `0` for empty/null guid checks where appropriate.

## Cambios en Database
- TrinityCore: `PreparedQueryResult`
- SkyFire: `QueryResult` (SkyFire often uses raw QueryResult for scripts or has different typedefs)

## Cambios en GameObject
- TrinityCore: `GameObject::Create(...)`
- SkyFire: `GameObject::Create(...)` (Signature differences in arguments)

## Cambios en Creature
- TrinityCore: `ThreatManager::GetThreatList()`
- SkyFire: `ThreatManager::getThreatList()` (lowercase method name)
- **New**: `SetRegenerateHealth` -> `setRegeneratingHealth`
- **New**: `GetWanderDistance` -> `GetRespawnRadius`
- **New**: `GetCurrentWaypointInfo` -> `GetCurrentWaypointID` (logic adaptation needed)
- **New**: `GetSpellHistory` -> Access via `m_CreatureSpellCooldowns` or similar if direct getter missing.

## Cambios en Player
- **New**: `ChatHandler::_ParseCommands` -> `ChatHandler::ParseCommands`

## Cambios en lua_data (Object.h)
- Standardized usage of `lua_data` for Eluna binding storage.

## Otros patrones identificados:
- **Group Methods**: Standardized Icons and member flags.
- **Guild Methods**: APIs standardized and registered.
- **Object.h**: Common base adaptations.

## Cambios en Macros Globales
- **New**: `GetTemplate` -> Se eliminó la macro `#define GetTemplate GetProto` en `ElunaUtility.h` ya que Skyfire MoP usa `GetTemplate()` nativamente.

Voy a actualizar este documento a medida que encuentre más patrones durante la compilación.
