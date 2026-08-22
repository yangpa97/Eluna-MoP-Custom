#!/usr/bin/env python3
"""Instala los hooks de Eluna en un checkout limpio de ProjectSkyfire/SkyFire_548.

POR QUE UN INSTALADOR Y NO UN .patch
------------------------------------
Nuestro fork paso clang-format por encima de Player.cpp y de otros ficheros
enteros. Un diff contra upstream mete el cableado de Eluna en los mismos hunks
que 50.000 lineas de reformateo, y git apply no puede aplicar eso a nadie.

Pero el cableado tiene una propiedad que el reformateo no destruye: casi todos
los bloques van CO-UBICADOS con una llamada sScriptMgr->OnX(...) que existe
verbatim en upstream. Asi que cada bloque se describe por su ANCLA -- una
secuencia de 1 a 3 lineas significativas vecinas que upstream tiene tal cual y
que es unica alli -- y este script lo inserta al lado, en el core que sea, con
el formato que tenga. Las lineas se comparan normalizando espacios y sin el
comentario de cola; la sangria se reproduce RELATIVA al ancla.

core_hooks.json lo genera gen_manifest.py desde nuestro arbol; no se edita a
mano salvo la seccion "manual", que cubre lo que no tiene ancla (una funcion
nueva entera, una lista de inicializacion, el fichero nuevo).

Es idempotente: un bloque cuyas lineas de codigo ya estan todas en el fichero
se da por instalado. Se ha verificado contra un checkout limpio de upstream:
las 192 lineas Eluna del core quedan en el mismo contexto (linea anterior y
posterior) que en nuestro arbol, y worldserver compila.

USO
---
    python install_core_hooks.py --core /ruta/a/SkyFire_548            # aplica
    python install_core_hooks.py --core /ruta/a/SkyFire_548 --dry-run  # solo cuenta

Despues: copiar este directorio (scripts/LuaEngine) al core y compilar. El
propio manifiesto anade -DELUNA_SKYFIRE -DELUNA_EXPANSION=4 a game/CMakeLists.
"""
import argparse
import json
import os
import re
import sys

AQUI = os.path.dirname(os.path.abspath(__file__))
MANIFEST = os.path.join(AQUI, "core_hooks.json")
TRIVIAL = re.compile(r"^\s*([{}]|//.*|/\*.*|\*.*|#(?!include).*|)\s*$")
CRLF = "\r\n"
LF = "\n"


ELUNA = re.compile(r"Eluna|eluna|LuaEngine|ELUNA|LuaVal|lua_data")


def sig(l):
    """Forma normalizada de una linea: sin comentario de cola, espacios colapsados.
    DEBE ser identica a la de gen_manifest.py."""
    l = re.sub(r"(?<=[;){}\s])//.*$", "", l.strip())
    return re.sub(r"\s+", " ", l.strip())


def indent_of(l):
    return l[: len(l) - len(l.lstrip())]


def reindent(block, ind):
    """Re-sangra el bloque para que su primera linea quede al nivel del ancla."""
    if not block:
        return block
    base = indent_of(block[0])
    out = []
    for l in block:
        out.append(ind + (l[len(base):] if l.startswith(base) else l.lstrip()))
    return out


def buscar_secuencia(sigs, seq, nth=None):
    """Indices de la ULTIMA linea de cada aparicion de `seq` entre las lineas
    significativas (las triviales -- llaves, blancos, comentarios -- se saltan)."""
    sign_idx = [i for i, s in enumerate(sigs) if not TRIVIAL.match(s)]
    n = len(seq)
    hits = [sign_idx[t + n - 1] for t in range(len(sign_idx) - n + 1)
            if [sigs[sign_idx[t + q]] for q in range(n)] == seq]
    if nth is not None and len(hits) > 1:
        # el manifiesto guarda nth SOLO cuando la secuencia no es unica: es la
        # n-esima aparicion (desde 0) de la SECUENCIA COMPLETA
        return [hits[nth]] if nth < len(hits) else []
    return hits


def ya_instalado(block, sigs):
    """Un bloque cuenta como instalado si TODAS sus lineas de codigo estan ya en
    el fichero (se mira el fichero tal como estaba antes de esta pasada). Mirar
    solo la mas larga fallaba en los dos sentidos: un `if (pItem)` envolvente
    existe en upstream, y el generico `if (Eluna* e = sWorld->GetEluna())` lo
    habra puesto ya cualquier otro bloque."""
    codigo = [sig(l) for l in block if not TRIVIAL.match(sig(l))]
    return bool(codigo) and all(l in sigs for l in codigo)


def leer(ruta):
    with open(ruta, encoding="utf-8", errors="replace", newline="") as f:
        raw = f.read()
    nl = CRLF if CRLF in raw else LF
    return raw, nl


def aplicar(core, entradas, dry):
    por_fichero = {}
    for e in entradas:
        por_fichero.setdefault(e["file"], []).append(e)

    ok = fallos = 0
    for rel, lista in por_fichero.items():
        ruta = os.path.join(core, rel)
        if not os.path.exists(ruta):
            print(f"  NO EXISTE {rel}")
            fallos += len(lista)
            continue
        raw, nl = leer(ruta)
        lines = raw.split(nl)
        sigs = [sig(l) for l in lines]

        inserciones = []
        for e in lista:
            a = e["anchor"]
            if ya_instalado(e["block"], set(sigs)):
                ok += 1
                continue
            idx = buscar_secuencia(sigs, a["seq"], a.get("nth"))
            if len(idx) != 1:
                print(f"  ANCLA {'ambigua' if idx else 'ausente'} ({len(idx)}) en {rel}: {' | '.join(a['seq'])[:90]}")
                fallos += 1
                continue
            i = idx[0]
            if a["position"] == "before":
                sign_idx = [q for q, s in enumerate(sigs) if not TRIVIAL.match(s)]
                i = sign_idx[sign_idx.index(i) - (len(a["seq"]) - 1)]
            # llaves que separaban el ancla del bloque en el arbol de origen: se
            # saltan aqui tambien, para caer en el mismo ambito (p. ej. un
            # "friend class Eluna;" que va DENTRO de "class WorldObject {")
            pos = i + 1 if a["position"] == "after" else i
            for br in a.get("braces", []):
                if a["position"] == "after":
                    while pos < len(sigs) and sigs[pos] == "": pos += 1
                    if pos < len(sigs) and sigs[pos] == br: pos += 1
                else:
                    q = pos - 1
                    while q >= 0 and sigs[q] == "": q -= 1
                    if q >= 0 and sigs[q] == br: pos = q
            # sangria: la del ancla mas el desplazamiento que tenia en el arbol de
            # origen (los dos arboles usan 4 espacios); nunca negativa
            ind = max(0, len(indent_of(lines[i]).expandtabs(4)) + a.get("indent_rel", 0))
            bloque = reindent(e["block"], " " * ind)
            inserciones.append((pos, len(inserciones), bloque))
            ok += 1

        if dry or not inserciones:
            continue
        # de abajo arriba para no mover los indices; a igual posicion, el ultimo
        # del manifiesto se inserta primero para que queden en el orden original
        for i, _, bloque in sorted(inserciones, key=lambda x: (-x[0], -x[1])):
            lines[i:i] = bloque
        with open(ruta, "w", encoding="utf-8", newline="") as f:
            f.write(nl.join(lines))
    return ok, fallos


def manual(core, m, dry):
    """Cambios que no encajan en el modelo de ancla+bloque."""
    hechos = 0
    for item in m.get("manual", []):
        ruta = os.path.join(core, item["file"])
        if item.get("new_file"):
            if not dry:
                os.makedirs(os.path.dirname(ruta), exist_ok=True)
                with open(ruta, "w", encoding="utf-8", newline=LF) as f:
                    f.write(LF.join(item["content"]) + LF)
            hechos += 1
            continue
        raw, nl = leer(ruta)
        if item.get("replace"):
            viejo, nuevo = item["replace"], nl.join(item["with"])
            if nuevo in raw:
                hechos += 1
                continue
            if raw.count(viejo) != 1:
                print(f"  MANUAL fallo en {item['file']}: '{viejo[:60]}' aparece {raw.count(viejo)} veces")
                continue
            raw = raw.replace(viejo, nuevo)
        else:
            lines = raw.split(nl)
            sigs = [sig(l) for l in lines]
            if ya_instalado(item["block"], set(sigs)):
                hechos += 1
                continue
            idx = [i for i, s in enumerate(sigs) if s == item["anchor"]]
            if len(idx) != 1:
                print(f"  MANUAL ancla {'ambigua' if idx else 'ausente'} en {item['file']}: {item['anchor'][:60]}")
                continue
            i = idx[0]
            bloque = reindent(item["block"], indent_of(lines[i]))
            pos = i + 1 if item["position"] == "after" else i
            lines[pos:pos] = bloque
            raw = nl.join(lines)
        if not dry:
            with open(ruta, "w", encoding="utf-8", newline="") as f:
                f.write(raw)
        hechos += 1
    return hechos


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--core", required=True, help="raiz del checkout de SkyFire_548")
    ap.add_argument("--dry-run", action="store_true")
    a = ap.parse_args()

    if not os.path.isdir(os.path.join(a.core, "src", "server", "game")):
        sys.exit(f"{a.core} no parece un checkout de SkyFire (falta src/server/game)")

    m = json.load(open(MANIFEST, encoding="utf-8"))
    print(f"manifiesto generado contra upstream {m['base_upstream'][:10]}; "
          f"{len(m['hooks'])} bloques con ancla, {len(m.get('manual', []))} manuales")

    ok, fallos = aplicar(a.core, m["hooks"], a.dry_run)
    hechos = manual(a.core, m, a.dry_run)
    print(f"\nbloques: {ok} aplicados, {fallos} sin ancla   manuales: {hechos}")
    if a.dry_run:
        print("(simulacro: no se ha escrito nada)")
    if fallos:
        sys.exit(1)


if __name__ == "__main__":
    main()
