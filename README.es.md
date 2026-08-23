### [![Eluna](docs/Eluna.png)](https://github.com/ElunaLuaEngine/Eluna)

**Español** · [English](README.md)

# Eluna para SkyFire — Mists of Pandaria 5.4.8

Un port del [motor Lua Eluna](https://github.com/ElunaLuaEngine/Eluna) moderno a
[SkyFire_548](https://github.com/ProjectSkyfire/SkyFire_548), el emulador de 5.4.8.

Eluna lleva años dando soporte a MaNGOS, cMaNGOS y TrinityCore, pero nunca a Mists of
Pandaria: la única Eluna de la época de MoP que circula es el árbol de 2010-2013 que
arrastran algunos forks de `pandaria_5.4.8`, anterior a casi toda la API actual. Este
repositorio es el motor moderno —hooks actuales, conjunto de métodos actual— corriendo
sobre 5.4.8.

**Estado:** en producción en un servidor privado desde junio de 2026. 149 de los 150 hooks
que implementa el motor están cableados a call-sites reales del core, y 18 escenarios
automáticos pasan contra un mundo vivo. El hook que falta es
`PLAYER_EVENT_ON_FREE_TALENT_POINTS_CHANGED`: en MoP los puntos de talento libres solo
existen para mascotas, y la firma del hook recibe un `Player*`, así que no hay dónde
cablearlo.

---

## Por qué te puede interesar

* **Programar en 5.4.8 sin tocar C++.** Jefes, eventos, sistemas propios, integración con
  una tienda web: 900+ métodos Lua y 150 hooks.
* **Se puede instalar.** Un solo script aplica 209 bloques de cableado sobre un checkout
  limpio de SkyFire. Sin fusiones a mano ni ruido de reformateo (ver [Instalación](#instalación)).
* **Lo que afirma está probado, no prometido.** Ver [Verificación](#verificación).

---

## Lo que añade sobre Eluna original

Este port no es una copia. Parte es específico de MoP; parte vale para cualquier core.

### Eventos nuevos

| Evento | Firma | Notas |
|---|---|---|
| `UNIT_EVENT_ON_DAMAGE` (36) | `(atacante, víctima, daño)` | **Devuelve un número para sustituir el daño.** Eluna original no lo expone |
| `UNIT_EVENT_ON_HEAL` (37) | `(sanador, receptor, cantidad)` | Igual, para curación |
| `PLAYER_EVENT_ON_SPEC_CHANGED` (55) | `(jugador, nueva, vieja)` | Cubre las tres vías: cambio de talentos, reinicio y alternar dual-spec |
| `MAP_EVENT_ON_GRID_LOAD` (19) | `(mapa, gx, gy)` | Declarado en Eluna desde hace años, nunca implementado |
| `MAP_EVENT_ON_GRID_UNLOAD` (20) | `(mapa, gx, gy)` | Igual. Dispara en la descarga *real*, no solo en el camino de recarga |

`MAP_EVENT_ON_CREATE` también estaba declarado y sin cablear; aquí está cableado.

### Métodos nuevos

* **Especialización y monedas de MoP** — `Player:GetSpecialization([spec])` (el id real de
  `ChrSpecialization`, no el índice), `SetSpecialization(id)`, `GetCurrency(id)`,
  `ModifyCurrency(id, n)`, `HasCurrency(id, n)`.
* **`Player:InjectPacket(paquete)`** *(inseguro)* — encola un paquete de cliente en una
  sesión como si lo hubiera enviado el cliente, de modo que un script puede accionar
  correo, comercio, subastas, gossip o chat por los manejadores reales del servidor.
  Hecho para pruebas automáticas.
* **Escritura de paquetes bit a bit** — `WriteBit`, `WriteBits`, `FlushBits`,
  `WriteGuidMask`, `WriteGuidBytes`, `WriteBytes`. Los paquetes de MoP empaquetan campos
  en tiras de bits y reparten los GUID en una máscara más una secuencia de bytes; sin
  esto no se puede construir uno.
* **`GetOpcodeByName(nombre)`** — el enum de opcodes de SkyFire es alfabético y
  autonumerado, así que los números no se pueden fijar a mano.
* **`RegisterVehicleEvent`** — no existe en Eluna original, así que los cinco hooks de
  vehículo no se podían registrar siquiera.
* **`Map:SetWeather(zona, tipo, grado)`** — era una función vacía.

### Fallos de upstream corregidos aquí

* **`AUCTION_EVENT_ON_REMOVE` no disparaba nunca.** Ni en este core ni —por lo que se lee
  del hook y del orden de llamadas— en los demás destinos de Eluna. El hook resuelve el
  `Item` subastado con `GetAItem()`, pero los tres call-sites retiran el objeto del mapa
  *antes* de retirar la subasta, que es donde vivía el hook. Así que siempre salía por su
  propia comprobación `if (!owner || !item) return;`, en silencio, el 100% de las veces.
  Corregido disparando en cada call-site, mientras el objeto todavía se puede resolver.
* **`ElunaQuery:GetRow()` colapsaba todas las columnas en una sola clave** en este core,
  porque los metadatos del `ResultSet` devolvían `"unknown"` como alias de todas.

---

## Requisitos y límites

Lee esta sección antes de abrir una incidencia.

* **SkyFire_548, `main` actual.** Verificado contra el upstream `0e0cfa60dd` e instalado
  limpiamente sobre `936493b` (más nuevo). El propio upstream exige ya **g++ 14 / MSVC con
  C++23** y **Boost 1.91**.
* **Un solo estado Lua.** Un `lua_State` para todo el mundo, sin candados.
  `MapUpdate.Threads` **debe ser 1**; el motor lo fuerza a 1 y avisa por el log si Eluna
  está activo con más, porque la alternativa son cuelgues aleatorios sin causa aparente.
* **Lua 5.2 / LuaJIT**, como upstream.
* **Las referencias a objetos solo valen dentro del callback que las produjo.** Guarda
  GUIDs y vuelve a resolverlos al principio de cada callback (`GetPlayerByGUID`,
  `map:GetWorldObject`). Es comportamiento de upstream, no algo que introduzca este port,
  pero es con diferencia la causa más común de `calling 'X' on bad self`, y le pasa a todo
  el mundo una vez.

---

## Instalación

El cableado son 209 bloques pequeños repartidos por 46 ficheros del core. Un `.patch`
normal no sirve: por este core se pasó `clang-format` sobre ficheros enteros, así que un
diff entierra los hooks bajo decenas de miles de líneas reformateadas. En su lugar, cada
bloque se describe por su **ancla**: una secuencia corta de líneas vecinas que existe tal
cual en upstream y que allí es única. El instalador busca cada ancla en *tu* checkout e
inserta el bloque al lado, tenga el formato que tenga.

```bash
git clone https://github.com/ProjectSkyfire/SkyFire_548.git
git clone https://github.com/yangpa97/Eluna-MoP-Custom.git \
          SkyFire_548/src/server/scripts/LuaEngine

cd SkyFire_548
python3 src/server/scripts/LuaEngine/patches/install_core_hooks.py --core "$PWD" --lang es
# → manifiesto generado contra upstream 0e0cfa60dd; 195 bloques con ancla, 14 manuales
#   bloques: 195 aplicados, 0 sin ancla   manuales: 14
```

Los 14 bloques que el instalador llama "manuales" son los que no tienen un ancla usable:
una función nueva entera, una lista de inicialización, un fichero nuevo. El script también
los escribe; no queda nada que pegar a mano.

Después, compila como siempre. El instalador añade además
`-DELUNA_SKYFIRE -DELUNA_EXPANSION=4` a `src/server/game/CMakeLists.txt` y registra el
motor en `src/server/scripts`.

Es **idempotente** —un bloque cuyas líneas ya están se salta—, así que puedes repetirlo
tras actualizar upstream. `--dry-run` informa de lo que haría sin escribir nada, y el
idioma sale del entorno (`LANG`) si no pasas `--lang`.

---

## Configuración

Añade a `worldserver.conf`:

```ini
Eluna.Enabled              = 1
Eluna.ScriptPath           = "lua_scripts"
Eluna.TraceBack            = 1     # traza completa de Lua al fallar
Eluna.ReloadCommand        = 1     # habilita ".reload eluna"
Eluna.ReloadSecurityLevel  = 3     # nivel de GM que hace falta para ello
Eluna.UseUnsafeMethods     = 1     # necesario para InjectPacket y consultas SQL crudas
Eluna.OnlyOnMaps           = ""    # vacío = todos los mapas
Eluna.BotsFireHooks        = 0     # ver abajo (la clave antigua
                                   # Eluna.BotsDisparanHooks sigue funcionando)
```

`Eluna.BotsFireHooks = 0` impide que los hooks cuyo sujeto es el jugador disparen para
sesiones de playerbot. La regla es deliberada: un bot **no** dispara los hooks cuyo sujeto
es el jugador (chat, guardado, objetos, gossip, subir a un vehículo), pero **sí** dispara
aquellos cuyo sujeto es otra entidad — un jefe escrito en Lua tiene que ver que un bot le
está pegando. Ponlo a 1 si no usas bots.

---

## Qué pinta tiene un script

```lua
-- Reduce a la mitad el daño que reciben los jugadores por debajo de nivel 10.
-- ON_DAMAGE es global, así que vive en la familia de eventos de servidor:
-- RegisterServerEvent, no RegisterUnitEvent.
RegisterServerEvent(36, function(event, atacante, victima, dano)
    local jugador = victima:ToPlayer()
    if jugador and jugador:GetLevel() < 10 then
        return math.floor(dano / 2)        -- devolver un número sustituye el daño
    end
    if dano > 100000 then
        print(("%s golpeó a %s por %d"):format(atacante:GetName(), victima:GetName(), dano))
    end
end)

-- Dar Puntos de Valor cuando un jugador cambia de especialización.
RegisterPlayerEvent(55, function(event, jugador, nueva, vieja)
    jugador:ModifyCurrency(396, 100)       -- 396 = Puntos de Valor
    jugador:SendBroadcastMessage(("Spec %d → %d"):format(vieja, nueva))
end)
```

La [documentación de la API de upstream](https://elunaluaengine.github.io/) vale para todo
lo heredado de Eluna; las tablas de arriba cubren lo que es nuevo aquí. Si vas a portar
Eluna a otro core de 5.x, [docs/PORTING_NOTES.md](docs/PORTING_NOTES.md) es el mapa de qué
hubo que cambiar y por qué.

---

## Verificación

Cada afirmación de este README corresponde a una comprobación que se ejecutó.

| Comprobación | Resultado |
|---|---|
| Compilación, Windows x64 (MSVC 2026, C++23) | 0 errores, tanto en este fork como en un checkout limpio de upstream |
| Compilación, Linux x86-64 (GCC 13, Ubuntu 24.04) | 0 errores — contra un árbol de SkyFire anterior al salto a C++23 de upstream. El `main` actual de upstream necesita g++ 14 y Boost 1.91, y ninguno está empaquetado aún para Ubuntu 24.04; eso es requisito de upstream, no de este motor |
| Instalador sobre un checkout limpio de upstream | 195/195 bloques con ancla + 14 manuales, 0 anclas fallidas — incluso sobre un commit de upstream *más nuevo* que el del manifiesto |
| Comprobación línea a línea de lo instalado contra el árbol de origen | cada línea de **código** del motor cae en el mismo contexto; las únicas diferencias están en los comentarios |
| Arranque del servidor desde ese árbol limpio | `World initialized`, 34 scripts cargados, apagado limpio; también con `Eluna.Enabled=0` y con `MapUpdate.Threads=4` |
| Escenarios in-game | **18/18** |

La batería in-game es un banco de pruebas en Lua que dirige a dos playerbots y **comprueba
el estado del juego, no solo que el hook disparara**: el hook de daño fuerza cada golpe a
1 y la vida del lobo debe bajar exactamente tantos puntos como golpes; la prueba de
subasta publica y cancela una subasta real y el objeto debe salir de la mochila; la de
duelo hace 5.000.000 de daño y el objetivo debe acabar a 1 de vida y vivo. Cubre daño,
curación, grupos, instancias y carga de terreno, métodos de MoP, filas de base de datos,
hermandades, clima, eventos de temporada, control del servidor, la familia de hooks de
jugador, chat/emote/addon, correo, gossip sin NPC, comercio, duelos, subastas y muerte.

Cuatro de los arreglos que aparecen en este README los encontró ese banco de pruebas, no
la lectura del código.

---

## Créditos y licencia

Esto es un fork del [motor Lua Eluna](https://github.com/ElunaLuaEngine/Eluna), del equipo
de Eluna y sus colaboradores. El diseño y la inmensa mayoría del código son suyos; este
repositorio lo porta a SkyFire 5.4.8 y lo extiende. **No está afiliado ni respaldado** por
el proyecto Eluna: por favor, no lleves a sus canales de soporte los problemas que
encuentres aquí.

Licencia **GPLv3**, como upstream. Ver [LICENSE](LICENSE).

Se agradecen informes de fallos y pull requests. Si lo pones en un reino en producción,
cuéntame qué se rompe: un servidor no es una muestra. Las incidencias son mucho más útiles
con el commit de tu core y el Lua que reproduce el problema.
