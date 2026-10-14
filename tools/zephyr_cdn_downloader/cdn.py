"""HTTP helpers for Blizzard-style TACT/CDN layouts."""
from __future__ import annotations

import hashlib
import os
import time
import urllib.error
import urllib.request
from pathlib import Path
from typing import Optional


DEFAULT_BASE = "https://zephyr.v4y.hu/wow"
USER_AGENT = "ZephyrCDNDownloader/1.0 (Draenor-Core)"


def hash_path(key_hex: str, kind: str = "data") -> str:
    key = key_hex.lower()
    return f"{kind}/{key[0:2]}/{key[2:4]}/{key}"


class CdnClient:
    def __init__(self, base_url: str = DEFAULT_BASE, cache_dir: Path | None = None, retries: int = 5):
        self.base_url = base_url.rstrip("/")
        self.cache_dir = cache_dir
        self.retries = retries
        if self.cache_dir:
            self.cache_dir.mkdir(parents=True, exist_ok=True)

    def url_for(self, key_hex: str, kind: str = "data", index: bool = False) -> str:
        path = hash_path(key_hex, kind)
        if index:
            path += ".index"
        return f"{self.base_url}/{path}"

    def local_path(self, key_hex: str, kind: str = "data", index: bool = False) -> Path:
        assert self.cache_dir is not None
        path = hash_path(key_hex, kind)
        if index:
            path += ".index"
        return self.cache_dir / path

    def fetch_text(self, name: str) -> str:
        url = f"{self.base_url}/{name}"
        return self._request_bytes(url).decode("utf-8", errors="replace")

    def head_size(self, key_hex: str, kind: str = "data", index: bool = False) -> Optional[int]:
        """Return Content-Length for a CDN object, or None if unknown."""
        url = self.url_for(key_hex, kind, index=index)
        local = self.local_path(key_hex, kind, index=index) if self.cache_dir else None
        if local is not None and local.exists() and local.stat().st_size > 0:
            return local.stat().st_size
        try:
            req = urllib.request.Request(url, method="HEAD", headers={"User-Agent": USER_AGENT})
            with urllib.request.urlopen(req, timeout=20) as resp:
                cl = resp.headers.get("Content-Length")
                return int(cl) if cl else None
        except Exception:
            # Some mirrors dislike HEAD — probe with a 1-byte Range
            try:
                req = urllib.request.Request(
                    url,
                    headers={"User-Agent": USER_AGENT, "Range": "bytes=0-0"},
                )
                with urllib.request.urlopen(req, timeout=20) as resp:
                    cr = resp.headers.get("Content-Range", "")
                    if "/" in cr:
                        return int(cr.rsplit("/", 1)[-1])
                    cl = resp.headers.get("Content-Length")
                    return int(cl) if cl else None
            except Exception:
                return None

    def download_key(
        self,
        key_hex: str,
        kind: str = "data",
        index: bool = False,
        expected_size: Optional[int] = None,
        progress_cb=None,
    ) -> Path:
        assert self.cache_dir is not None
        dest = self.local_path(key_hex, kind, index=index)
        dest.parent.mkdir(parents=True, exist_ok=True)
        url = self.url_for(key_hex, kind, index=index)

        if dest.exists() and dest.stat().st_size > 0:
            if expected_size is None or dest.stat().st_size == expected_size:
                if progress_cb:
                    progress_cb(dest.stat().st_size, dest.stat().st_size, key_hex)
                return dest

        tmp = dest.with_suffix(dest.suffix + ".partial")
        existing = tmp.stat().st_size if tmp.exists() else 0
        headers = {"User-Agent": USER_AGENT}
        if existing > 0:
            headers["Range"] = f"bytes={existing}-"

        last_err: Exception | None = None
        for attempt in range(1, self.retries + 1):
            try:
                req = urllib.request.Request(url, headers=headers)
                with urllib.request.urlopen(req, timeout=120) as resp:
                    mode = "ab" if existing and resp.status == 206 else "wb"
                    if mode == "wb":
                        existing = 0
                    total = resp.length
                    if total is not None and "Content-Range" not in resp.headers:
                        total_size = total
                    elif "Content-Range" in resp.headers:
                        # bytes start-end/total
                        total_size = int(resp.headers["Content-Range"].split("/")[-1])
                    else:
                        total_size = expected_size

                    written = existing
                    with open(tmp, mode) as out:
                        while True:
                            chunk = resp.read(1024 * 1024)
                            if not chunk:
                                break
                            out.write(chunk)
                            written += len(chunk)
                            if progress_cb:
                                progress_cb(written, total_size or written, key_hex)

                if expected_size is not None and tmp.stat().st_size != expected_size:
                    raise IOError(
                        f"size mismatch for {key_hex}: got {tmp.stat().st_size}, expected {expected_size}"
                    )
                os.replace(tmp, dest)
                return dest
            except Exception as exc:  # noqa: BLE001 - retry network/CDN glitches
                last_err = exc
                # Complete-file partial + Range → 416 on some mirrors: treat as done or restart
                err_s = str(exc)
                if "416" in err_s and tmp.exists() and tmp.stat().st_size > 0:
                    if expected_size is None or tmp.stat().st_size == expected_size:
                        os.replace(tmp, dest)
                        if progress_cb:
                            progress_cb(dest.stat().st_size, dest.stat().st_size, key_hex)
                        return dest
                    try:
                        tmp.unlink()
                    except OSError:
                        pass
                    existing = 0
                    headers = {"User-Agent": USER_AGENT}
                    time.sleep(min(2 ** attempt, 15))
                    continue
                time.sleep(min(2 ** attempt, 15))
                existing = tmp.stat().st_size if tmp.exists() else 0
                headers = {"User-Agent": USER_AGENT}
                if existing > 0:
                    headers["Range"] = f"bytes={existing}-"

        raise RuntimeError(f"failed to download {url}: {last_err}")

    def _request_bytes(self, url: str) -> bytes:
        last_err: Exception | None = None
        for attempt in range(1, self.retries + 1):
            try:
                req = urllib.request.Request(url, headers={"User-Agent": USER_AGENT})
                with urllib.request.urlopen(req, timeout=60) as resp:
                    return resp.read()
            except Exception as exc:  # noqa: BLE001
                last_err = exc
                time.sleep(min(2 ** attempt, 10))
        raise RuntimeError(f"failed to GET {url}: {last_err}")


def md5_hex(data: bytes) -> str:
    return hashlib.md5(data).hexdigest()
