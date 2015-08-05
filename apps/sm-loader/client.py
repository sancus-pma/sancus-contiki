#!/usr/bin/env python3

import argparse
import socket
import struct
import tempfile
import subprocess
import binascii
import os

import sancus.crypto


def vendor_id(arg):
    try:
        vid = int(arg, 16)
    except ValueError:
        raise argparse.ArgumentTypeError('The vendor ID should be a '
                                         'hexadecimal integer')

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
            return symtab_data[:-1].decode('ascii')


def write_symtab(symtab):
    fd, name = tempfile.mkstemp('.ld')

    with open(fd, 'w') as f:
        f.write(symtab)

    return name


def link_sm(sm_file, symtab_file):
    linked_file = tempfile.mkstemp('.elf')[1]
    subprocess.check_output(['msp430-ld', '-T', symtab_file,
                             '-o', linked_file, sm_file])
    return linked_file


def create_data(sm_file, name, vid):
    # The packet format is [LEN NAME \0 VID ELF_FILE]
    # LEN is the length of the packet without LEN itself
    with open(sm_file, 'rb') as f:
        file_data = f.read()

    # +3 is the NULL terminator of the name + 2 bytes of the VID
    length = len(file_data) + len(name) + 3

    return pack_int(length) + \
           name.encode('ascii') + b'\0' + \
           pack_int(vid) + \
           file_data


def to_hex_str(b):
    return binascii.hexlify(b).decode('ascii')


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
parser.add_argument('--vendor-key',
                    help='Key for the given vendor ID',
                    type=bytes.fromhex,
                    required=True)
parser.add_argument('file',
                    help='File containing the SM to load')
args = parser.parse_args()

with socket.create_connection((args.server, args.port), timeout=2) as s:
    s.sendall(create_data(args.file, args.sm_name, args.vendor_id))
    sm_id = recv_vendor_id(s)
    symtab_file = write_symtab(recv_symtab(s))
    linked_sm_file = link_sm(args.file, symtab_file)

    with open(linked_sm_file, 'rb') as f:
        sm_key = sancus.crypto.get_sm_key(f, args.sm_name, args.vendor_key)

    for file in (symtab_file, linked_sm_file):
        os.remove(file)

    print('SM "{}" loaded with id {} and key {}'
            .format(args.sm_name, sm_id, to_hex_str(sm_key)))
