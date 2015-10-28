#!/usr/bin/env python3

import argparse
import ipaddress

MAGIC_SEQUENCE = b'\xf5\xd7\xc5\xbd'

parser = argparse.ArgumentParser()
parser.add_argument('input')
parser.add_argument('-o',
                    required=True,
                    dest='output')
parser.add_argument('--ip',
                    required=True,
                    type=ipaddress.IPv4Address)
args = parser.parse_args()

with open(args.input, 'rb') as f:
    contents = bytearray(f.read())

magic_begin = contents.find(MAGIC_SEQUENCE)
magic_end = magic_begin + len(MAGIC_SEQUENCE)

assert magic_begin != -1, 'Magic sequence not found in input file'
assert MAGIC_SEQUENCE not in contents[magic_end:], \
       'Magic sequence found multiple times in input'

contents[magic_begin:magic_end] = args.ip.packed

with open(args.output, 'wb') as f:
    f.write(contents)
