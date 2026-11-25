import struct
import sys

path = sys.argv[1]
with open(path, "r+b") as f:
    header = bytearray(f.read(0x40))
    assert header[0x10:0x14] == b"NRO0"
    end = max(
        struct.unpack_from("<I", header, 0x20 + i * 8)[0]
        + struct.unpack_from("<I", header, 0x24 + i * 8)[0]
        for i in range(3)
    )
    struct.pack_into("<I", header, 0x18, end)
    f.seek(0x18)
    f.write(header[0x18:0x1C])
