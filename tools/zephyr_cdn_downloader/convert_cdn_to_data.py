#!/usr/bin/env python3
"""
Convert Zephyr CDN archives into local CASC Data layout:

  client/
    .build.info
    Wow-64.exe          (optional, via --with-install)
    Data/
      config/…          (BuildConfig + CDNConfig)
      data/
        data.000 …
        XX00000001.idx

This is what CascLib / mapextractor expect — NOT a raw CDN mirror.
"""
from __future__ import annotations

import argparse
import shutil
import subprocess
import sys
import tempfile
import time
from pathlib import Path

from archive import ArchiveIndex, build_index_from_cdn, parse_archive_index, resolve_ekey_bytes
from cdn import CdnClient, DEFAULT_BASE
from download_client import Progress, download_install_files, human, pick_version_row, write_build_info
from local_casc import LocalCascWriter
from tact import (
    EncodingTable,
    archive_keys_from_cdnconfig,
    parse_kv_config,
    parse_versions_table,
)


def place_config(dest_root: Path, key_hex: str, data: bytes) -> Path:
    key = key_hex.lower()
    path = dest_root / "config" / key[0:2] / key[2:4] / key
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_bytes(data)
    return path


def convert_archives(
    cdn: CdnClient,
    archive_keys: list[str],
    writer: LocalCascWriter,
    progress: Progress,
    *,
    keep_cdn_cache: bool,
) -> None:
    total = len(archive_keys)
    for i, key in enumerate(archive_keys, 1):
        print(f"[{i}/{total}] archive {key}")
        try:
            idx_path = cdn.download_key(key, kind="data", index=True, progress_cb=progress)
        except Exception as exc:  # noqa: BLE001
            print(f"  skip index: {exc}")
            continue

        parsed = parse_archive_index(idx_path.read_bytes(), key)
        # Index may store both 9-byte and padded 16-byte keys — keep unique EKey9
        locs = []
        seen9 = set()
        for ekey, loc in parsed.items():
            if len(ekey) not in (9, 16):
                continue
            e9 = ekey[:9]
            if e9 in seen9:
                continue
            seen9.add(e9)
            full = ekey if len(ekey) == 16 else ekey + b"\x00" * 7
            locs.append((full, loc))

        # Download full archive once (resume-safe), then slice locally
        try:
            arch_path = cdn.download_key(key, kind="data", index=False, progress_cb=progress)
        except Exception as exc:  # noqa: BLE001
            print(f"  skip archive: {exc}")
            continue

        written = 0
        skipped = 0
        arch_size = arch_path.stat().st_size
        with open(arch_path, "rb") as af:
            for full, loc in locs:
                if loc.offset < 0 or loc.size <= 0 or loc.offset + loc.size > arch_size:
                    skipped += 1
                    continue
                try:
                    af.seek(loc.offset)
                    blob = af.read(loc.size)
                except OSError as exc:
                    print(f"  read fail {full.hex()[:18]}: {exc}")
                    skipped += 1
                    continue
                if len(blob) != loc.size:
                    skipped += 1
                    continue
                if writer.write_blte(full, blob):
                    written += 1
                else:
                    skipped += 1

        print(f"  -> {written}/{len(locs)} records into local data.* (skipped {skipped})")
        if not keep_cdn_cache:
            try:
                arch_path.unlink(missing_ok=True)
            except OSError:
                pass


def main() -> int:
    ap = argparse.ArgumentParser(
        description="CDN → local CASC Data converter (data.NNN + .idx)"
    )
    ap.add_argument("--out", required=True, type=Path, help="Client folder (Wow.exe + Data)")
    ap.add_argument("--base", default=DEFAULT_BASE, help=f"CDN base (default {DEFAULT_BASE})")
    ap.add_argument("--region", default="us")
    ap.add_argument("--cache", type=Path, default=None, help="CDN download cache (default: temp)")
    ap.add_argument(
        "--keep-cache",
        action="store_true",
        help="Keep downloaded CDN archives after conversion",
    )
    ap.add_argument(
        "--with-install",
        action="store_true",
        help="Also extract Wow.exe / DLLs from install manifest",
    )
    ap.add_argument(
        "--tags",
        nargs="*",
        default=["Windows", "enUS", "x86_64"],
        help="Install tags (with --with-install)",
    )
    ap.add_argument("--patch-exe", action="store_true", help="Patch exe → zephyr after install")
    ap.add_argument("--build-config", default=None)
    ap.add_argument("--cdn-config", default=None)
    ap.add_argument(
        "--limit-archives",
        type=int,
        default=0,
        help="Convert only first N archives (debug/smoke)",
    )
    args = ap.parse_args()

    client_dir: Path = args.out
    client_dir.mkdir(parents=True, exist_ok=True)
    data_root = client_dir / "Data"
    local_data = data_root / "data"

    tmp_cache = None
    if args.cache:
        cache_dir = args.cache
        cache_dir.mkdir(parents=True, exist_ok=True)
    else:
        tmp_cache = Path(tempfile.mkdtemp(prefix="zephyr_casc_conv_"))
        cache_dir = tmp_cache

    print(f"==> Client : {client_dir}")
    print(f"==> Cache  : {cache_dir}")
    print(f"==> Local Data/data : {local_data}")

    # Fresh local data journals (do not mix with CDN junk)
    if local_data.exists():
        backup = data_root / f"_data_backup_{int(time.time())}"
        print(f"==> Moving old Data/data -> {backup.name}")
        local_data.rename(backup)

    local_data.mkdir(parents=True, exist_ok=True)
    (data_root / "config").mkdir(parents=True, exist_ok=True)
    (data_root / "patch").mkdir(parents=True, exist_ok=True)

    cdn = CdnClient(base_url=args.base, cache_dir=cache_dir)
    progress = Progress()

    print("==> versions / configs")
    versions_text = cdn.fetch_text("versions")
    rows = parse_versions_table(versions_text)
    row = pick_version_row(rows, args.region)
    if not row:
        print("No versions row", file=sys.stderr)
        return 1

    build_config = (args.build_config or row.get("BuildConfig", "")).lower()
    cdn_config = (args.cdn_config or row.get("CDNConfig", "")).lower()
    build_id = row.get("BuildId", "")
    version_name = row.get("VersionsName", "")
    print(f"Build {version_name} id={build_id}")
    print(f"BuildConfig={build_config}")
    print(f"CDNConfig={cdn_config}")

    build_path = cdn.download_key(build_config, kind="config", progress_cb=progress)
    cdncfg_path = cdn.download_key(cdn_config, kind="config", progress_cb=progress)
    place_config(data_root, build_config, build_path.read_bytes())
    place_config(data_root, cdn_config, cdncfg_path.read_bytes())

    build_cfg = parse_kv_config(build_path.read_text(encoding="utf-8", errors="replace"))
    cdn_cfg = parse_kv_config(cdncfg_path.read_text(encoding="utf-8", errors="replace"))
    archives = archive_keys_from_cdnconfig(cdn_cfg)
    if args.limit_archives and args.limit_archives > 0:
        archives = archives[: args.limit_archives]
        print(f"==> Limiting to {len(archives)} archives (smoke)")

    print(f"==> Converting {len(archives)} CDN archives -> local data.NNN / .idx")
    print("    This is the real full Data (tens of GB). Resume via --cache.")
    writer = LocalCascWriter(local_data)
    try:
        convert_archives(
            cdn,
            archives,
            writer,
            progress,
            keep_cdn_cache=args.keep_cache or bool(args.cache),
        )
    finally:
        writer.close()

    recs, nbytes, narch = writer.stats
    print(f"==> Wrote {recs} records, {human(nbytes)}, across {narch} data.* file(s)")

    write_build_info(client_dir, args.region, build_config, cdn_config, version_name, build_id)

    if args.with_install:
        encoding_keys = build_cfg.get("encoding", [])
        install_ckey = build_cfg.get("install", [None])[0]
        if len(encoding_keys) >= 2 and install_ckey:
            encoding_ekey = encoding_keys[1]
            print("==> Encoding (for install extract)")
            # Prefer already-converted local blob via CDN cache / loose
            archive_index = build_index_from_cdn(cdn, archives)
            enc_raw = resolve_ekey_bytes(
                cdn, bytes.fromhex(encoding_ekey), archive_index=archive_index, progress_cb=progress
            )
            encoding = EncodingTable.parse(enc_raw)
            install_ekey = encoding.get_ekey(install_ckey)
            if install_ekey:
                print("==> Install files")
                install_bytes = resolve_ekey_bytes(
                    cdn, install_ekey, archive_index=archive_index, progress_cb=progress
                )
                n = download_install_files(
                    cdn, encoding, install_bytes, client_dir, args.tags, progress, archive_index
                )
                print(f"Install files: {n}")
            if args.patch_exe:
                patcher = Path(__file__).with_name("patch_client.py")
                for name in ("Wow-64.exe", "Wow.exe"):
                    exe = client_dir / name
                    if exe.exists():
                        subprocess.run(
                            [
                                sys.executable,
                                str(patcher),
                                "--exe",
                                str(exe),
                                "--region",
                                args.region,
                            ],
                            check=False,
                        )

    if tmp_cache and tmp_cache.exists() and not args.keep_cache:
        print("==> Cleaning temp CDN cache")
        shutil.rmtree(tmp_cache, ignore_errors=True)

    print()
    print("Done.")
    print(f"  Data/data : {local_data}")
    print("  Open with CascLib / mapextractor against the client folder.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
