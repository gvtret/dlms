#!/usr/bin/env bash
set -euo pipefail
cd /e/work/dlms

python - <<'PY'
import re, pathlib

cpp = pathlib.Path('lib/dlms-cosem/src/cosem/simple_objects.cpp').read_text(encoding='utf-8', errors='replace')

def extract_bodies(src, fn_name):
    out = {}
    pat = re.compile(r'CosemStatus\s+(Cosem\w+Object)::' + fn_name + r'\([^)]*\)\s*(?:const)?\s*\{', re.M)
    for m in pat.finditer(src):
        cls = m.group(1)
        i = m.end() - 1
        depth = 0
        start = i
        while i < len(src):
            ch = src[i]
            if ch == '{': depth += 1
            elif ch == '}':
                depth -= 1
                if depth == 0:
                    out[cls] = src[start:i+1]
                    break
            i += 1
    return out

read_bodies   = extract_bodies(cpp, 'ReadAttribute')
write_bodies  = extract_bodies(cpp, 'WriteAttribute')
invoke_bodies = extract_bodies(cpp, 'InvokeMethod')

attr_eq   = re.compile(r'(?:case\s+|attributeId\s*==\s*)(k\w*(?:AttributeId|Id))\b')
method_eq = re.compile(r'(?:case\s+|methodId\s*==\s*)(k\w*MethodId)\b')
num_attr  = re.compile(r'(?:case\s+|attributeId\s*==\s*)(\d+)\b')

# Range form: 'methodId >= kX && methodId <= kY' or with explicit numerics like 'methodId >= 6u && methodId <= 8u'.
def expand_method_range(body, src):
    """Find ranges and resolve constants -> integer list."""
    out = set()
    # numeric range like '6u && methodId <= 8u' or '6 && methodId <= 8'
    for m in re.finditer(r'methodId\s*>=\s*(\d+)u?\s*&&\s*methodId\s*<=\s*(\d+)u?', body):
        lo, hi = int(m.group(1)), int(m.group(2))
        for x in range(lo, hi+1):
            out.add(f'#{x}')
    # constant range
    for m in re.finditer(r'methodId\s*>=\s*(k\w*MethodId)\s*&&\s*methodId\s*<=\s*(k\w*MethodId)', body):
        klo, khi = m.group(1), m.group(2)
        lo = resolve_const(klo, src)
        hi = resolve_const(khi, src)
        if lo is not None and hi is not None:
            for x in range(lo, hi+1):
                out.add(f'#{x}')
    return out

def resolve_const(name, src):
    m = re.search(r'constexpr\s+std::uint8_t\s+' + re.escape(name) + r'\s*=\s*(\d+)u?', src)
    return int(m.group(1)) if m else None

rows = []
classes = sorted(set(list(read_bodies) + list(invoke_bodies) + list(write_bodies)))
for cls in classes:
    rb = read_bodies.get(cls, '')
    wb = write_bodies.get(cls, '')
    ib = invoke_bodies.get(cls, '')
    a = len(set(attr_eq.findall(rb))) or len(set(num_attr.findall(rb)))
    w = len(set(attr_eq.findall(wb))) or len(set(num_attr.findall(wb)))
    m_eq = set(method_eq.findall(ib))
    m_rng = expand_method_range(ib, cpp)
    # Add numeric '==' too
    for nm in re.finditer(r'methodId\s*==\s*(\d+)u?', ib):
        m_rng.add(f'#{nm.group(1)}')
    m = len(m_eq) + len(m_rng)
    rows.append((cls, a, w, m))

print(f"{'Class':50s} {'rdA':>4s} {'wrA':>4s} {'meth':>5s}")
print(f"{'-'*50} {'----':>4s} {'----':>4s} {'-----':>5s}")
for cls, a, w, m in rows:
    print(f"{cls:50s} {a:4d} {w:4d} {m:5d}")
PY
