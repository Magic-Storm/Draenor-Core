"""TACT config / encoding / install parsers for WoD-era builds."""
from __future__ import annotations

import struct
from dataclasses import dataclass
from typing import Dict, Iterable, List, Optional, Set, Tuple

from blte import decode_blte


def parse_kv_config(text: str) -> Dict[str, List[str]]:
    result: Dict[str, List[str]] = {}
    for line in text.splitlines():
        line = line.strip()
        if not line or line.startswith("#") or "=" not in line:
            continue
        key, value = line.split("=", 1)
        result[key.strip()] = value.strip().split()
    return result


def parse_versions_table(text: str) -> List[Dict[str, str]]:
    lines = [ln for ln in text.splitlines() if ln.strip()]
    if len(lines) < 2:
        return []
    headers = [h.split("!")[0] for h in lines[0].split("|")]
    rows = []
    for line in lines[1:]:
        cols = line.split("|")
        rows.append({headers[i]: cols[i] if i < len(cols) else "" for i in range(len(headers))})
    return rows


@dataclass
class InstallEntry:
    path: str
    ckey: bytes
    size: int
    tags: Set[str]


@dataclass
class InstallManifest:
    entries: List[InstallEntry]
    tags: Dict[str, int]  # name -> type

    def filter_tags(self, required: Iterable[str]) -> List[InstallEntry]:
        need = set(required)
        if not need:
            return list(self.entries)
        out = []
        for e in self.entries:
            if need.issubset(e.tags) or (e.tags & need and len(need) == 1):
                # Prefer entries matching ALL required tags when possible
                if need.issubset(e.tags):
                    out.append(e)
        if out:
            return out
        # Fallback: entry must match every required tag that exists on any entry,
        # otherwise match if it has all of the tags that apply to platform/locale.
        for e in self.entries:
            if need <= e.tags:
                out.append(e)
        return out


class EncodingTable:
    """CKey (content MD5) -> list of EKeys (CDN hashes)."""

    def __init__(self):
        self.ckey_to_ekeys: Dict[bytes, List[bytes]] = {}

    def get_ekey(self, ckey: bytes | str) -> Optional[bytes]:
        if isinstance(ckey, str):
            ckey = bytes.fromhex(ckey)
        ekeys = self.ckey_to_ekeys.get(ckey)
        return ekeys[0] if ekeys else None

    @classmethod
    def parse(cls, data: bytes) -> "EncodingTable":
        # Encoding may be BLTE-wrapped
        if data.startswith(b"BLTE"):
            data = decode_blte(data)

        if data[0:2] != b"EN":
            raise ValueError("not an encoding file")

        version = data[2]
        hash_size_ckey = data[3]
        hash_size_ekey = data[4]
        ckey_page_size_kb = struct.unpack(">H", data[5:7])[0]
        ekey_page_size_kb = struct.unpack(">H", data[7:9])[0]
        ckey_page_count = struct.unpack(">I", data[9:13])[0]
        ekey_page_count = struct.unpack(">I", data[13:17])[0]
        _unk = data[17]
        espec_size = struct.unpack(">I", data[18:22])[0]

        pos = 22 + espec_size
        # CE key page index table: pageCount * (ckey + page hash)
        pos += ckey_page_count * (hash_size_ckey + 16)

        table = cls()
        page_size = ckey_page_size_kb * 1024
        for _ in range(ckey_page_count):
            page = data[pos : pos + page_size]
            pos += page_size
            table._parse_ckey_page(page, hash_size_ckey, hash_size_ekey)

        return table

    def _parse_ckey_page(self, page: bytes, ckey_len: int, ekey_len: int) -> None:
        off = 0
        length = len(page)
        while off + 6 < length:
            key_count = page[off]
            # 0 key_count marks end of page entries
            if key_count == 0:
                break
            # uint40 BE file size
            size = int.from_bytes(page[off + 1 : off + 6], "big")
            off += 6
            if off + ckey_len + key_count * ekey_len > length:
                break
            ckey = page[off : off + ckey_len]
            off += ckey_len
            ekeys = []
            for _ in range(key_count):
                ekeys.append(page[off : off + ekey_len])
                off += ekey_len
            self.ckey_to_ekeys[ckey] = ekeys
            _ = size


def parse_install(data: bytes) -> InstallManifest:
    if data.startswith(b"BLTE"):
        data = decode_blte(data)
    if data[0:2] != b"IN":
        raise ValueError("not an install manifest")

    version = data[2]
    hash_size = data[3]
    num_tags = struct.unpack(">H", data[4:6])[0]
    num_entries = struct.unpack(">I", data[6:10])[0]
    pos = 10

    if version >= 2:
        # optional extra header bytes seen on later builds; WoD is typically v1
        pass

    bitmask_bytes = (num_entries + 7) // 8
    tags: Dict[str, int] = {}
    tag_masks: List[Tuple[str, int, bytes]] = []

    for _ in range(num_tags):
        # name\0, type uint16 BE, bitmask
        end = data.index(b"\x00", pos)
        name = data[pos:end].decode("utf-8", errors="replace")
        pos = end + 1
        tag_type = struct.unpack(">H", data[pos : pos + 2])[0]
        pos += 2
        mask = data[pos : pos + bitmask_bytes]
        pos += bitmask_bytes
        tags[name] = tag_type
        tag_masks.append((name, tag_type, mask))

    entries: List[InstallEntry] = []
    for i in range(num_entries):
        end = data.index(b"\x00", pos)
        path = data[pos:end].decode("utf-8", errors="replace")
        pos = end + 1
        ckey = data[pos : pos + hash_size]
        pos += hash_size
        size = struct.unpack(">I", data[pos : pos + 4])[0]
        pos += 4

        file_tags: Set[str] = set()
        byte_index = i // 8
        bit_index = 7 - (i % 8)  # MSB first
        for name, _t, mask in tag_masks:
            if byte_index < len(mask) and (mask[byte_index] >> bit_index) & 1:
                file_tags.add(name)
        entries.append(InstallEntry(path=path, ckey=ckey, size=size, tags=file_tags))

    return InstallManifest(entries=entries, tags=tags)


def archive_keys_from_cdnconfig(cfg: Dict[str, List[str]]) -> List[str]:
    # Individual archives only. archive-group is a rolled-up index and often absent as .index.
    return list(cfg.get("archives", []))
