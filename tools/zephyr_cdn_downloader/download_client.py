#!/usr/bin/env python3
"""
Download a FULL WoW 6.2.4 client from https://zephyr.v4y.hu/wow/

Result (flat client folder — same place people keep the base):
  out/Wow-64.exe (+ DLLs)
  out/Data/config|data|patch

--install-only: only exe/DLLs (temp cache, then deleted).
"""
from __future__ import annotations

import argparse
import os
import shutil
import subprocess
import sys
import tempfile
import time
from pathlib import Path

from archive import ArchiveIndex, build_index_from_cdn, resolve_ekey_bytes
from blte import decode_blte
from cdn import CdnClient, DEFAULT_BASE
from tact import (
    EncodingTable,
    archive_keys_from_cdnconfig,
    parse_install,
    parse_kv_config,
    parse_versions_table,
)


def human(n: float) -> str:
    units = ["B", "KB", "MB", "GB", "TB"]
    i = 0
    while n >= 1024 and i < len(units) - 1:
        n /= 1024
        i += 1
    return f"{n:.2f} {units[i]}"


class Progress:
    def __init__(self):
        self._last = 0.0

    def __call__(self, written: int, total: int, name: str) -> None:
        now = time.time()
        if now - self._last < 0.2 and written != total:
            return
        self._last = now
        short = name[:12]
        if total:
            pct = 100.0 * written / total
            print(f"\r  [{short}] {human(written)} / {human(total)} ({pct:5.1f}%)", end="", flush=True)
        else:
            print(f"\r  [{short}] {human(written)}", end="", flush=True)
        if written == total:
            print()


def write_text(path: Path, text: str) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(text, encoding="utf-8", newline="\n")


def pick_version_row(rows, region: str):
    for row in rows:
        if row.get("Region", "").lower() == region.lower():
            return row
    return rows[0] if rows else None


def download_archives(cdn: CdnClient, archive_keys: list[str], progress: Progress) -> None:
    total = len(archive_keys)
    done_bytes = 0
    for i, key in enumerate(archive_keys, 1):
        local = cdn.local_path(key, kind="data")
        skipped = local.exists() and local.stat().st_size > 0
        print(f"[{i}/{total}] archive {key}" + (" (cached)" if skipped else ""))
        path = cdn.download_key(key, kind="data", index=False, progress_cb=progress)
        done_bytes += path.stat().st_size
        try:
            cdn.download_key(key, kind="data", index=True, progress_cb=progress)
        except Exception as exc:  # noqa: BLE001
            print(f"  warn: index for {key}: {exc}")
        if i % 10 == 0 or i == total:
            print(f"  … mirrored so far: {human(done_bytes)} in {i}/{total} archives")


def link_or_junction(link: Path, target: Path) -> str:
    """Create directory junction (Windows) or symlink. Returns method used."""
    target = target.resolve()
    if link.exists() or link.is_symlink() or link.is_junction():
        return "exists"
    link.parent.mkdir(parents=True, exist_ok=True)
    if os.name == "nt":
        # Junctions do not require admin (unlike symlinks on some setups)
        r = subprocess.run(
            ["cmd", "/c", "mklink", "/J", str(link), str(target)],
            capture_output=True,
            text=True,
        )
        if r.returncode == 0:
            return "junction"
        try:
            link.symlink_to(target, target_is_directory=True)
            return "symlink"
        except OSError as exc:
            raise RuntimeError(f"cannot link {link} -> {target}: {r.stderr or exc}") from exc
    link.symlink_to(target, target_is_directory=True)
    return "symlink"


def setup_client_data(client_dir: Path, cdn_root: Path) -> None:
    """
    Wire client/Data to the CDN mirror so the game sees a full local Data tree:
      Data/config, Data/data, Data/patch
    """
    data_dir = client_dir / "Data"
    data_dir.mkdir(parents=True, exist_ok=True)
    mapping = {
        "config": cdn_root / "config",
        "data": cdn_root / "data",
        "patch": cdn_root / "patch",
    }
    for name, target in mapping.items():
        target.mkdir(parents=True, exist_ok=True)
        dest = data_dir / name
        try:
            how = link_or_junction(dest, target)
            print(f"  Data/{name} -> {target} ({how})")
        except Exception as exc:  # noqa: BLE001
            print(f"  warn: Data/{name}: {exc}")
            write_text(
                data_dir / f"{name}_PATH.txt",
                f"Place or junction this folder to:\n{target}\n",
            )


def download_install_files(
    cdn: CdnClient,
    encoding: EncodingTable,
    install_bytes: bytes,
    client_dir: Path,
    tags: list[str],
    progress: Progress,
    archive_index: ArchiveIndex | None,
) -> int:
    manifest = parse_install(install_bytes)
    print(f"Install tags available ({len(manifest.tags)}): {', '.join(sorted(manifest.tags)[:40])}...")
    entries = manifest.filter_tags(tags) if tags else list(manifest.entries)
    if tags and not entries:
        print(f"No entries matched tags {tags}; falling back to all entries.", file=sys.stderr)
        entries = list(manifest.entries)

    print(f"Installing {len(entries)} files into {client_dir}")
    client_dir.mkdir(parents=True, exist_ok=True)
    ok = 0
    for idx, entry in enumerate(entries, 1):
        ekey = encoding.get_ekey(entry.ckey)
        if not ekey:
            print(f"  skip (no ekey): {entry.path}")
            continue
        print(f"[{idx}/{len(entries)}] {entry.path} ({human(entry.size)})")
        try:
            raw = resolve_ekey_bytes(cdn, ekey, archive_index=archive_index, progress_cb=progress)
        except Exception as exc:  # noqa: BLE001
            print(f"  download failed: {exc}")
            continue
        try:
            payload = decode_blte(raw) if raw.startswith(b"BLTE") else raw
        except Exception as exc:  # noqa: BLE001
            print(f"  BLTE decode failed for {entry.path}: {exc}")
            continue
        out = client_dir / entry.path.replace("\\", "/")
        out.parent.mkdir(parents=True, exist_ok=True)
        out.write_bytes(payload)
        ok += 1
    return ok


def write_build_info(
    client_dir: Path,
    region: str,
    build_config: str,
    cdn_config: str,
    version_name: str,
    build_id: str,
    *,
    cdn_hosts: str = "zephyr.v4y.hu",
    cdn_path: str = "wow",
) -> None:
    # CascLib requires Build Key + CDN Key + CDN Hosts + CDN Path (else BAD_FORMAT)
    header = (
        "Branch!STRING:0|Active!DEC:1|Build Key!HEX:16|CDN Key!HEX:16|"
        "Region!STRING:0|CDN Path!STRING:0|CDN Hosts!STRING:0|"
        "Version!STRING:0|KeyRing!HEX:16|BuildId!DEC:4|VersionName!String:0"
    )
    row = (
        f"wow|1|{build_config}|{cdn_config}|{region}|{cdn_path}|{cdn_hosts}||"
        f"{build_config}|{build_id}|{version_name}"
    )
    write_text(client_dir / ".build.info", header + "\n" + row + "\n")


def main() -> int:
    ap = argparse.ArgumentParser(
        description="Download FULL WoW 6.2.4 client (exe + Data) from Zephyr CDN"
    )
    ap.add_argument("--out", required=True, type=Path, help="Output directory")
    ap.add_argument("--base", default=DEFAULT_BASE, help=f"CDN base URL (default: {DEFAULT_BASE})")
    ap.add_argument("--region", default="us", help="Region row from versions (eu/us/...)")
    ap.add_argument("--tags", nargs="*", default=["Windows", "enUS", "x86_64"], help="Install tags filter")
    ap.add_argument(
        "--install-only",
        action="store_true",
        help="Minimal client only (Wow.exe/DLLs). Skips Data archives (~tens of GB).",
    )
    ap.add_argument(
        "--archives-only",
        action="store_true",
        help="Only download Data archives into client/Data (no exe extract).",
    )
    ap.add_argument(
        "--patch-exe",
        action="store_true",
        help="After download, patch Wow-64.exe/Wow.exe to use zephyr versions/cdns.",
    )
    ap.add_argument("--build-config", default=None, help="Override BuildConfig hash")
    ap.add_argument("--cdn-config", default=None, help="Override CDNConfig hash")
    args = ap.parse_args()

    out: Path = args.out
    # Flat layout: out/ IS the client folder (Wow.exe + Data next to each other)
    client_dir = out
    client_dir.mkdir(parents=True, exist_ok=True)

    # Full client: download straight into Data/ (normal game layout).
    # install-only: use TEMP so we never leave a local CDN tree for the user.
    tmp_cache: Path | None = None
    if args.install_only:
        tmp_cache = Path(tempfile.mkdtemp(prefix="zephyr_wow_dl_"))
        cdn_root = tmp_cache
        print(f"==> Temp cache: {cdn_root}")
    else:
        cdn_root = client_dir / "Data"
        cdn_root.mkdir(parents=True, exist_ok=True)
        print(f"==> Data folder: {cdn_root}")

    cdn = CdnClient(base_url=args.base, cache_dir=cdn_root)
    progress = Progress()

    print("==> Fetching versions / cdns from Zephyr")
    versions_text = cdn.fetch_text("versions")
    _ = cdn.fetch_text("cdns")  # validate CDN is reachable; client will re-fetch after patch

    rows = parse_versions_table(versions_text)
    row = pick_version_row(rows, args.region)
    if not row:
        print("No versions rows found", file=sys.stderr)
        return 1

    build_config = (args.build_config or row.get("BuildConfig", "")).lower()
    cdn_config = (args.cdn_config or row.get("CDNConfig", "")).lower()
    build_id = row.get("BuildId", "")
    version_name = row.get("VersionsName", "")
    print(f"Build {version_name} (id {build_id})")
    print(f"BuildConfig={build_config}")
    print(f"CDNConfig={cdn_config}")

    print("==> Build / CDN configs")
    build_path = cdn.download_key(build_config, kind="config", progress_cb=progress)
    cdncfg_path = cdn.download_key(cdn_config, kind="config", progress_cb=progress)
    build_cfg = parse_kv_config(build_path.read_text(encoding="utf-8", errors="replace"))
    cdn_cfg = parse_kv_config(cdncfg_path.read_text(encoding="utf-8", errors="replace"))

    encoding_keys = build_cfg.get("encoding", [])
    if len(encoding_keys) < 2:
        print("Build config missing encoding ckey/ekey", file=sys.stderr)
        return 1
    encoding_ckey, encoding_ekey = encoding_keys[0], encoding_keys[1]
    install_ckey = build_cfg.get("install", [None])[0]

    print("==> Encoding table")
    enc_path = cdn.download_key(encoding_ekey, kind="data", progress_cb=progress)
    encoding = EncodingTable.parse(enc_path.read_bytes())
    print(f"Encoding entries: {len(encoding.ckey_to_ekeys)}")

    archives = archive_keys_from_cdnconfig(cdn_cfg)
    patch_archives = list(cdn_cfg.get("patch-archives", []))
    archive_index: ArchiveIndex | None = None

    if not args.install_only:
        print(
            f"==> Downloading FULL Data from Zephyr: {len(archives)} archives"
            + (f" + {len(patch_archives)} patch-archives" if patch_archives else "")
        )
        print("    Into Data\\  (often 40–80+ GB). Resume-safe.")
        download_archives(cdn, archives, progress)
        if patch_archives:
            print(f"==> Patch archives ({len(patch_archives)})")
            download_archives(cdn, patch_archives, progress)
        archive_index = ArchiveIndex()
        for key in archives:
            ipath = cdn.local_path(key, kind="data", index=True)
            if ipath.exists():
                archive_index.add_index(key, ipath.read_bytes())
    else:
        print(f"==> Archive indices (to extract Wow.exe only)")
        archive_index = build_index_from_cdn(cdn, archives)

    if not args.archives_only:
        if not install_ckey:
            print("No install key in build config", file=sys.stderr)
            return 1
        install_ekey = encoding.get_ekey(install_ckey)
        if not install_ekey:
            print(f"Install ckey {install_ckey} not in encoding", file=sys.stderr)
            return 1
        print("==> Install manifest (exe / DLLs)")
        try:
            install_bytes = resolve_ekey_bytes(
                cdn, install_ekey, archive_index=archive_index, progress_cb=progress
            )
        except Exception:
            inst_path = cdn.download_key(install_ekey.hex(), kind="data", progress_cb=progress)
            install_bytes = inst_path.read_bytes()
        count = download_install_files(
            cdn, encoding, install_bytes, client_dir, args.tags, progress, archive_index
        )
        write_build_info(client_dir, args.region, build_config, cdn_config, version_name, build_id)
        print(f"Install files written: {count}")

    # Always patch for Zephyr when requested (or by default for full client via bat)
    if args.patch_exe and not args.archives_only:
        print("==> Patching exe → zephyr.v4y.hu")
        patcher = Path(__file__).with_name("patch_client.py")
        for name in ("Wow-64.exe", "Wow.exe"):
            exe = client_dir / name
            if exe.exists():
                subprocess.run(
                    [sys.executable, str(patcher), "--exe", str(exe), "--region", args.region],
                    check=False,
                )

    # Never leave a top-level cdn/ folder for the user
    legacy_cdn = out / "cdn"
    if legacy_cdn.exists():
        print(f"==> Removing leftover folder: {legacy_cdn}")
        shutil.rmtree(legacy_cdn, ignore_errors=True)
    for name in ("versions", "cdns"):
        p = out / name
        if p.exists() and p.is_file():
            p.unlink(missing_ok=True)
    if tmp_cache and tmp_cache.exists():
        print("==> Cleaning temp cache")
        shutil.rmtree(tmp_cache, ignore_errors=True)

    print()
    print("Done.")
    print(f"  Client : {client_dir}")
    if args.install_only:
        print("  Data   : will be filled by Wow.exe from Zephyr on first run")
    else:
        mirrored = 0
        data_dir = cdn_root / "data"
        if data_dir.exists():
            mirrored = sum(
                p.stat().st_size
                for p in data_dir.rglob("*")
                if p.is_file() and not p.name.endswith(".index")
            )
        print(f"  Data   : {cdn_root} ({human(mirrored)})")
        print("  Run    :", (client_dir / "Wow-64.exe").resolve())
    return 0


if __name__ == "__main__":
    sys.exit(main())
