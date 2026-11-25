import struct
import sys

nro_path, elf_path = sys.argv[1], sys.argv[2]

elf = open(elf_path, "rb").read()
e_phoff = struct.unpack_from("<Q", elf, 0x20)[0]
e_phentsize = struct.unpack_from("<H", elf, 0x36)[0]
e_phnum = struct.unpack_from("<H", elf, 0x38)[0]

loads = []
for i in range(e_phnum):
    off = e_phoff + i * e_phentsize
    p_type = struct.unpack_from("<I", elf, off)[0]
    if p_type == 1:
        loads.append(struct.unpack_from("<QQQQQQQQ", elf, off))
assert len(loads) == 3

with open(nro_path, "r+b") as f:
    header = bytearray(f.read(0x40))
    assert header[0x10:0x14] == b"NRO0"
    old_size = struct.unpack_from("<I", header, 0x18)[0]
    segments = [
        struct.unpack_from("<II", header, 0x20 + i * 8) for i in range(3)
    ]
    new_size = max(off + size for off, size in segments)
    if new_size <= old_size:
        sys.exit(0)

    f.seek(0)
    asset = f.read()[old_size:]

    data_off, data_size = segments[2]
    elf_offset = loads[2][1] + (old_size - data_off)
    restore = elf[elf_offset:elf_offset + (new_size - old_size)]
    assert len(restore) == new_size - old_size

    f.seek(old_size)
    f.write(restore)
    f.seek(new_size)
    f.write(asset)
    f.truncate(new_size + len(asset))

    struct.pack_into("<I", header, 0x18, new_size)
    f.seek(0x18)
    f.write(header[0x18:0x1C])
