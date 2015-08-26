#!/usr/bin/env python3

import argparse
import socket
import struct
import ipaddress
import enum


class Command(enum.IntEnum):
    Connect   = 0x0
    SetKey    = 0x1
    PostEvent = 0x2


class Result(enum.IntEnum):
    Ok                = 0x0
    ErrIllegalCommand = 0x1
    ErrPayloadFormat  = 0x2
    ErrInternal       = 0x3


def hex_16bit(arg):
    try:
        id = int(arg, 16)
    except ValueError:
        raise argparse.ArgumentTypeError('Expected a hexadecimal integer')

    if 0 <= id < 2**16:
        return id

    raise argparse.ArgumentTypeError('The SM ID should be a 16-bit integer')


def pack_int(i):
    return struct.pack('!H', i)


def unpack_int(buf):
    return struct.unpack('!H', buf)[0]


def create_packet(command, payload):
    return pack_int(command) + pack_int(len(payload)) + payload


def handle_connect(sock, args):
    # The payload format is [from_sm from_output to_sm to_address to_input]
    payload = pack_int(args.from_sm)     + \
              pack_int(args.from_output) + \
              pack_int(args.to_sm)       + \
              args.to_address.packed     + \
              pack_int(args.to_input)

    sock.sendall(create_packet(Command.Connect, payload))
    result = Result(sock.recv(1)[0])

    if result != Result.Ok:
        print('Error sending connect command: {}'.format(result))


parser = argparse.ArgumentParser()
parser.add_argument('--server',
                    help='IP address/host name of the reactive server',
                    type=ipaddress.IPv4Address,
                    required=True)
parser.add_argument('--port',
                    help='Port on which the reactive server is listening',
                    type=int,
                    default=2001)

subparsers = parser.add_subparsers(dest='command')
# Workaround for a Python bug. See http://bugs.python.org/issue9253#msg186387
subparsers.required = True

connect_parser = subparsers.add_parser('connect', help='Add a connection')
connect_parser.set_defaults(command_handler=handle_connect)
connect_parser.add_argument('--from-sm',
                            help='ID of the "from" SM',
                            type=hex_16bit,
                            required=True)
connect_parser.add_argument('--from-output',
                            help='Output ID of the "from" SM',
                            type=hex_16bit,
                            required=True)
connect_parser.add_argument('--to-sm',
                            help='ID of the "to" SM',
                            type=hex_16bit,
                            required=True)
connect_parser.add_argument('--to-address',
                            help='IP address of the node of the "to" SM '
                                 '(defaults to the server\'s address)',
                            type=ipaddress.IPv4Address)
connect_parser.add_argument('--to-input',
                            help='Input ID of the "to" SM',
                            type=hex_16bit,
                            required=True)

args = parser.parse_args()

if args.to_address is None:
    args.to_address = args.server

with socket.create_connection((str(args.server), args.port), timeout=2) as sock:
    args.command_handler(sock, args)
