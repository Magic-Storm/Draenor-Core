#!/usr/bin/env python3
"""
Inject missing ENCODING / ROOT / DOWNLOAD / INSTALL into an existing local Data.

CascLib opens storage via the encoding EKey in .idx — if conversion skipped that
archive, mapextractor fails with BAD_FORMAT even when data.* looks huge.
"""
from __future__ import annotations

import argparse
import sys
from pathlib import Path

from archive import ArchiveIndex, build_index_from_cdn, resolve_ekey_bytes
from cdn import CdnClient, DEFAULT_BASE
from download_client import Progress, human, pick_version_row
from local_casc import LocalCascWriter
from tact import (
    EncodingTable,
    archive_keys_from_cdnconfig,
    parse_kv_config,
    parse_versions_table,
)


def main() -> int:
    ap = argparse.ArgumentParser(description="Repair local CASC: add encoding/root/…")
    ap.add_argument("--out", required=True, type=Path, help="Client folder")
    ap.add_argument("--base", default=DEFAULT_BASE)
    ap.add_argument("--region", default="us")
    ap.add_argument("--cache", type=Path, default=None)
    args = ap.parse_args()

    client = args.out
    data_dir = client / "Data" / "data"
    if not data_dir.is_dir():
        print(f"No Data/data at {data_dir}", file=sys.stderr)
        return 1

    cache = args.cache or (client / "_cdn_cache")
    cache.mkdir(parents=True, exist_ok=True)
    cdn = CdnClient(base_url=args.base, cache_dir=cache)
    progress = Progress()

    print("==> versions / configs")
    rows = parse_versions_table(cdn.fetch_text("versions"))
    row = pick_version_row(rows, args.region)
    if not row:
        print("No versions row", file=sys.stderr)
        return 1

    build_config = row["BuildConfig"].lower()
    cdn_config = row["CDNConfig"].lower()
    build_path = cdn.download_key(build_config, kind="config", progress_cb=progress)
    cdncfg_path = cdn.download_key(cdn_config, kind="config", progress_cb=progress)
    build_cfg = parse_kv_config(build_path.read_text(encoding="utf-8", errors="replace"))
    cdn_cfg = parse_kv_config(cdncfg_path.read_text(encoding="utf-8", errors="replace"))
    archives = archive_keys_from_cdnconfig(cdn_cfg)

    encoding_keys = build_cfg.get("encoding", [])
    if len(encoding_keys) < 2:
        print("Build config missing encoding", file=sys.stderr)
        return 1
    encoding_ckey, encoding_ekey = encoding_keys[0], encoding_keys[1]

    print("==> Loading archive indices (for CDN lookups)")
    archive_index = build_index_from_cdn(cdn, archives)

    print("==> Fetch encoding")
    enc_blte = resolve_ekey_bytes(
        cdn, bytes.fromhex(encoding_ekey), archive_index=archive_index, progress_cb=progress
    )
    encoding = EncodingTable.parse(enc_blte)

    # Build-config content keys that CascLib must resolve via encoding -> ekey -> idx
    needed: list[tuple[str, bytes]] = [("encoding", bytes.fromhex(encoding_ekey))]
    for name in ("root", "download", "install", "patch"):
        vals = build_cfg.get(name, [])
        if not vals:
            continue
        ckey = vals[0]
        ekey = encoding.get_ekey(ckey)
        if not ekey:
            print(f"  warn: no ekey for {name} ckey={ckey}")
            continue
        needed.append((name, ekey))

    print(f"==> Opening existing Data/data ({data_dir})")
    writer = LocalCascWriter.open_existing(data_dir)
    added = 0
    try:
        for name, ekey in needed:
            if writer.has_ekey(ekey):
                print(f"  OK  {name} already present ({ekey.hex()[:16]}…)")
                continue
            print(f"  ADD {name} {ekey.hex()[:16]}…")
            blob = resolve_ekey_bytes(cdn, ekey, archive_index=archive_index, progress_cb=progress)
            if writer.write_blte(ekey, blob):
                added += 1
                print(f"      wrote {human(len(blob))}")
            else:
                print("      skipped (duplicate)")
    finally:
        writer.close()

    print()
    print(f"Done. Added {added} essential file(s). Re-run Extract.bat / mapextractor.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
