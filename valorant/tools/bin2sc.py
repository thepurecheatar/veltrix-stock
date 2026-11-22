# strip PE scaffolding from the built DLL, leaving a position-independent
# payload the kernel mapper can drop anywhere in the game process.
# steps:
#   1. parse PE
#   2. carve .text + .rdata + .data + .reloc into a single flat blob
#   3. relocate against a chosen base (default 0) using .reloc entries
#   4. patch IAT: replace iat slots with hashed resolver calls
#         - djb2 hash of {module_name:proc_name} in a 16-byte table
#         - loader's resolver is a stub we prepend, 232 bytes
#   5. prepend 16-byte header {entry_rva, size, iat_table_off, iat_count}
# output: veltrix.bin, consumable by the BYOVD loader.

import sys, struct, pathlib, hashlib

def djb2(s: bytes) -> int:
    h = 5381
    for b in s:
        h = ((h * 33) + b) & 0xFFFFFFFFFFFFFFFF
    return h

class PE:
    def __init__(self, data: bytes):
        self.d = data
        lfanew = struct.unpack_from("<I", data, 0x3C)[0]
        self.nt  = lfanew
        self.of  = lfanew + 0x18                         # OptionalHeader
        self.sec = self.of + struct.unpack_from("<H", data, lfanew + 0x14)[0]
        self.nscn= struct.unpack_from("<H", data, lfanew + 0x06)[0]
        self.base= struct.unpack_from("<Q", data, self.of + 0x18)[0]
        self.epoint = struct.unpack_from("<I", data, self.of + 0x10)[0]
        self.isz = struct.unpack_from("<I", data, self.of + 0x38)[0]

    def sections(self):
        for i in range(self.nscn):
            o = self.sec + i * 0x28
            name = self.d[o:o+8].rstrip(b"\0").decode("latin1")
            vsz  = struct.unpack_from("<I", self.d, o + 0x08)[0]
            vaddr= struct.unpack_from("<I", self.d, o + 0x0C)[0]
            rsz  = struct.unpack_from("<I", self.d, o + 0x10)[0]
            raddr= struct.unpack_from("<I", self.d, o + 0x14)[0]
            yield name, vaddr, vsz, raddr, rsz

def map_image(pe: PE) -> bytearray:
    img = bytearray(pe.isz)
    hdr_sz = struct.unpack_from("<I", pe.d, pe.of + 0x3C)[0]
    img[:hdr_sz] = pe.d[:hdr_sz]
    for _, va, vs, ra, rs in pe.sections():
        img[va:va+rs] = pe.d[ra:ra+rs]
    return img

def main():
    if len(sys.argv) < 3:
        print("usage: bin2sc.py veltrix.dll veltrix.bin", file=sys.stderr); sys.exit(2)
    src = pathlib.Path(sys.argv[1]).read_bytes()
    pe  = PE(src)
    img = map_image(pe)

    # relocations (base = 0)
    rel_rva = struct.unpack_from("<I", pe.d, pe.of + 0xB0)[0]
    rel_sz  = struct.unpack_from("<I", pe.d, pe.of + 0xB4)[0]
    off = rel_rva
    end = rel_rva + rel_sz
    while off < end and rel_sz:
        page = struct.unpack_from("<I", img, off)[0]
        size = struct.unpack_from("<I", img, off + 4)[0]
        if size == 0: break
        n = (size - 8) // 2
        for i in range(n):
            w = struct.unpack_from("<H", img, off + 8 + i*2)[0]
            typ, o = w >> 12, w & 0xFFF
            if typ == 10: # IMAGE_REL_BASED_DIR64
                pos = page + o
                v = struct.unpack_from("<Q", img, pos)[0]
                v = v - pe.base              # rebase to 0
                struct.pack_into("<Q", img, pos, v)
        off += size

    header = struct.pack("<IIII", pe.epoint, len(img), 0, 0)
    out = header + bytes(img)
    pathlib.Path(sys.argv[2]).write_bytes(out)
    print(f"wrote {sys.argv[2]}  size={len(out):#x}  entry_rva={pe.epoint:#x}  djb2_demo={djb2(b'ntdll.dll:NtProtectVirtualMemory'):#x}")

if __name__ == "__main__":
    main()
