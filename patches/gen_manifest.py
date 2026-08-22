"""Genera patches/core_hooks.json desde nuestro arbol.

Metodo: diferencia de LINEAS (normalizando espacios) entre upstream y el nuestro.
Una linea "anadida" es la que no existe en upstream. Un bloque Eluna es una
racha de lineas nuestras alrededor de una que menciona Eluna:

  hacia ABAJO  - todo lo que esta mas sangrado que la primera linea de codigo
                 del bloque (el cuerpo del if), las llaves que el propio bloque
                 abre, los comentarios, y las continuaciones conocidas (CONT)
                 si ademas son nuestras.
  hacia ARRIBA - los comentarios nuestros pegados, y las cabeceras de control
                 (if/else/for) cuyo UNICO cuerpo es el bloque: si un `if (pItem)`
                 solo envuelve nuestro hook, ese if es nuestro aunque la misma
                 linea exista en otro sitio de upstream.
  auxiliares   - una linea nuestra mas arriba (hasta 8 significativas) que
                 declara un identificador que el bloque usa (`uint32 const
                 specAntes = ...`) se emite como bloque propio, antes.

El ANCLA de cada bloque es la secuencia de 1..3 lineas significativas
inmediatamente anteriores que existen en upstream, la mas corta que sea UNICA
alli. Si ni con 3 es unica, se guarda ademas que ocurrencia es (n-esima).
Asi un `return pItem;` repetido 40 veces no sirve solo, pero `SetState(...)` +
`return pItem;` si.

Cada bloque guarda su sangria RELATIVA al ancla (indent_rel): el instalador la
reproduce en el fichero destino, que usa la misma convencion de 4 espacios.
"""
import subprocess, re, os, json, sys
# Se ejecuta desde la raiz del core que contiene NUESTRO arbol (el que tiene los
# hooks). Necesita un remoto "upstream" apuntando a ProjectSkyfire/SkyFire_548.
os.chdir(sys.argv[1] if len(sys.argv) > 1 else os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "..", "..", "..", ".."))
base = subprocess.check_output(['git', 'merge-base', 'HEAD', 'upstream/main'], text=True).strip()
files = subprocess.check_output(['git', 'grep', '-lE', 'Eluna|LuaEngine|ELUNA|LuaVal', '--',
                                 'src/server/game/', 'src/server/scripts/CMakeLists.txt'], text=True).split()
files = [f for f in files if 'game/LuaEngine/' not in f]

TRIVIAL = re.compile(r'^\s*([{}]|//.*|/\*.*|\*.*|#(?!include).*|)\s*$')   # '#' = comentario de CMake
ELUNA = re.compile(r'Eluna|eluna|LuaEngine|ELUNA|LuaVal')   # LuaVal = tipo del motor (lua_data)
CONT = ('e->', 'if (e->', 'if (!e->', 'return', 'delete packet', 'packet = NULL', 'continue;', 'else',
        'delete lua_data')
BLANCO_O_COMENTARIO = re.compile(r'^\s*(//.*|/\*.*|\*.*|#(?!include).*|)\s*$')   # como TRIVIAL pero SIN las llaves
HEADER = re.compile(r'^\s*(else\s+if|if|for|while|else)\b')
DECL = re.compile(r'^\s*(?:[\w:<>]+(?:\s*[*&])?\s+)+(?:const\s+)?[*&]?\s*(\w+)\s*(?:=|;)')
def sig(l):
    """Forma normalizada de una linea: sin comentario de cola, espacios colapsados.
    DEBE ser identica a la de install_core_hooks.py."""
    l = re.sub(r'(?<=[;){}\s])//.*$', '', l.strip())
    return re.sub(r'\s+', ' ', l.strip())
def indent(l): return len(l) - len(l.lstrip())

# bloques que NO van por ancla: los cubre la seccion "manual" del manifiesto
EXCLUIR = ('OnSpecChanged(this, tree, old)',          # funcion entera SetTalentSpecialization
           'HandleGossipSelectOption(_player, item')  # rama else-if nueva de MiscHandler

# lineas Eluna que NO abren bloque: la lista de inicializacion de un ctor no es
# un statement y el manifiesto la cubre con un "replace" manual
SALTAR_INICIO = ('m_elunaProcessorId(0), lua_data(NULL)',)

manifest, sin_ancla, stats = [], [], {'bloques': 0, 'ancla1': 0, 'ancla2': 0, 'ancla3': 0, 'nth': 0, 'aux': 0, 'wrappers': 0}

def buscar_ancla(ours, osig, upsig, upsig_sign, start, k):
    """Ancla para el bloque ours[start:k]. Devuelve dict o None."""
    prev = []
    j = start - 1
    while j >= 0 and len(prev) < 3:
        if not TRIVIAL.match(osig[j]) and osig[j] in upsig and not ELUNA.search(ours[j]):
            prev.insert(0, osig[j])
        j -= 1
    if prev and HEADER.match(prev[-1]) and not prev[-1].endswith(('{', ';', '}')):
        prev = []   # un `if (x)` sin llave justo antes: el bloque no puede ir "after"
    ancla = None
    for n in (1, 2, 3):
        if len(prev) < n: break
        seq = prev[-n:]
        hits = [t for t in range(len(upsig_sign) - n + 1) if upsig_sign[t:t+n] == seq]
        if len(hits) == 1:
            ancla = {'seq': seq}; stats[f'ancla{n}'] += 1; break
        if n == 3 and hits:
            # no unica ni con 3: ordinal de la SECUENCIA COMPLETA sobre las lineas
            # significativas nuestras que existen en upstream (la misma vista que
            # tendra el instalador)
            our_sign_idx = [t for t in range(len(ours)) if not TRIVIAL.match(osig[t]) and osig[t] in upsig]
            our_sign = [osig[t] for t in our_sign_idx]
            occ = [q for q in range(len(our_sign) - n + 1) if our_sign[q:q+n] == seq]
            antes = [q for q in occ if our_sign_idx[q + n - 1] < start]
            nth = len(antes) - 1
            if nth >= 0 and len(occ) == len(hits):
                ancla = {'seq': seq, 'nth': nth}
                stats['nth'] += 1
    if ancla:
        ancla['position'] = 'after'
        # llaves entre la ultima linea del ancla y el bloque: el instalador las
        # salta para insertar DENTRO del mismo ambito (friend dentro de class {)
        llaves = []
        t = start - 1
        while t >= 0 and osig[t] != ancla['seq'][-1]:
            if osig[t] in ('{', '}'): llaves.insert(0, osig[t])
            t -= 1
        if llaves: ancla['braces'] = llaves
        ancla['indent_rel'] = indent(ours[start]) - indent(ours[t]) if t >= 0 else 0
        return ancla
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
            ancla = {'seq': seq, 'position': 'before'}
            llaves = []
            t = k
            while t < len(ours) and osig[t] != seq[0]:
                if osig[t] in ('{', '}'): llaves.append(osig[t])
                t += 1
            if llaves: ancla['braces'] = llaves
            ancla['indent_rel'] = indent(ours[start]) - indent(ours[t]) if t < len(ours) else 0
            stats[f'ancla{n}'] += 1
            return ancla
    return None

def primera_codigo(ours, osig, start, k):
    for t in range(start, k):
        if not TRIVIAL.match(osig[t]): return t
    return start

for f in files:
    try:
        up = subprocess.check_output(['git', 'show', f'{base}:{f}'], text=True, encoding='utf-8', errors='replace', stderr=subprocess.DEVNULL).splitlines()
    except subprocess.CalledProcessError:
        continue
    upsig_list = [sig(l) for l in up]
    upsig_sign = [s for s in upsig_list if not TRIVIAL.match(s)]   # solo significativas, en orden
    upsig = set(upsig_sign)
    upsig_all = set(upsig_list)
    ours = open(f, encoding='utf-8', errors='replace').read().splitlines()
    osig = [sig(l) for l in ours]
    added = [(not TRIVIAL.match(osig[i]) and osig[i] not in upsig) for i in range(len(ours))]
    en_bloque = [False] * len(ours)

    def sig_siguiente(k):
        """Indice de la siguiente linea desde k que no sea blanco ni comentario.
        Las LLAVES cuentan: un `{` suelto abre un ambito de upstream y el codigo
        de dentro va mas sangrado sin ser nuestro."""
        while k < len(ours) and BLANCO_O_COMENTARIO.match(osig[k]): k += 1
        return k if k < len(ours) else None

    i = 0
    while i < len(ours):
        if en_bloque[i]:
            i += 1; continue
        desde_comentario = False
        # (un marcador de cierre pegado a un bloque ya emitido no abre otro)
        if osig[i].startswith('//') and ELUNA.search(ours[i]) and osig[i] not in upsig_all and not (i > 0 and en_bloque[i-1]):
            n = sig_siguiente(i)
            if n is not None and not en_bloque[n] and not (added[n] and ELUNA.search(ours[n])) and not osig[n].startswith('//'):
                desde_comentario = True; i = n   # tentativo: se valida al final
        if not desde_comentario and (not (added[i] and ELUNA.search(ours[i])) or any(x in ours[i] for x in SALTAR_INICIO)):
            i += 1; continue
        start = i
        i_codigo = i   # por si el arranque por comentario se rechaza
        base_ind = indent(ours[i])

        # ---- hacia abajo ----
        k = i + 1
        depth = 0
        cerro = False      # la linea anterior fue un `}` que cerraba una llave NUESTRA
        while k < len(ours):
            s = osig[k]
            if s.startswith('else') and cerro:
                k += 1; cerro = False; continue   # el else de nuestro propio if
            if s != '' and not s.startswith('//'):
                cerro = False
            if s == '':
                n = sig_siguiente(k)
                if n is None: break
                # un hueco: seguir solo si lo que viene sigue siendo del bloque
                if osig[n] in ('{', '}'):
                    if depth > 0: k += 1; continue
                    break
                if (indent(ours[n]) > base_ind) or (added[n] and (ELUNA.search(ours[n]) or ours[n].strip().startswith(CONT))):
                    k += 1; continue
                break
            if s == '{':
                depth += 1; k += 1; continue
            if s == '}':
                if depth > 0: depth -= 1; k += 1; cerro = True; continue
                break
            if s.startswith('//'):
                n = sig_siguiente(k)
                if n is not None and osig[n] in ('{', '}'):
                    if depth > 0: k += 1; continue
                    break
                if n is not None and (indent(ours[n]) > base_ind or (added[n] and (ELUNA.search(ours[n]) or ours[n].strip().startswith(CONT))) or depth > 0):
                    k += 1; continue
                break
            if depth > 0 or indent(ours[k]) > base_ind:
                k += 1; continue
            if added[k] and (ELUNA.search(ours[k]) or ours[k].strip().startswith(CONT)):
                k += 1; continue
            break

        # ---- hacia arriba: comentarios nuestros y cabeceras que solo envuelven el bloque ----
        while True:
            cambio = False
            # comentarios nuestros pegados
            while start - 1 >= 0 and osig[start-1].startswith(('//', '#')) and osig[start-1] not in upsig_all:
                start -= 1; cambio = True
            # cabecera de control (puede ocupar varias lineas) cuyo unico cuerpo es el bloque
            j = start - 1
            while j >= 0 and osig[j] == '': j -= 1
            if j >= 0 and not osig[j].startswith('//') and not osig[j].endswith((';', '{', '}')):
                top = j
                while top - 1 >= 0 and not HEADER.match(ours[top]) and not osig[top-1].endswith((';', '{', '}')) and osig[top-1] != '':
                    top -= 1
                if HEADER.match(ours[top]) and not en_bloque[top]:
                    h_ind = indent(ours[top])
                    cuerpo = primera_codigo(ours, osig, start, k)
                    n = sig_siguiente(k)
                    sole = indent(ours[cuerpo]) > h_ind and (n is None or indent(ours[n]) <= h_ind)
                    if sole:
                        start = top; base_ind = h_ind; stats['wrappers'] += 1; cambio = True
            if not cambio: break

        bloque = ours[start:k]
        while bloque and not bloque[-1].strip(): bloque.pop()
        k = start + len(bloque)
        if desde_comentario:
            # valido solo si (a) contiene codigo Eluna nuestro y (b) sus lineas que
            # existen en upstream NO estan cerca del ancla alli (o sea, son nuestras)
            tiene_eluna = any(added[t] and ELUNA.search(ours[t]) and not osig[t].startswith('//') for t in range(start, k))
            a = buscar_ancla(ours, osig, upsig, upsig_sign, start, k) if tiene_eluna else None
            ok = False
            if a:
                seq = a['seq']; n = len(seq)
                hit = next(t for t in range(len(upsig_sign) - n + 1) if upsig_sign[t:t+n] == seq)
                ventana = upsig_sign[hit + n: hit + n + (k - start) + 3] if a['position'] == 'after' else upsig_sign[max(0, hit - (k - start) - 3): hit]
                presentes = [osig[t] for t in range(start, k) if not TRIVIAL.match(osig[t]) and osig[t] in upsig]
                ok = not any(p in ventana for p in presentes)
                for key in ('ancla1', 'ancla2', 'ancla3', 'nth'):
                    pass
            if not ok:
                i = i_codigo + 1; continue   # el comentario no abria nada: seguir dentro
            stats['comentario'] = stats.get('comentario', 0) + 1
        for t in range(start, k): en_bloque[t] = True
        stats['bloques'] += 1

        if any(x in l for l in bloque for x in EXCLUIR):
            i = k; continue

        # ---- auxiliares: declaraciones nuestras mas arriba que el bloque usa ----
        usados = set(re.findall(r'\b[A-Za-z_]\w*\b', '\n'.join(bloque)))
        j, vistos, aux = start - 1, 0, []
        while j >= 0 and vistos < 8:
            if not TRIVIAL.match(osig[j]):
                vistos += 1
                m = DECL.match(ours[j])
                if added[j] and not en_bloque[j] and m and m.group(1) in usados and not ELUNA.search(ours[j]):
                    aux.append(j)
            j -= 1
        for j in sorted(aux):
            a = buscar_ancla(ours, osig, upsig, upsig_sign, j, j + 1)
            en_bloque[j] = True; stats['aux'] += 1
            entry = {'file': f, 'line': j + 1, 'anchor': a, 'block': [ours[j]]}
            (manifest if a else sin_ancla).append(entry)

        ancla = buscar_ancla(ours, osig, upsig, upsig_sign, start, k)
        entry = {'file': f, 'line': start + 1, 'anchor': ancla, 'block': bloque}
        (manifest if ancla else sin_ancla).append(entry)
        i = k

# la seccion "manual" se conserva tal cual: se edita a mano, no se regenera
MANIFEST = 'src/server/scripts/LuaEngine/patches/core_hooks.json'
manual_previo = json.load(open(MANIFEST, encoding='utf-8')).get('manual', []) if os.path.exists(MANIFEST) else []
json.dump({'base_upstream': base, 'hooks': manifest, 'manual': manual_previo},
          open(MANIFEST, 'w', encoding='utf-8'), ensure_ascii=False, indent=1)
print(stats, '  sin ancla:', len(sin_ancla))
for e in sin_ancla:
    print(f"  SIN ANCLA {e['file'].replace('src/server/game/','')}:{e['line']}  {e['block'][0].strip()[:80]}")
