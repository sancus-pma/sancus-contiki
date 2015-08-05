#!/usr/bin/env python3

import argparse
import socket
import struct


def vendor_id(arg):
    vid = int(arg)

    if 0 <= vid < 2**16:
        return vid

    raise argparse.ArgumentTypeError('The vendor ID should be a 16-bit integer')


def pack_int(i):
    return struct.pack('!H', i)


def unpack_int(buf):
    return struct.unpack('!H', buf)[0]


def recv_vendor_id(s):
    return unpack_int(s.recv(2))


def recv_symtab(s):
    symtab_data = bytearray()

    while True:
        symtab_data += s.recv(4096)

        if symtab_data[-1] == 0:
            return symtab_data.decode('ascii')


def create_data(file, name, vid):
    # The packet format is [LEN NAME \0 VID ELF_FILE]
    # LEN is the length of the packet without LEN itself
    file_data = file.read()
    # +3 is the NULL terminator of the name + 2 bytes of the VID
    length = len(file_data) + len(name) + 3

    return pack_int(length) + \
           name.encode('ascii') + b'\0' + \
           pack_int(vid) + \
           file_data


parser = argparse.ArgumentParser()
parser.add_argument('--server',
                    help='IP address/host name of the SM server',
                    required=True)
parser.add_argument('--port',
                    help='Port on which the SM server is listening',
                    type=int,
                    default=2000)
parser.add_argument('--sm-name',
                    help='Name of the SM',
                    required=True)
parser.add_argument('--vendor-id',
                    help='Vendor ID for the loaded SM',
                    type=vendor_id,
                    required=True)
parser.add_argument('file',
                    help='File containing the SM to load',
                    type=argparse.FileType('rb'))
args = parser.parse_args()

with socket.create_connection((args.server, args.port), timeout=2) as s:
    s.sendall(create_data(args.file, args.sm_name, args.vendor_id))
    sm_id = recv_vendor_id(s)
    print('SM "{}" loaded with id {}'.format(args.sm_name, sm_id))
    symtab = recv_symtab(s)
    print(symtab)
