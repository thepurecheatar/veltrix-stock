# regenerate src/offsets.h from a dumper SDK json.
# consumes output of a mapped uc-dumper stub or any dumper that emits
# { "GWorld": "0x...", "UWorld.PersistentLevel": 72, ... }

import json, sys, pathlib, re

TPL = pathlib.Path(__file__).parent.parent / "src" / "offsets.h"

def patch(name, value, src):
    pat = re.compile(rf"(constexpr\s+\w+\s+{re.escape(name)}\s*=\s*)[^;]+;")
    return pat.sub(lambda m: f"{m.group(1)}{value};", src, count=1)

def fmt(v):
    if isinstance(v, str) and v.startswith("0x"):
        return v
    if isinstance(v, int):
        return f"0x{v:X}"
    return str(v)

def main():
    if len(sys.argv) < 2:
        print("usage: sdk_to_offsets.py sdk.json", file=sys.stderr); sys.exit(2)
    data = json.loads(pathlib.Path(sys.argv[1]).read_text())
    src = TPL.read_text()
    for key, val in data.items():
        # flatten "A.B" -> last segment used as symbol name in offsets.h
        name = key.split(".")[-1]
        src = patch(name, fmt(val), src)
    TPL.write_text(src)
    print(f"patched {TPL}")

if __name__ == "__main__":
    main()
