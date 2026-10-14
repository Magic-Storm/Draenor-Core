"""Bob Jenkins lookup3 (hashlittle / hashlittle2) — public domain algorithm."""
from __future__ import annotations

import struct
from typing import Tuple


def _rot(x: int, k: int) -> int:
    x &= 0xFFFFFFFF
    return ((x << k) | (x >> (32 - k))) & 0xFFFFFFFF


def _mix(a: int, b: int, c: int) -> Tuple[int, int, int]:
    a = (a - c) & 0xFFFFFFFF
    a ^= _rot(c, 4)
    c = (c + b) & 0xFFFFFFFF
    b = (b - a) & 0xFFFFFFFF
    b ^= _rot(a, 6)
    a = (a + c) & 0xFFFFFFFF
    c = (c - b) & 0xFFFFFFFF
    c ^= _rot(b, 8)
    b = (b + a) & 0xFFFFFFFF
    a = (a - c) & 0xFFFFFFFF
    a ^= _rot(c, 16)
    c = (c + b) & 0xFFFFFFFF
    b = (b - a) & 0xFFFFFFFF
    b ^= _rot(a, 19)
    a = (a + c) & 0xFFFFFFFF
    c = (c - b) & 0xFFFFFFFF
    c ^= _rot(b, 4)
    b = (b + a) & 0xFFFFFFFF
    return a, b, c


def _final(a: int, b: int, c: int) -> Tuple[int, int, int]:
    c ^= b
    c = (c - _rot(b, 14)) & 0xFFFFFFFF
    a ^= c
    a = (a - _rot(c, 11)) & 0xFFFFFFFF
    b ^= a
    b = (b - _rot(a, 25)) & 0xFFFFFFFF
    c ^= b
    c = (c - _rot(b, 16)) & 0xFFFFFFFF
    a ^= c
    a = (a - _rot(c, 4)) & 0xFFFFFFFF
    b ^= a
    b = (b - _rot(a, 14)) & 0xFFFFFFFF
    c ^= b
    c = (c - _rot(b, 24)) & 0xFFFFFFFF
    return a, b, c


def _hash_bytes(key: bytes, length: int, pc: int, pb: int) -> Tuple[int, int]:
    """Core hashlittle2; returns (pc, pb)."""
    a = b = c = (0xDEADBEEF + length + (pc & 0xFFFFFFFF)) & 0xFFFFFFFF
    c = (c + (pb & 0xFFFFFFFF)) & 0xFFFFFFFF

    # Byte-at-a-time path (always correct; avoids alignment tricks).
    k = key
    off = 0
    while length > 12:
        a = (a + k[off] + (k[off + 1] << 8) + (k[off + 2] << 16) + (k[off + 3] << 24)) & 0xFFFFFFFF
        b = (b + k[off + 4] + (k[off + 5] << 8) + (k[off + 6] << 16) + (k[off + 7] << 24)) & 0xFFFFFFFF
        c = (c + k[off + 8] + (k[off + 9] << 8) + (k[off + 10] << 16) + (k[off + 11] << 24)) & 0xFFFFFFFF
        a, b, c = _mix(a, b, c)
        length -= 12
        off += 12

    if length == 0:
        return c, b

    # Fall-through switch (no break) matching lookup3.c
    if length >= 12:
        c = (c + (k[off + 11] << 24)) & 0xFFFFFFFF
    if length >= 11:
        c = (c + (k[off + 10] << 16)) & 0xFFFFFFFF
    if length >= 10:
        c = (c + (k[off + 9] << 8)) & 0xFFFFFFFF
    if length >= 9:
        c = (c + k[off + 8]) & 0xFFFFFFFF
    if length >= 8:
        b = (b + (k[off + 7] << 24)) & 0xFFFFFFFF
    if length >= 7:
        b = (b + (k[off + 6] << 16)) & 0xFFFFFFFF
    if length >= 6:
        b = (b + (k[off + 5] << 8)) & 0xFFFFFFFF
    if length >= 5:
        b = (b + k[off + 4]) & 0xFFFFFFFF
    if length >= 4:
        a = (a + (k[off + 3] << 24)) & 0xFFFFFFFF
    if length >= 3:
        a = (a + (k[off + 2] << 16)) & 0xFFFFFFFF
    if length >= 2:
        a = (a + (k[off + 1] << 8)) & 0xFFFFFFFF
    if length >= 1:
        a = (a + k[off]) & 0xFFFFFFFF

    a, b, c = _final(a, b, c)
    return c, b


def hashlittle(key: bytes, initval: int = 0) -> int:
    pc, _ = _hash_bytes(key, len(key), initval & 0xFFFFFFFF, 0)
    return pc


def hashlittle2(key: bytes, pc: int = 0, pb: int = 0) -> Tuple[int, int]:
    """Returns (pc, pb) — CascLib / wowdev use pc as the stored hash."""
    return _hash_bytes(key, len(key), pc & 0xFFFFFFFF, pb & 0xFFFFFFFF)
