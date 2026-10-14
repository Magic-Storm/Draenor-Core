"""Minimal BLTE decoder (N / Z / F)."""
from __future__ import annotations

import struct
import zlib


class BlteError(ValueError):
    pass


def decode_blte(data: bytes) -> bytes:
    if len(data) < 8 or data[0:4] != b"BLTE":
        raise BlteError("not a BLTE file")

    header_size = struct.unpack(">I", data[4:8])[0]
    if header_size == 0:
        return _decode_block(data[8:])

    if len(data) < header_size:
        raise BlteError("truncated BLTE header")

    table_format = data[8]
    num_blocks = int.from_bytes(data[9:12], "big")
    if num_blocks == 0:
        raise BlteError("BLTE has zero blocks")

    # 0x0F: raw(4)+logical(4)+hash(16) = 24
    # 0x10: raw(4)+logical(4)+hash(16)+uchash(16) = 40
    if table_format == 0x10:
        frame = 40
    elif table_format in (0x0F, 15):
        frame = 24
    else:
        raise BlteError(f"unsupported BLTE table format 0x{table_format:02x}")

    pos = 12
    block_meta = []
    for _ in range(num_blocks):
        raw_size, logical_size = struct.unpack(">II", data[pos : pos + 8])
        # wowdev: rawSize = compressed (incl mode byte), logicalSize = decompressed
        block_meta.append((raw_size, logical_size))
        pos += frame

    out = bytearray()
    data_pos = header_size
    for raw_size, _logical in block_meta:
        block = data[data_pos : data_pos + raw_size]
        if len(block) != raw_size:
            raise BlteError("truncated BLTE block")
        out.extend(_decode_block(block))
        data_pos += raw_size
    return bytes(out)


def _decode_block(block: bytes) -> bytes:
    if not block:
        raise BlteError("empty BLTE block")
    mode = chr(block[0])
    payload = block[1:]
    if mode == "N":
        return payload
    if mode == "Z":
        return zlib.decompress(payload)
    if mode == "F":
        return decode_blte(payload)
    raise BlteError(f"unsupported BLTE mode '{mode}' (encrypted/lz4 not handled)")
