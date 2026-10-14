#!/usr/bin/env python3
"""
Serve a downloaded Zephyr CDN mirror as a local patch+CDN host.

Exposes:
  /wow/versions
  /wow/cdns          (Hosts rewritten to --host)
  /wow/config/...
  /wow/data/...
  /wow/patch/...

Example (admin / port 80 preferred for real Wow.exe):
  python serve_cdn.py --root D:\\WoD_Client --host local.wow.cdn --port 80

Add to hosts file:
  127.0.0.1  local.wow.cdn
"""
from __future__ import annotations

import argparse
import sys
from http.server import SimpleHTTPRequestHandler, ThreadingHTTPServer
from pathlib import Path
from urllib.parse import unquote


class CdnHandler(SimpleHTTPRequestHandler):
    root: Path
    public_host: str

    def translate_path(self, path: str) -> str:
        path = unquote(path.split("?", 1)[0])
        # Accept /wow/... or bare versions/cdns at root
        if path.startswith("/wow/"):
            rel = path[len("/wow/") :]
        elif path in ("/versions", "/cdns"):
            rel = path.lstrip("/")
        else:
            rel = path.lstrip("/")
        candidate = (self.root / "cdn" / "wow" / rel).resolve()
        root = (self.root / "cdn" / "wow").resolve()
        if not str(candidate).startswith(str(root)):
            return str(root / "__deny__")
        return str(candidate)

    def do_GET(self):  # noqa: N802
        path = unquote(self.path.split("?", 1)[0])
        if path.endswith("/cdns") or path == "/cdns" or path == "/wow/cdns":
            self._serve_cdns()
            return
        if path.endswith("/versions") or path == "/versions" or path == "/wow/versions":
            self._serve_file(self.root / "versions")
            return
        return super().do_GET()

    def _serve_cdns(self) -> None:
        src = self.root / "cdns"
        if not src.exists():
            src = self.root / "cdn" / "wow" / "cdns"
        if not src.exists():
            self.send_error(404, "cdns not found")
            return
        lines = src.read_text(encoding="utf-8", errors="replace").splitlines()
        out_lines = []
        for i, line in enumerate(lines):
            if i == 0 or not line.strip():
                out_lines.append(line)
                continue
            cols = line.split("|")
            # Name|Path|Hosts|Servers|ConfigPath
            if len(cols) >= 4:
                cols[2] = self.public_host
                cols[3] = f"http://{self.public_host}"
            out_lines.append("|".join(cols))
        body = ("\n".join(out_lines) + "\n").encode("utf-8")
        self.send_response(200)
        self.send_header("Content-Type", "text/plain")
        self.send_header("Content-Length", str(len(body)))
        self.end_headers()
        self.wfile.write(body)

    def _serve_file(self, path: Path) -> None:
        if not path.exists():
            self.send_error(404)
            return
        data = path.read_bytes()
        self.send_response(200)
        self.send_header("Content-Type", "text/plain")
        self.send_header("Content-Length", str(len(data)))
        self.end_headers()
        self.wfile.write(data)

    def log_message(self, fmt: str, *args) -> None:
        sys.stderr.write("%s - %s\n" % (self.address_string(), fmt % args))


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--root", required=True, type=Path, help="Output dir from download_client.py")
    ap.add_argument("--host", default="local.wow.cdn", help="Hostname written into cdns")
    ap.add_argument("--bind", default="0.0.0.0")
    ap.add_argument("--port", type=int, default=80)
    args = ap.parse_args()

    root = args.root.resolve()
    if not (root / "cdn" / "wow").exists():
        print(f"Missing CDN mirror at {root / 'cdn' / 'wow'}", file=sys.stderr)
        return 1

    CdnHandler.root = root
    CdnHandler.public_host = args.host
    httpd = ThreadingHTTPServer((args.bind, args.port), CdnHandler)
    print(f"Serving {root / 'cdn' / 'wow'} as http://{args.host}:{args.port}/wow/")
    print(f"versions: http://{args.host}:{args.port}/wow/versions")
    print(f"cdns:     http://{args.host}:{args.port}/wow/cdns")
    print("Add hosts entry: 127.0.0.1  " + args.host)
    print("Patch Wow.exe versions/cdns URLs to this host (port 80 recommended).")
    try:
        httpd.serve_forever()
    except KeyboardInterrupt:
        print("\nStopped.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
