#!/usr/bin/env python3
"""
Fill ekeys missing from local Data because Zephyr (or another mirror) served
truncated CDN archives. Prefer archive.wow.tools / casc.wago.tools for the
same archive keys — download only archives that are larger there, slice the
missing BLTE blobs into existing data.NNN / .idx.

Does not wipe Data; resume-safe.
"""
from __future__ import annotations

import argparse
import sys
from pathlib import Path

from archive import parse_archive_index
from cdn import CdnClient, DEFAULT_BASE
from convert_cdn_to_data import _loc_plausible, convert_archives
from download_client import Progress, human
from local_casc import LocalCascWriter
from tact import archive_keys_from_cdnconfig, parse_kv_config

# Community archives that still host full WoD-era blobs (Zephyr often truncates).
FILL_BASES = [
    "https://archive.wow.tools/tpr/wow",
    "https://casc.wago.tools/tpr/wow",
]


def main() -> int:
    ap = argparse.ArgumentParser(description="Fill truncated CDN archives into local Data")
    ap.add_argument("--out", type=Path, default=Path("."), help="Client root (has Data/)")
    ap.add_argument("--cache", type=Path, default=Path("_cdn_cache_fill"))
    ap.add_argument(
        "--primary",
        default=DEFAULT_BASE,
        help="CDN used for configs / index list (default: Zephyr)",
    )
    ap.add_argument(
        "--fill-base",
        action="append",
        default=[],
        help="Mirror to pull full archives from (repeatable)",
    )
    ap.add_argument("--kind", choices=("data", "patch", "both"), default="both")
    args = ap.parse_args()

    fill_bases = args.fill_base or list(FILL_BASES)
    client = args.out.resolve()
    data_dir = client / "Data" / "data"
    if not data_dir.is_dir():
        print(f"Missing {data_dir}", file=sys.stderr)
        return 1

    args.cache.mkdir(parents=True, exist_ok=True)
    primary = CdnClient(args.primary, args.cache)
    progress = Progress()

    from download_client import pick_version_row
    from tact import parse_versions_table

    row = pick_version_row(parse_versions_table(primary.fetch_text("versions")), "us")
    if not row:
        print("No versions row", file=sys.stderr)
        return 1
    cdn_config = row["CDNConfig"].lower()
    cdncfg_path = primary.download_key(cdn_config, kind="config", progress_cb=progress)
    cdn_cfg = parse_kv_config(cdncfg_path.read_text(encoding="utf-8", errors="replace"))

    jobs: list[tuple[str, str]] = []
    if args.kind in ("data", "both"):
        jobs.extend(("data", k) for k in archive_keys_from_cdnconfig(cdn_cfg))
    if args.kind in ("patch", "both"):
        jobs.extend(("patch", k) for k in cdn_cfg.get("patch-archives", []))

    print(f"==> Scanning {len(jobs)} archives for truncation vs fill mirrors", flush=True)
    fill_clients = [CdnClient(b, args.cache) for b in fill_bases]
    truncated: list[tuple[str, str, int, str, int]] = []
    for n, (kind, key) in enumerate(jobs, 1):
        if n == 1 or n % 25 == 0 or n == len(jobs):
            print(f"  … scan {n}/{len(jobs)}", flush=True)
        primary_sz = primary.head_size(key, kind=kind) or 0
        best_base = None
        best_sz = primary_sz
        for base, fc in zip(fill_bases, fill_clients):
            sz = fc.head_size(key, kind=kind) or 0
            if sz > best_sz + 1024:
                best_base, best_sz = base, sz
                break  # first fuller mirror is enough
        if best_base and best_sz > primary_sz + 1024:
            truncated.append((kind, key, primary_sz, best_base, best_sz))
            print(
                f"  trunc {kind} {key[:16]}… "
                f"primary={human(primary_sz)} fill={human(best_sz)} @ {best_base}",
                flush=True,
            )

    if not truncated:
        print("No truncated archives detected (or fill mirrors unreachable).")
        return 0

    print(f"==> Filling {len(truncated)} archives into {data_dir}")
    writer = LocalCascWriter.open_existing(data_dir)
    try:
        # Indexes often exist only on the primary (Zephyr); full blobs on fill mirrors.
        # Prefetch Zephyr .index into the shared cache, then download archives from fill.
        for kind, key, _ps, base, _fs in truncated:
            try:
                primary.download_key(key, kind=kind, index=True, progress_cb=progress)
            except Exception as exc:  # noqa: BLE001
                print(f"  warn: primary index {kind} {key[:16]}…: {exc}", flush=True)

        by_base: dict[str, list[tuple[str, str]]] = {}
        for kind, key, _ps, base, _fs in truncated:
            by_base.setdefault(base, []).append((kind, key))

        for base, items in by_base.items():
            cdn = CdnClient(base, args.cache)
            for kind, keys in (
                ("data", [k for knd, k in items if knd == "data"]),
                ("patch", [k for knd, k in items if knd == "patch"]),
            ):
                if not keys:
                    continue
                print(f"==> {base} kind={kind} count={len(keys)}")
                convert_archives(
                    cdn,
                    keys,
                    writer,
                    progress,
                    keep_cdn_cache=False,
                    kind=kind,
                )
    finally:
        writer.close()

    recs, nbytes, narch = writer.stats
    print(f"==> Done. idx records={recs}, data={human(nbytes)}, files={narch}")
    print("Re-run Extract.bat / mapextractor for missing ADTs.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
