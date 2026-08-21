# Lua Library Sources
file(GLOB lualib_sources
    "${CMAKE_CURRENT_SOURCE_DIR}/deps/lualib/*.c"
)
# Exclude lua.c and luac.c
list(FILTER lualib_sources EXCLUDE REGEX "lua\\.c$")
list(FILTER lualib_sources EXCLUDE REGEX "luac\\.c$")

add_library(lualib STATIC ${lualib_sources})

# Define public include_directories(
target_include_directories(lualib PUBLIC
  ${CMAKE_SOURCE_DIR}/src/server/scripts/LuaEngine
  ${CMAKE_SOURCE_DIR}/src/server/game
  ${CMAKE_SOURCE_DIR}/src/server/shared
  ${CMAKE_SOURCE_DIR}/src/server/database
  ${CMAKE_SOURCE_DIR}/src/server/game/Miscellaneous
  ${CMAKE_SOURCE_DIR}/src/server/shared/Configuration
  ${CMAKE_SOURCE_DIR}/src/server/shared/Cryptography
  ${CMAKE_SOURCE_DIR}/src/server/shared/DataStores
  ${CMAKE_SOURCE_DIR}/src/server/shared/Database
  ${CMAKE_SOURCE_DIR}/src/server/shared/Debugging
  ${CMAKE_SOURCE_DIR}/src/server/shared/Dynamic
  ${CMAKE_SOURCE_DIR}/src/server/shared/Logging
  ${CMAKE_SOURCE_DIR}/src/server/shared/Packets
  ${CMAKE_SOURCE_DIR}/src/server/shared/Threading
  ${CMAKE_SOURCE_DIR}/src/server/shared/Utilities
  ${CMAKE_SOURCE_DIR}/src/server/game/DataStores
  ${CMAKE_SOURCE_DIR}/src/server/game/AI
  ${CMAKE_SOURCE_DIR}/src/server/game/Accounts
  ${CMAKE_SOURCE_DIR}/src/server/game/Achievements
  ${CMAKE_SOURCE_DIR}/src/server/game/Addons
  ${CMAKE_SOURCE_DIR}/src/server/game/AuctionHouse
  ${CMAKE_SOURCE_DIR}/src/server/game/BattlePet
  ${CMAKE_SOURCE_DIR}/src/server/game/Battlefield
  ${CMAKE_SOURCE_DIR}/src/server/game/Battlegrounds
  ${CMAKE_SOURCE_DIR}/src/server/game/BlackMarket
  ${CMAKE_SOURCE_DIR}/src/server/game/Boost
  ${CMAKE_SOURCE_DIR}/src/server/game/Calendar
  ${CMAKE_SOURCE_DIR}/src/server/game/Chat
  ${CMAKE_SOURCE_DIR}/src/server/game/Cinematic
  ${CMAKE_SOURCE_DIR}/src/server/game/Combat
  ${CMAKE_SOURCE_DIR}/src/server/game/Conditions
  ${CMAKE_SOURCE_DIR}/src/server/game/Chat
  ${CMAKE_SOURCE_DIR}/src/server/game/Chat/Channels
  ${CMAKE_SOURCE_DIR}/src/server/game/Cinematic
  ${CMAKE_SOURCE_DIR}/src/server/game/Entities
  ${CMAKE_SOURCE_DIR}/src/server/game/Entities/AreaTrigger
  ${CMAKE_SOURCE_DIR}/src/server/game/Entities/Corpse
  ${CMAKE_SOURCE_DIR}/src/server/game/Entities/Creature
  ${CMAKE_SOURCE_DIR}/src/server/game/Entities/DynamicObject
  ${CMAKE_SOURCE_DIR}/src/server/game/Entities/GameObject
  ${CMAKE_SOURCE_DIR}/src/server/game/Entities/Item
  ${CMAKE_SOURCE_DIR}/src/server/game/Entities/Item/Container
  ${CMAKE_SOURCE_DIR}/src/server/game/Entities/Object
  ${CMAKE_SOURCE_DIR}/src/server/game/Entities/Object/Updates
  ${CMAKE_SOURCE_DIR}/src/server/game/Entities/Pet
  ${CMAKE_SOURCE_DIR}/src/server/game/Entities/Player
  ${CMAKE_SOURCE_DIR}/src/server/game/Entities/Transport
  ${CMAKE_SOURCE_DIR}/src/server/game/Entities/Unit
  ${CMAKE_SOURCE_DIR}/src/server/game/Entities/Vehicle
  ${CMAKE_SOURCE_DIR}/src/server/game/Events
  ${CMAKE_SOURCE_DIR}/src/server/game/Globals
  ${CMAKE_SOURCE_DIR}/src/server/game/Grids
  ${CMAKE_SOURCE_DIR}/src/server/game/Groups
  ${CMAKE_SOURCE_DIR}/src/server/game/Guilds
  ${CMAKE_SOURCE_DIR}/src/server/game/Handlers
  ${CMAKE_SOURCE_DIR}/src/server/game/Instances
  ${CMAKE_SOURCE_DIR}/src/server/game/Loot
  ${CMAKE_SOURCE_DIR}/src/server/game/Mails
  ${CMAKE_SOURCE_DIR}/src/server/game/Maps
  ${CMAKE_SOURCE_DIR}/src/server/game/Miscellaneous
  ${CMAKE_SOURCE_DIR}/src/server/game/Movement
  ${CMAKE_SOURCE_DIR}/src/server/game/Movement/MovementGenerators
  ${CMAKE_SOURCE_DIR}/src/server/game/Movement/Spline
  ${CMAKE_SOURCE_DIR}/src/server/game/Movement/Waypoints
  ${CMAKE_SOURCE_DIR}/src/server/game/Accounts
  ${CMAKE_SOURCE_DIR}/src/server/game/Achievement
  ${CMAKE_SOURCE_DIR}/src/server/game/Addons
  ${CMAKE_SOURCE_DIR}/src/server/game/AI/CoreAI
  ${CMAKE_SOURCE_DIR}/src/server/game/AI/EventAI
  ${CMAKE_SOURCE_DIR}/src/server/game/AI/ScriptedAI
  ${CMAKE_SOURCE_DIR}/src/server/game/AI/SmartScripts
  ${CMAKE_SOURCE_DIR}/src/server/game/Areagroup
  ${CMAKE_SOURCE_DIR}/src/server/game/Arena
  ${CMAKE_SOURCE_DIR}/src/server/game/AuctionHouse
  ${CMAKE_SOURCE_DIR}/src/server/game/Battlefield
  ${CMAKE_SOURCE_DIR}/src/server/game/Battlegrounds
  ${CMAKE_SOURCE_DIR}/src/server/game/Battlegrounds/Arena
  ${CMAKE_SOURCE_DIR}/src/server/game/Battlegrounds/Zones
  ${CMAKE_SOURCE_DIR}/src/server/game/Calendar
  ${CMAKE_SOURCE_DIR}/src/server/game/Chat
  ${CMAKE_SOURCE_DIR}/src/server/game/Chat/Channels
  ${CMAKE_SOURCE_DIR}/src/server/game/Chat/Commands
  ${CMAKE_SOURCE_DIR}/src/server/game/Combat
  ${CMAKE_SOURCE_DIR}/src/server/game/Conditions
  ${CMAKE_SOURCE_DIR}/src/server/game/DataStores
  ${CMAKE_SOURCE_DIR}/src/server/game/DungeonFinding
  ${CMAKE_SOURCE_DIR}/src/server/game/Entities
  ${CMAKE_SOURCE_DIR}/src/server/game/Entities/Corpse
  ${CMAKE_SOURCE_DIR}/src/server/game/Entities/Creature
  ${CMAKE_SOURCE_DIR}/src/server/game/Entities/DynamicObject
  ${CMAKE_SOURCE_DIR}/src/server/game/Entities/GameObject
  ${CMAKE_SOURCE_DIR}/src/server/game/Entities/Item
  ${CMAKE_SOURCE_DIR}/src/server/game/Entities/Item/Container
  ${CMAKE_SOURCE_DIR}/src/server/game/Entities/Object
  ${CMAKE_SOURCE_DIR}/src/server/game/Entities/Pet
  ${CMAKE_SOURCE_DIR}/src/server/game/Entities/Player
  ${CMAKE_SOURCE_DIR}/src/server/game/Entities/Totem
  ${CMAKE_SOURCE_DIR}/src/server/game/Entities/Unit
  ${CMAKE_SOURCE_DIR}/src/server/game/Entities/Vehicle
  ${CMAKE_SOURCE_DIR}/src/server/game/GameEvents
  ${CMAKE_SOURCE_DIR}/src/server/game/Globals
  ${CMAKE_SOURCE_DIR}/src/server/game/Grids
  ${CMAKE_SOURCE_DIR}/src/server/game/Grids/Cells
  ${CMAKE_SOURCE_DIR}/src/server/game/Grids/Notifiers
  ${CMAKE_SOURCE_DIR}/src/server/game/Groups
  ${CMAKE_SOURCE_DIR}/src/server/game/Guilds
  ${CMAKE_SOURCE_DIR}/src/server/game/Handlers
  ${CMAKE_SOURCE_DIR}/src/server/game/Instances
  ${CMAKE_SOURCE_DIR}/src/server/game/Loot
  ${CMAKE_SOURCE_DIR}/src/server/game/Mails
  ${CMAKE_SOURCE_DIR}/src/server/game/Maps
  ${CMAKE_SOURCE_DIR}/src/server/game/Miscellaneous
  ${CMAKE_SOURCE_DIR}/src/server/game/Movement
  ${CMAKE_SOURCE_DIR}/src/server/game/Movement/Spline
  ${CMAKE_SOURCE_DIR}/src/server/game/Opcodes
  ${CMAKE_SOURCE_DIR}/src/server/game/OutdoorPvP
  ${CMAKE_SOURCE_DIR}/src/server/game/Pools
  ${CMAKE_SOURCE_DIR}/src/server/game/PrecompiledHeaders
  ${CMAKE_SOURCE_DIR}/src/server/game/Quests
  ${CMAKE_SOURCE_DIR}/src/server/game/Reputation
  ${CMAKE_SOURCE_DIR}/src/server/game/Scene
  ${CMAKE_SOURCE_DIR}/src/server/game/Scripting
  ${CMAKE_SOURCE_DIR}/src/server/game/Server
  ${CMAKE_SOURCE_DIR}/src/server/game/Server/Protocol
  ${CMAKE_SOURCE_DIR}/src/server/game/Skills
  ${CMAKE_SOURCE_DIR}/src/server/game/Spells
  ${CMAKE_SOURCE_DIR}/src/server/game/Spells/Auras
  ${CMAKE_SOURCE_DIR}/src/server/game/Texts
  ${CMAKE_SOURCE_DIR}/src/server/game/Tools
  ${CMAKE_SOURCE_DIR}/src/server/game/Warden
  ${CMAKE_SOURCE_DIR}/src/server/game/Weather
  ${CMAKE_SOURCE_DIR}/src/server/game/World
  ${CMAKE_SOURCE_DIR}/src/server/shared/Dynamic/LinkedReference
  ${CMAKE_SOURCE_DIR}/src/server/collision
  ${CMAKE_SOURCE_DIR}/src/server/collision/Maps
  ${CMAKE_SOURCE_DIR}/src/server/collision/Management
  ${CMAKE_SOURCE_DIR}/src/server/collision/Models
  ${ACE_INCLUDE_DIR}
  ${MYSQL_INCLUDE_DIR}
  ${OPENSSL_INCLUDE_DIR}
  ${CMAKE_SOURCE_DIR}/dep/recastnavigation/Detour
  ${CMAKE_SOURCE_DIR}/dep/recastnavigation/Recast
  ${CMAKE_SOURCE_DIR}/dep/g3dlite/include
  ${CMAKE_SOURCE_DIR}/src/server/scripts/LuaEngine/deps/lualib
)

if(UNIX)
  target_compile_definitions(lualib PRIVATE LUA_USE_LINUX)
endif()

# ELUNA_EXPANSION se pasa NUMERICO (4 == EXP_MOP) a proposito: ElunaIncludes.h
# usa las comparaciones `#if ELUNA_EXPANSION > EXP_CLASSIC` ANTES de incluir
# ElunaUtility.h, donde viven las constantes EXP_*. Con la forma simbolica esas
# comparaciones se evaluarian contra 0 y elegirian la rama equivocada.
add_definitions(-DELUNA_SKYFIRE -DELUNA -DELUNA_EXPANSION=4)

# Eluna Engine Sources
file(GLOB eluna_sources
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/hooks/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/hooks/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/methods/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/methods/*.h"
)
# Exclude lmarshal (it might be included or standalone, check later. Usually part of engine)
# Exclude deps folder files just in case
list(FILTER eluna_sources EXCLUDE REGEX "/deps/")

add_library(eluna STATIC ${eluna_sources})
target_link_libraries(eluna PUBLIC lualib)
target_include_directories(eluna PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_include_directories(eluna PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/methods/TrinityCore")
target_include_directories(eluna PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/methods")

# Link scripts to eluna?
# Actually 'scripts' target is defined in parent directory.
# We can't easily link 'scripts' to 'eluna' HERE if 'scripts' is defined there.
# But 'eluna' is added via add_subdirectory.
# We should probably export 'eluna' so 'worldserver' can link it.

set(CUSTOM_METHODS_HEADER "${CMAKE_CURRENT_SOURCE_DIR}/methods/Custom/CustomMethods.h")
if(EXISTS "${CUSTOM_METHODS_HEADER}")
  if(NOT ${LUA_VERSION} MATCHES "luajit")
    target_compile_definitions(lualib PUBLIC ELUNA_USE_CUSTOM_METHODS)
    target_compile_definitions(eluna PUBLIC ELUNA_USE_CUSTOM_METHODS)
  else()
    target_compile_definitions(lualib INTERFACE ELUNA_USE_CUSTOM_METHODS)
    target_compile_definitions(eluna INTERFACE ELUNA_USE_CUSTOM_METHODS)
  endif()
endif()

# Define variables for paths
set(MODULES_DIR "${CMAKE_CURRENT_SOURCE_DIR}/modules")
if(UNIX)
  set(MODULES_OUTPUT_DIR "bin/lua_scripts/modules")
elseif(WIN32)
  set(MODULES_OUTPUT_DIR "${CMAKE_BINARY_DIR}/bin/lua_scripts/modules")
  set(MODULES_OUTPUT_DIR_CONF "${CMAKE_BINARY_DIR}/bin/$<CONFIG>/lua_scripts/modules")
endif()

# Add Lua compatibility definition
if(NOT ${LUA_VERSION} MATCHES "luajit")
  target_compile_definitions(lualib PUBLIC LUA_COMPAT_MODULE) 
  target_compile_definitions(eluna PUBLIC LUA_COMPAT_MODULE)
endif()

# Collect module directories
file(GLOB_RECURSE modules_list LIST_DIRECTORIES true ${MODULES_DIR}/*)
foreach(dir ${modules_list})
  if(IS_DIRECTORY ${dir})
    get_filename_component(module_name ${dir} NAME)
    
    # Gather all module source files
    file(GLOB sources_module
      ${dir}/*.c
      ${dir}/*.cpp
      ${dir}/*.h
      ${dir}/*.hpp)
    
    # Add module target and properties
    if(sources_module)
      add_library(${module_name} MODULE ${sources_module})
      target_include_directories(${module_name} INTERFACE ${PUBLIC_INCLUDES})
      target_link_libraries(${module_name} PUBLIC lualib) # Modules link to lua
      set_target_properties(${module_name} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${MODULES_OUTPUT_DIR})
      
      if(WIN32)
        set_target_properties(${module_name} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${MODULES_OUTPUT_DIR})
        if(MSVC)
          foreach(OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES})
            string( TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG )
            set_target_properties(${module_name}
              PROPERTIES
              RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${MODULES_OUTPUT_DIR_CONF}
              LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${MODULES_OUTPUT_DIR_CONF})
          endforeach()
        endif()
      endif()
      
      message(STATUS "Added Eluna custom module: ${dir}")
    endif()
    
    # Append to lists
    list(APPEND list_module_names ${module_name})
    list(APPEND list_module_sources ${sources_module})
    list(APPEND list_module_includes ${dir})
  endif()
endforeach()

# Safeguard to remove module sources from all other build targets than the modules themselves
macro(remove_module_sources target)
  get_target_property(_sources ${target} SOURCES)
  if(_sources)
    foreach(item ${list_module_sources})
      list(REMOVE_ITEM _sources ${item})
    endforeach()
    set_property(TARGET ${target} PROPERTY SOURCES ${_sources})
  endif()
  
  get_target_property(_includes ${target} INCLUDE_DIRECTORIES)
  if(_includes)
    foreach(dir ${list_module_includes})
      list(REMOVE_ITEM _includes ${dir})
    endforeach()
    set_property(TARGET ${target} PROPERTY INCLUDE_DIRECTORIES ${_includes})
  endif()
endmacro()

foreach(target ${all_targets})
  if (NOT ${target} IN_LIST list_module_names)
    remove_module_sources(${target})
  endif()
endforeach()
