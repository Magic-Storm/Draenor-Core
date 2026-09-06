#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Parse Draenor-Core PacketLog binary dumps (worldserver.conf: PacketLogFile).

Record format (see src/server/game/Server/Protocol/PacketLog.cpp):
    int32 opcode, int32 size, uint32 unixtime, uint8 direction
    direction: 0 = CLIENT_TO_SERVER, 1 = SERVER_TO_CLIENT
    followed by `size` payload bytes

Usage:
    python parse_packet_log.py <packet.log> [tail] [--all] [--hex]

    tail    how many last records to print (default 60)
    --all   print every record
    --hex   also dump first 64 bytes of payload for the printed records

Opcode names are resolved from src/server/game/Server/Protocol/Opcodes.h
"""

import os
import re
import struct
import sys
import time

OPCODES_H = os.path.normpath(os.path.join(os.path.dirname(os.path.abspath(__file__)),
                                          '..', 'src', 'server', 'game', 'Server', 'Protocol', 'Opcodes.h'))

RECORD = struct.Struct('<iiIB')  # opcode, size, unixtime, direction


def load_opcode_names():
    c2s, s2c = {}, {}
    try:
        with open(OPCODES_H, 'r', encoding='utf-8', errors='ignore') as fh:
            for line in fh:
                m = re.match(r'\s*((?:C|S)MSG_[A-Z0-9_]+)\s*=\s*(0x[0-9A-Fa-f]+|\d+)', line)
                if m:
                    value = int(m.group(2), 0)
                    name = m.group(1)
                    if name.startswith('CMSG_'):
                        c2s.setdefault(value, name)
                    else:
                        s2c.setdefault(value, name)
    except OSError as ex:
        print('WARNING: cannot read %s: %s' % (OPCODES_H, ex))
    return c2s, s2c


def hexdump(payload, limit=64):
    chunk = payload[:limit]
    return ' '.join('%02X' % b for b in chunk)


def main():
    args = [a for a in sys.argv[1:] if not a.startswith('--')]
    flags = [a for a in sys.argv[1:] if a.startswith('--')]

    if not args:
        print(__doc__)
        return 1

    path = args[0]
    tail = int(args[1]) if len(args) > 1 else 60
    show_all = '--all' in flags
    show_hex = '--hex' in flags

    c2s, s2c = load_opcode_names()

    with open(path, 'rb') as fh:
        data = fh.read()

    entries = []
    offset = 0
    index = 0
    truncated = False

    while offset + RECORD.size <= len(data):
        opcode, size, tstamp, direction = RECORD.unpack_from(data, offset)
        offset += RECORD.size
        if size < 0 or offset + size > len(data):
            truncated = True
            break
        payload = data[offset:offset + size]
        offset += size
        table = c2s if direction == 0 else s2c
        name = table.get(opcode, 'UNKNOWN')
        entries.append((index, direction, opcode, size, tstamp, name, payload))
        index += 1

    print('records: %d   bytes consumed: %d/%d%s' % (
        len(entries), offset, len(data),
        '   [TRUNCATED TAIL - last record incomplete]' if truncated else ''))

    shown = entries if show_all else entries[-tail:]
    for (i, direction, opcode, size, tstamp, name, payload) in shown:
        dirn = 'C->S' if direction == 0 else 'S->C'
        try:
            tstr = time.strftime('%H:%M:%S', time.localtime(tstamp)) if tstamp else '--:--:--'
        except (ValueError, OverflowError):
            tstr = '--:--:--'
        line = '[%5d] %s %-4s %-42s 0x%04X (%5d) size=%d' % (i, tstr, dirn, name, opcode, opcode, size)
        print(line)
        if show_hex and payload:
            print('         %s' % hexdump(payload))

    return 0


if __name__ == '__main__':
    sys.exit(main())
