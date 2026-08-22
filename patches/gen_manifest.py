"""Genera patches/core_hooks.json desde nuestro arbol.

Metodo: diferencia de LINEAS (normalizando espacios) entre upstream y el nuestro.
Una linea "anadida" es la que no existe en upstream. Un bloque Eluna es una
racha de lineas anadidas que empieza en una que menciona Eluna y se extiende
por las continuaciones sintacticas (cuerpo del if, llaves, comentarios).

El ANCLA de cada bloque es la secuencia de 1..3 lineas significativas
inmediatamente anteriores que existen en upstream, la mas corta que sea UNICA
alli. Si ni con 3 es unica, se guarda ademas que ocurrencia es (n-esima).
Asi un `return pItem;` repetido 40 veces no sirve solo, pero `SetState(...)` +
`return pItem;` si.
"""
import subprocess, re, os, json, sys
# Se ejecuta desde la raiz del core que contiene NUESTRO arbol (el que tiene los
# hooks). Necesita un remoto "upstream" apuntando a ProjectSkyfire/SkyFire_548.
os.chdir(sys.argv[1] if len(sys.argv) > 1 else os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "..", "..", "..", ".."))
base = subprocess.check_output(['git', 'merge-base', 'HEAD', 'upstream/main'], text=True).strip()
files = subprocess.check_output(['git', 'grep', '-lE', 'Eluna|LuaEngine|ELUNA', '--',
                                 'src/server/game/', 'src/server/scripts/CMakeLists.txt'], text=True).split()
files = [f for f in files if 'game/LuaEngine/' not in f]

TRIVIAL = re.compile(r'^\s*([{}]|//.*|/\*.*|\*.*|#(?!include).*|)\s*$')   # '#' = comentario de CMake
ELUNA = re.compile(r'Eluna|eluna|LuaEngine|ELUNA')
CONT = ('e->', 'if (e->', 'if (!e->', 'return', 'delete packet', 'packet = NULL', 'continue;', 'else',
        'uint32 sender', 'uint32 action', 'if (IS_ITEM_GUID', 'if (Item* item', '_player->PlayerTalkClass->SendCloseGossip',
        'if (res != EQUIP_ERR_OK', 'if (InventoryResult res', 'std::string paquete', 'if (questGiver &&',
        'questGiver->GetTypeId()', 'if (Creature* creTarget', 'else if (gameObjTarget', 'else if (itemTarget',
        'if (Creature* accesorio', 'if (Passenger->HasUnitTypeMask', 'uint32 const specAntes', 'SetActiveSpec(spec);',
        'if (GetTalentSpecialization()', 'Item* item = (questGiver', 'if (old != tree', 'uint32 const old',
        '_talentMgr->SpecInfo', 'if (GridMaps[gx][gy])', 'if (!i_data)', 'i_data = e->', 'if (!murio')
def sig(l): return re.sub(r'\s+', ' ', l.strip())

SALTAR_NO_ELUNA = {  # lineas anadidas nuestras que NO son de Eluna y van pegadas a un bloque
    'newChar.LearnSpecialization(specializationId);',
}

manifest, sin_ancla, stats = [], [], {'bloques': 0, 'ancla1': 0, 'ancla2': 0, 'ancla3': 0, 'nth': 0}
for f in files:
    try:
        up = subprocess.check_output(['git', 'show', f'{base}:{f}'], text=True, encoding='utf-8', errors='replace').splitlines()
    except subprocess.CalledProcessError:
        continue
    upsig_list = [sig(l) for l in up]
    upsig_sign = [s for s in upsig_list if not TRIVIAL.match(s)]   # solo significativas, en orden
    upsig = set(upsig_sign)
    upsig_all = set(upsig_list)
    ours = open(f, encoding='utf-8', errors='replace').read().splitlines()
    osig = [sig(l) for l in ours]
    added = [(not TRIVIAL.match(osig[i]) and osig[i] not in upsig and osig[i] not in SALTAR_NO_ELUNA) for i in range(len(ours))]

    i = 0
    while i < len(ours):
        if not (added[i] and ELUNA.search(ours[i])) and not (added[i] and ours[i].strip().startswith('// ====') and ELUNA.search(ours[i])):
            i += 1; continue
        # arranque: incluir comentarios anadidos justo encima (el '// Eluna - ...' o '// ===== CUSTOM')
        start = i
        while start - 1 >= 0 and (osig[start-1].startswith(('//', '#')) and osig[start-1] not in upsig_all and osig[start-1] != ''):
            start -= 1
        # extension
        k = i + 1
        while k < len(ours):
            s = osig[k]
            if s == '' or s in ('{', '}'):
                # llaves/blancos: seguir solo si lo siguiente significativo sigue siendo nuestro
                n = k + 1
                while n < len(ours) and (osig[n] == '' or osig[n] in ('{', '}')): n += 1
                if n < len(ours) and added[n] and (ELUNA.search(ours[n]) or ours[n].strip().startswith(CONT) or s == '{'):
                    k += 1; continue
                break
            if added[k] and (ELUNA.search(ours[k]) or ours[k].strip().startswith(CONT) or s.startswith('//')):
                k += 1; continue
            break
        bloque = ours[start:k]
        while bloque and not bloque[-1].strip(): bloque.pop()
        stats['bloques'] += 1

        # ancla: 1..3 lineas significativas anteriores presentes en upstream, unicas como secuencia
        prev = []
        j = start - 1
        while j >= 0 and len(prev) < 3:
            if not TRIVIAL.match(osig[j]) and osig[j] in upsig and not ELUNA.search(ours[j]):
                prev.insert(0, osig[j])
            elif not TRIVIAL.match(osig[j]) and (osig[j] not in upsig):
                pass   # linea nuestra no-Eluna entre medias: la saltamos
            j -= 1
        ancla = None
        for n in (1, 2, 3):
            if len(prev) < n: break
            seq = prev[-n:]
            hits = [t for t in range(len(upsig_sign) - n + 1) if upsig_sign[t:t+n] == seq]
            if len(hits) == 1:
                ancla = {'seq': seq}; stats[f'ancla{n}'] += 1; break
            if n == 3 and hits:
                # no unica ni con 3: ordinal de la SECUENCIA COMPLETA (no de su ultima
                # linea, que puede tener apariciones extra en nuestro fichero). Se cuenta
                # sobre las lineas significativas nuestras que existen en upstream, que
                # es la misma vista que tendra el instalador.
                our_sign_idx = [t for t in range(len(ours)) if not TRIVIAL.match(osig[t]) and osig[t] in upsig]
                our_sign = [osig[t] for t in our_sign_idx]
                occ = [q for q in range(len(our_sign) - n + 1) if our_sign[q:q+n] == seq]
                # la ocurrencia cuya ultima linea queda justo antes de nuestro bloque
                antes = [q for q in occ if our_sign_idx[q + n - 1] < start]
                nth = len(antes) - 1
                if nth >= 0 and len(occ) == len(hits):
                    ancla = {'seq': seq, 'nth': nth}
                    stats['nth'] += 1
        if ancla: ancla['position'] = 'after'
        else:
            # alternativa: ancla POSTERIOR (1..3 significativas tras el bloque, presentes en upstream)
            nxt = []; j = k
            while j < len(ours) and len(nxt) < 3:
                if not TRIVIAL.match(osig[j]) and osig[j] in upsig and not ELUNA.search(ours[j]):
                    nxt.append(osig[j])
                j += 1
            for n in (1, 2, 3):
                if len(nxt) < n: break
                seq = nxt[:n]
                hits = [t for t in range(len(upsig_sign) - n + 1) if upsig_sign[t:t+n] == seq]
                if len(hits) == 1:
                    ancla = {'seq': seq, 'position': 'before'}; stats[f'ancla{n}'] += 1; break
        entry = {'file': f, 'line': start + 1, 'anchor': ancla, 'block': bloque}
        if any('OnSpecChanged(this, tree, old)' in l for l in bloque):
            i = k; continue   # va dentro de la funcion entera que lleva la seccion manual
        (manifest if ancla else sin_ancla).append(entry)
        i = k

json.dump({'base_upstream': base, 'hooks': manifest, 'manual': json.load(open('src/server/scripts/LuaEngine/patches/core_hooks.json', encoding='utf-8')).get('manual', [])  # la seccion manual se conserva},
          open('src/server/scripts/LuaEngine/patches/core_hooks.json', 'w', encoding='utf-8'), ensure_ascii=False, indent=1)
print(stats, '  sin ancla:', len(sin_ancla))
for e in sin_ancla:
    print(f"  SIN ANCLA {e['file'].replace('src/server/game/','')}:{e['line']}  {e['block'][0].strip()[:80]}")
