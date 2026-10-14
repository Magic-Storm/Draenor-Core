"""Write local CASC layout: Data/data/data.NNN + bucket .idx journals."""
from __future__ import annotations

import hashlib
import socket
import struct
from dataclasses import dataclass, field
from pathlib import Path
from typing import Dict, List, Optional, Tuple

from jenkins import hashlittle, hashlittle2

# Agent checksum table (wowdev / Agent 8020)
_CHECKSUM_B_TABLE = [
    0x049396B8, 0x72A82A9B, 0xEE626CCA, 0x9917754F,
    0x15DE40B1, 0xF5A8A9B6, 0x421EAC7E, 0xA9D55C9A,
    0x317FD40C, 0x04FAF80D, 0x3D6BE971, 0x52933CFD,
    0x27F64B7D, 0xC6F5C11B, 0xD5757E3A, 0x6C388745,
]

HEADER_SIZE = 30
OFFSET_BITS = 30
OFFSET_MASK = (1 << OFFSET_BITS) - 1
# Stay under 30-bit offset with room for a large last record
MAX_DATA_FILE = 0x3C000000  # ~960 MiB
IDX_PAD = 0x10000


def bucket_index(ekey: bytes) -> int:
    """XOR-nibble bucket for first 9 bytes of EKey (wowdev)."""
    i = 0
    for b in ekey[:9]:
        i ^= b
    return (i & 0xF) ^ (i >> 4)


def _crossref_base_hash() -> bytes:
    host = socket.gethostname().encode("utf-8", errors="replace")
    h = hashlib.md5()
    h.update(host)
    h.update(b"data/data")
    digest = bytearray(h.digest())
    for i in range(9, 16):
        digest[i] = 0
    return bytes(digest)


def checksum_a(header22: bytes) -> int:
    return hashlittle(header22[:0x16], 0x3D6BE971)


def checksum_b(header: bytes, archive_index: int, archive_offset: int) -> int:
    offset = (archive_offset & OFFSET_MASK) | ((archive_index & 3) << 30)
    encoded_offset = _CHECKSUM_B_TABLE[(offset + 0x1E) & 0xF] ^ ((offset + 0x1E) & 0xFFFFFFFF)
    hb = bytearray(4)
    for i in range(0x1A):
        hb[(i + offset) & 3] ^= header[i]
    eo = encoded_offset.to_bytes(4, "little")
    cb = bytearray(4)
    for j in range(4):
        i = j + 0x1A + offset
        cb[j] = hb[i & 3] ^ eo[i & 3]
    return int.from_bytes(cb, "little")


def build_data_header(
    ekey: bytes,
    total_size: int,
    archive_index: int,
    archive_offset: int,
    *,
    flags: bytes = b"\x00\x00",
) -> bytes:
    """30-byte data.NNN record header (EKey reversed + size + checksums)."""
    if len(ekey) < 9:
        raise ValueError("ekey too short")
    key16 = (ekey + b"\x00" * 16)[:16]
    header = bytearray(30)
    header[0:16] = bytes(reversed(key16))
    struct.pack_into("<I", header, 16, total_size)
    header[20:22] = flags
    struct.pack_into("<I", header, 0x16, checksum_a(header))
    struct.pack_into("<I", header, 0x1A, checksum_b(header, archive_index, archive_offset))
    return bytes(header)


@dataclass
class IndexEntry:
    ekey9: bytes
    archive: int
    offset: int
    size: int

    def pack(self) -> bytes:
        loc = ((self.archive & 0x3FF) << OFFSET_BITS) | (self.offset & OFFSET_MASK)
        return self.ekey9[:9] + loc.to_bytes(5, "big") + struct.pack("<I", self.size)


@dataclass
class LocalCascWriter:
    """Accumulate BLTE records into data.NNN and emit .idx journals."""

    data_dir: Path
    idx_version: int = 1
    max_file: int = MAX_DATA_FILE
    _archive: int = 0
    _offset: int = 0
    _fh: Optional[object] = field(default=None, repr=False)
    _entries: Dict[int, List[IndexEntry]] = field(default_factory=dict)
    _seen: set = field(default_factory=set)
    _written: int = 0
    _bytes: int = 0

    def __post_init__(self) -> None:
        self.data_dir.mkdir(parents=True, exist_ok=True)
        for b in range(16):
            self._entries[b] = []

    def _open_archive(self, index: int) -> None:
        if self._fh is not None:
            self._fh.close()
        path = self.data_dir / f"data.{index:03d}"
        self._fh = open(path, "wb")
        self._archive = index
        self._offset = 0
        self._write_crossrefs()

    def _write_crossrefs(self) -> None:
        base = bytearray(_crossref_base_hash())
        for i in range(16):
            key = bytearray(base)
            key[0] = i
            key[1] = self._archive & 0xFF
            self._write_record(bytes(key), b"", flags=b"\x01\x00", indexable=True)

    def _ensure_space(self, need: int) -> None:
        if self._fh is None:
            self._open_archive(0)
            return
        if self._offset + need > self.max_file or self._offset + need > OFFSET_MASK:
            self._open_archive(self._archive + 1)

    def _write_record(
        self,
        ekey: bytes,
        blte: bytes,
        *,
        flags: bytes = b"\x00\x00",
        indexable: bool = True,
    ) -> Optional[Tuple[int, int, int]]:
        total = HEADER_SIZE + len(blte)
        self._ensure_space(total)
        assert self._fh is not None
        arch, off = self._archive, self._offset
        header = build_data_header(ekey, total, arch, off, flags=flags)
        self._fh.write(header)
        if blte:
            self._fh.write(blte)
        self._offset += total
        self._bytes += total
        if indexable:
            ekey9 = (ekey + b"\x00" * 9)[:9]
            if ekey9 not in self._seen:
                self._seen.add(ekey9)
                bucket = bucket_index(ekey)
                # Cross-refs use (bucket+1)%16 per wowdev
                if flags == b"\x01\x00":
                    bucket = (bucket + 1) % 16
                self._entries[bucket].append(
                    IndexEntry(ekey9=ekey9, archive=arch, offset=off, size=total)
                )
            self._written += 1
        return arch, off, total

    def write_blte(self, ekey: bytes, blte: bytes) -> bool:
        """Write one BLTE blob. Returns False if this EKey was already stored."""
        ekey9 = (ekey + b"\x00" * 9)[:9]
        if ekey9 in self._seen:
            return False
        if not blte.startswith(b"BLTE") and len(blte) > 0:
            # Allow raw non-BLTE (rare); still store as payload
            pass
        self._write_record(ekey, blte, flags=b"\x00\x00", indexable=True)
        return True

    def close(self) -> None:
        if self._fh is not None:
            self._fh.close()
            self._fh = None
        self._write_indices()

    def _write_indices(self) -> None:
        for bucket in range(16):
            entries = self._entries[bucket]
            if not entries:
                continue
            entries.sort(key=lambda e: e.ekey9)
            body = b"".join(e.pack() for e in entries)
            self._write_idx(bucket, body)

    def _write_idx(self, bucket: int, entries_blob: bytes) -> None:
        # Header block (16 bytes) hashed with hashlittle2
        hdr = bytearray(16)
        struct.pack_into("<H", hdr, 0, 7)  # version
        hdr[2] = bucket
        hdr[3] = 0  # ExtraBytes
        hdr[4] = 4  # size bytes
        hdr[5] = 5  # offset bytes
        hdr[6] = 9  # key bytes
        hdr[7] = OFFSET_BITS
        struct.pack_into("<Q", hdr, 8, 0x4000000000)

        pc, _pb = hashlittle2(bytes(hdr), 0, 0)
        # SizeAndHash + header, then pad to 16-byte boundary before entries SizeAndHash
        # Layout: [4 size][4 hash][16 header][pad to align][4 esize][4 ehash][entries...][pad 0x10000]
        prefix = struct.pack("<II", 16, pc) + bytes(hdr)
        # CascLib: FilePosition = (8 + HeaderHashSize + 0x0F) & ~0x0F  → 32 when size=16
        file_pos = (8 + 16 + 0x0F) & ~0x0F
        prefix = prefix.ljust(file_pos, b"\x00")

        pc_e = pb_e = 0
        for i in range(0, len(entries_blob), 18):
            pc_e, pb_e = hashlittle2(entries_blob[i : i + 18], pc_e, pb_e)

        block = prefix + struct.pack("<II", len(entries_blob), pc_e) + entries_blob
        # Pad so CascLib sees >= 0x7800 after 0x1000 align (Agent uses 64 KiB files)
        if len(block) < IDX_PAD:
            block = block.ljust(IDX_PAD, b"\x00")
        else:
            # Align up to next 0x1000 then ensure 0x7800 tail
            aligned = (len(block) + 0xFFF) & ~0xFFF
            need = aligned + 0x7800
            if len(block) < need:
                block = block.ljust(need, b"\x00")

        name = f"{bucket:02x}{self.idx_version:08x}.idx"
        (self.data_dir / name).write_bytes(block)

    @property
    def stats(self) -> Tuple[int, int, int]:
        """(records, bytes, archive_count)."""
        return self._written, self._bytes, self._archive + 1
