"""CDN archive index parsing and BLTE slice extraction."""
from __future__ import annotations

import struct
import urllib.request
from dataclasses import dataclass
from pathlib import Path
from typing import Dict, Optional, Tuple

from cdn import USER_AGENT, CdnClient


@dataclass
class ArchiveLoc:
    archive_key: str
    offset: int
    size: int


def _parse_footer(data: bytes) -> Tuple[int, int, int, int, int]:
    """
    Return (footer_size, page_size, key_bytes, size_bytes, offset_bytes).
    WoD-era footer is 28 bytes:
      [0:8] toc hash-ish, [8]=version, [9..10]=unk,
      [11]=blockSizeKB, [12]=offsetBytes, [13]=sizeBytes, [14]=keyBytes, [15]=hashSize,
      [16:20]=numElements (LE), [20:28]=footer checksum
    """
    if len(data) < 28:
        return 0, 4096, 16, 4, 4
    footer = data[-28:]
    block_size_kb = footer[11]
    offset_bytes = footer[12]
    size_bytes = footer[13]
    key_bytes = footer[14]
    if key_bytes not in (9, 16) or block_size_kb == 0 or offset_bytes == 0 or size_bytes == 0:
        return 28, 4096, 16, 4, 4
    return 28, block_size_kb * 1024, key_bytes, size_bytes, offset_bytes


def parse_archive_index(data: bytes, archive_key: str) -> Dict[bytes, ArchiveLoc]:
    """Parse a paged archive .index (16+4+4 entries, 16-byte page hash) into ekey -> location."""
    if len(data) < 28:
        return {}

    footer_size, page_size, key_bytes, size_bytes, offset_bytes = _parse_footer(data)
    entry_size = key_bytes + size_bytes + offset_bytes
    page_hash = 16
    body = data[:-footer_size] if footer_size else data

    out: Dict[bytes, ArchiveLoc] = {}
    pos = 0
    while pos < len(body):
        chunk = min(page_size, len(body) - pos)
        page = body[pos : pos + chunk]
        pos += chunk
        if chunk == page_size and chunk > page_hash:
            usable = page[:-page_hash]
        else:
            # last page: strip trailing hash if present / keep aligned records
            if len(page) > page_hash and (len(page) % entry_size) != 0:
                usable = page[:-page_hash]
            else:
                usable = page
        limit = len(usable) - (len(usable) % entry_size)
        for off in range(0, limit, entry_size):
            rec = usable[off : off + entry_size]
            ekey_raw = rec[0:key_bytes]
            # Pad truncated keys to 16 bytes for map lookups against full ekeys
            ekey = ekey_raw + (b"\x00" * (16 - len(ekey_raw))) if len(ekey_raw) < 16 else ekey_raw
            size_off = key_bytes
            offset_off = key_bytes + size_bytes
            size = int.from_bytes(rec[size_off : size_off + size_bytes], "big")
            offset = int.from_bytes(rec[offset_off : offset_off + offset_bytes], "big")
            out[ekey] = ArchiveLoc(archive_key=archive_key, offset=offset, size=size)
            if key_bytes < 16:
                # also store by raw truncated key for prefix matching helpers
                out[ekey_raw] = ArchiveLoc(archive_key=archive_key, offset=offset, size=size)
    return out


class ArchiveIndex:
    def __init__(self):
        self.map: Dict[bytes, ArchiveLoc] = {}

    def add_index(self, archive_key: str, data: bytes) -> int:
        parsed = parse_archive_index(data, archive_key)
        self.map.update(parsed)
        return len(parsed)

    def find(self, ekey: bytes | str) -> Optional[ArchiveLoc]:
        if isinstance(ekey, str):
            ekey = bytes.fromhex(ekey)
        hit = self.map.get(ekey)
        if hit:
            return hit
        # Truncated-key indices (9-byte): match on prefix key stored during parse
        if len(ekey) == 16:
            return self.map.get(ekey[:9])
        return None


def build_index_from_cdn(
    cdn: CdnClient,
    archive_keys: list[str],
    progress_cb=None,
) -> ArchiveIndex:
    idx = ArchiveIndex()
    total = len(archive_keys)
    for i, key in enumerate(archive_keys, 1):
        if progress_cb:
            progress_cb(i, total, key)
        try:
            path = cdn.download_key(key, kind="data", index=True)
            n = idx.add_index(key, path.read_bytes())
            print(f"  index {i}/{total} {key} (+{n} entries, total {len(idx.map)})")
        except Exception as exc:  # noqa: BLE001
            print(f"  warn: index {key}: {exc}")
    return idx


def fetch_archive_slice(cdn: CdnClient, loc: ArchiveLoc, progress_cb=None) -> bytes:
    """Download only the needed byte range from an archive (or whole file if cached)."""
    assert cdn.cache_dir is not None
    local = cdn.local_path(loc.archive_key, kind="data")
    if local.exists() and local.stat().st_size >= loc.offset + loc.size:
        with open(local, "rb") as f:
            f.seek(loc.offset)
            return f.read(loc.size)

    # HTTP Range request against CDN
    url = cdn.url_for(loc.archive_key, kind="data")
    headers = {
        "User-Agent": USER_AGENT,
        "Range": f"bytes={loc.offset}-{loc.offset + loc.size - 1}",
    }
    req = urllib.request.Request(url, headers=headers)
    with urllib.request.urlopen(req, timeout=120) as resp:
        data = resp.read()
    if len(data) != loc.size:
        # Some servers ignore Range — fall back to full archive download
        path = cdn.download_key(loc.archive_key, kind="data", progress_cb=progress_cb)
        with open(path, "rb") as f:
            f.seek(loc.offset)
            data = f.read(loc.size)
    if progress_cb:
        progress_cb(loc.size, loc.size, loc.archive_key)
    return data


def _try_loose(cdn: CdnClient, ekey_hex: str) -> Optional[bytes]:
    assert cdn.cache_dir is not None
    local = cdn.local_path(ekey_hex, kind="data")
    if local.exists() and local.stat().st_size > 0:
        return local.read_bytes()
    url = cdn.url_for(ekey_hex, kind="data")
    try:
        req = urllib.request.Request(url, method="HEAD", headers={"User-Agent": USER_AGENT})
        with urllib.request.urlopen(req, timeout=30) as resp:
            if resp.status >= 400:
                return None
    except Exception:
        # Some CDNs dislike HEAD — probe with GET range
        try:
            req = urllib.request.Request(
                url, headers={"User-Agent": USER_AGENT, "Range": "bytes=0-0"}
            )
            with urllib.request.urlopen(req, timeout=30) as resp:
                if resp.status >= 400:
                    return None
        except Exception:
            return None
    path = cdn.download_key(ekey_hex, kind="data")
    return path.read_bytes()


def resolve_ekey_bytes(
    cdn: CdnClient,
    ekey: bytes,
    archive_index: Optional[ArchiveIndex] = None,
    progress_cb=None,
) -> bytes:
    ekey_hex = ekey.hex()
    loose = _try_loose(cdn, ekey_hex)
    if loose is not None:
        if progress_cb:
            progress_cb(len(loose), len(loose), ekey_hex)
        return loose

    if archive_index is None:
        raise FileNotFoundError(f"ekey {ekey_hex} not a loose file and no archive index loaded")
    loc = archive_index.find(ekey)
    if not loc:
        raise FileNotFoundError(f"ekey {ekey_hex} not found in archive indices")
    return fetch_archive_slice(cdn, loc, progress_cb=progress_cb)
