#!/usr/bin/env python3

import argparse
import socket
import struct
import ipaddress
import enum
import binascii

import sancus.config
import sancus.crypto


class Command(enum.IntEnum):
    Connect   = 0x0
    SetKey    = 0x1
    PostEvent = 0x2


class ResultCode(enum.IntEnum):
    Ok                = 0x0
    ErrIllegalCommand = 0x1
    ErrPayloadFormat  = 0x2
    ErrInternal       = 0x3


class Result:
    def __init__(self, code, payload=bytearray()):
        self.code = code
        self.payload = payload


class SetKeyResultCode(enum.IntEnum):
    Ok                   = 0x0,
    ErrIllegalConnection = 0x1,
    ErrWrongTag          = 0x2


class Error(Exception):
    pass


def hex_16bit(arg):
    try:
        id = int(arg, 16)
    except ValueError:
        raise argparse.ArgumentTypeError('Expected a hexadecimal integer')

    if 0 <= id < 2**16:
        return id

    raise argparse.ArgumentTypeError('The SM ID should be a 16-bit integer')


def hex_key(arg):
    try:
        key = binascii.unhexlify(arg)
    except binascii.Error:
        raise argparse.ArgumentTypeError('Expected a hexadecimal key')

    if len(key) != sancus.config.SECURITY / 8:
        msg = 'Expected a {} bit key, got a {} bit one' \
                    .format(sancus.config.SECURITY, len(key) * 8)
        raise argparse.ArgumentTypeError(msg)

    return key


def pack_int(i):
    return struct.pack('!H', i)


def unpack_int(buf):
    return struct.unpack('!H', buf)[0]


def create_packet(command, payload):
    return pack_int(command) + pack_int(len(payload)) + payload


def recv_result(sock, length=0):
    data = bytearray()

    while len(data) < length + 1:
        data += sock.recv(4096)

    return Result(ResultCode(data[0]), data[1:])


def to_hex_str(data):
    return binascii.hexlify(data).decode('ascii')


def handle_connect(sock, args):
    if args.to_address is None:
        args.to_address = args.server

    # The payload format is [from_sm from_output to_sm to_address to_input]
    payload = pack_int(args.from_sm)     + \
              pack_int(args.from_output) + \
              pack_int(args.to_sm)       + \
              args.to_address.packed     + \
              pack_int(args.to_input)

    sock.sendall(create_packet(Command.Connect, payload))
    result = recv_result(sock)

    if result.code != ResultCode.Ok:
        raise Error('Error sending connect command: {}'.format(result.code))


def handle_set_key(sock, args):
    nonce = pack_int(args.nonce)
    connection_id = pack_int(args.connection)
    ad = nonce + connection_id
    cipher, tag = sancus.crypto.wrap(args.sm_key, ad, args.key)

    # The payload format is [sm_id, 16 bit nonce, index, wrapped(key), tag]
    # where the tag includes the nonce and the index.
    payload = pack_int(args.sm_id) + ad + cipher + tag
    sock.sendall(create_packet(Command.SetKey, payload))

    # The result format is [16 bit result code, tag] where the tag includes the
    # nonce and the result code.
    result_len = 2 + sancus.config.SECURITY // 8
    result = recv_result(sock, result_len)

    if result.code != ResultCode.Ok:
        raise Error('Error sending set-key command: {}'.format(result.code))

    if len(result.payload) != result_len:
        msg = 'Wrong result payload length (expected {}, got {})' \
                    .format(result_len, len(result.payload))
        raise Error(msg)

    set_key_raw_code = result.payload[0:2]
    set_key_code = unpack_int(set_key_raw_code)
    set_key_tag = result.payload[2:]
    set_key_ad = nonce + set_key_raw_code
    expected_tag = sancus.crypto.mac(args.sm_key, set_key_ad)

    if set_key_tag != expected_tag:
        msg = 'Response from SM has a wrong tag (expected {}, got {}' \
                .format(to_hex_str(expected_tag), to_hex_str(set_key_tag))
        raise Error(msg)

    if set_key_code != SetKeyResultCode.Ok:
        raise Error('Got error code from SM: {}'.format(set_key_code))


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

connect_parser = subparsers.add_parser('set-key',
                                       help='Set the key used for a connection')
connect_parser.set_defaults(command_handler=handle_set_key)
connect_parser.add_argument('--sm-id',
                            help='ID of the SM',
                            type=hex_16bit,
                            required=True)
connect_parser.add_argument('--connection',
                            help='ID of the connection',
                            type=hex_16bit,
                            required=True)
connect_parser.add_argument('--nonce',
                            help='Nonce to use',
                            type=hex_16bit,
                            required=True)
connect_parser.add_argument('--sm-key',
                            help='Key to communicate with the SM',
                            type=hex_key,
                            required=True)
connect_parser.add_argument('key',
                            help='Key to set',
                            type=hex_key)

args = parser.parse_args()

with socket.create_connection((str(args.server), args.port), timeout=2) as sock:
    args.command_handler(sock, args)
    print('OK')
