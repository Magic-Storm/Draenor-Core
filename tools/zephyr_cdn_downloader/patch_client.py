#!/usr/bin/env python3
"""
Patch WoD 6.2.4 Wow*.exe so CAS fetches versions/cdns from Zephyr CDN.

Fixes:
  The CAS system was unable to initialize: ... VersionInfo: us. CDNInfo: us

The stock client asks Blizzard:
  http://%s.patch.battle.net:1119/%s/versions
  http://%s.patch.battle.net:1119/%s/cdns

We rewrite those (same length) to hit Zephyr, keeping the two %s so sprintf still works.
Query string is ignored by the static CDN:

  http://zephyr.v4y.hu/wow/versions?x=%s&%s
  http://zephyr.v4y.hu/wow/cdns?xx=%s&%s
"""
from __future__ import annotations

import argparse
import shutil
import sys
from pathlib import Path


VERSIONS_OLD = b"http://%s.patch.battle.net:1119/%s/versions"
CDNS_OLD = b"http://%s.patch.battle.net:1119/%s/cdns"

# Same lengths as originals (43 / 39). %s kept for client sprintf.
VERSIONS_NEW = b"http://zephyr.v4y.hu/wow/versions?a=%s&b=%s"
CDNS_NEW = b"http://zephyr.v4y.hu/wow/cdns?a=%s&b=%s"

# Also patch the substring form used by some tools / partial matches
VERSIONS_OLD_SHORT = b"%s.patch.battle.net:1119/%s/versions"
CDNS_OLD_SHORT = b"%s.patch.battle.net:1119/%s/cdns"


def _fit(old: bytes, new: bytes) -> bytes:
    if len(new) > len(old):
        raise ValueError(f"replacement longer than original: {len(new)} > {len(old)} ({new!r})")
    return new + (b"\x00" * (len(old) - len(new)))


def patch_bytes(data: bytes) -> tuple[bytes, list[str]]:
    assert len(VERSIONS_NEW) == len(VERSIONS_OLD), (len(VERSIONS_NEW), len(VERSIONS_OLD))
    assert len(CDNS_NEW) == len(CDNS_OLD), (len(CDNS_NEW), len(CDNS_OLD))

    log: list[str] = []
    out = data

    for label, old, new in (
        ("versions", VERSIONS_OLD, VERSIONS_NEW),
        ("cdns", CDNS_OLD, CDNS_NEW),
    ):
        count = out.count(old)
        if count == 0:
            log.append(f"WARNING: {label} URL not found (already patched?)")
            continue
        out = out.replace(old, new)
        log.append(f"patched {label}: {count} occurrence(s)")

    return out, log


def write_build_info(client_dir: Path, region: str = "us") -> None:
    build_config = "451679f16b266632976beb8d5c922d9f"
    cdn_config = "eee453d9035886d03a1308de041de88c"
    version = "6.2.4.21742"
    # CascLib requires CDN Hosts + CDN Path or CascOpenStorage -> BAD_FORMAT
    header = (
        "Branch!STRING:0|Active!DEC:1|Build Key!HEX:16|CDN Key!HEX:16|"
        "Region!STRING:0|CDN Path!STRING:0|CDN Hosts!STRING:0|"
        "Version!STRING:0|KeyRing!HEX:16|BuildId!DEC:4|VersionName!String:0"
    )
    row = (
        f"wow|1|{build_config}|{cdn_config}|{region}|wow|zephyr.v4y.hu||"
        f"{build_config}|21742|{version}"
    )
    (client_dir / ".build.info").write_text(header + "\n" + row + "\n", encoding="utf-8", newline="\n")


def main() -> int:
    ap = argparse.ArgumentParser(description="Point WoD client CAS at zephyr.v4y.hu")
    ap.add_argument("--exe", type=Path, required=True, help="Path to Wow.exe / Wow-64.exe")
    ap.add_argument("-o", "--output", type=Path, default=None, help="Output exe (default: overwrite --exe, or *-zephyr.exe if locked)")
    ap.add_argument("--region", default="us", help="Region written into .build.info")
    ap.add_argument("--no-backup", action="store_true")
    args = ap.parse_args()

    exe: Path = args.exe
    if not exe.exists():
        print(f"Not found: {exe}", file=sys.stderr)
        return 1

    data = exe.read_bytes()
    out_path = args.output
    if VERSIONS_NEW in data and CDNS_NEW in data:
        print("Already patched for Zephyr.")
        patched = data
        if out_path is None:
            out_path = exe
    else:
        patched, log = patch_bytes(data)
        for line in log:
            print(line)
        if patched == data and VERSIONS_OLD not in data:
            print("Could not find Blizzard versions/cdns URLs to patch.", file=sys.stderr)
            return 1
        if out_path is None:
            out_path = exe
        if out_path.resolve() == exe.resolve() and not args.no_backup:
            bak = exe.with_suffix(exe.suffix + ".bak")
            if not bak.exists():
                shutil.copy2(exe, bak)
                print(f"Backup: {bak}")

    try:
        out_path.write_bytes(patched)
    except PermissionError:
        alt = exe.with_name(exe.stem + "-zephyr" + exe.suffix)
        alt.write_bytes(patched)
        out_path = alt
        print(f"Original locked; wrote {out_path}")
    else:
        print(f"Wrote {out_path}")

    write_build_info(exe.parent, region=args.region)
    print(f"Updated {exe.parent / '.build.info'} (region={args.region})")
    print()
    print("CAS will fetch:")
    print("  ", VERSIONS_NEW.decode())
    print("  ", CDNS_NEW.decode())
    print("CDN data host comes from cdns file (zephyr.v4y.hu / path wow).")
    print(f"Run: {out_path}")
    print("First launch downloads Data/ from the CDN (large).")
    return 0


if __name__ == "__main__":
    # length self-check
    assert len(VERSIONS_NEW) == len(VERSIONS_OLD)
    assert len(CDNS_NEW) == len(CDNS_OLD)
    sys.exit(main())
