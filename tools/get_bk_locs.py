#!/opt/local/bin/python3
"""
Get block info from LevelDB.
Input: block hashes (stdin), LevelDB path
Output: bk file+offset (2x4=8 bytes per bk)

Test: 80..90"/630k,

Args: -i /Users/eugene/tmp/btc/test.hex -l /Users/eugene/tmp/blockchain/blocks/index -o /Users/eugene/tmp/btc/test.bin

TODO:
- check infile type (txt/gx/*)
"""

import argparse
import os
import sys
from collections import namedtuple
from struct import pack

# import leveldb
import plyvel

# consts
BLOCK_HAVE_DATA = 8
BLOCK_HAVE_UNDO = 16
TOOBIGOFFSET = 256 << 20  # 256MB > any blk*.dat

# vars
LdbRec = namedtuple('LdbRec', ['v', 'h', 's', 't', 'f', 'd', 'u'])
db = None
TODUMP = False
MAXBKNO = 2999


def eprint(s: str):
    """
    Print message to stderr
    :param s: string to print
    :return: None
    """
    print(s, file=sys.stderr)


def hex2bytes(s: str) -> bytes:
    """
    Converts hex-string to bytes object
    :param s: string to convert
    :return: bytes converted to
    """
    return bytes.fromhex(s)


def bytes2hex(b: bytes) -> str:
    """
    Convert bytes to hex-string
    :param b: bytes to convert
    :return: hex-string converted to
    """
    return b.hex()


def __get_varint(rec: bytes, pos: int = 0) -> tuple:
    """
    Decode serialized varint.
    Powered by Mark Friedenbach's [VarInt](https://github.com/maaku/python-bitcoin/blob/master/bitcoin/serialize.py)
    :param rec: record itself
    :param pos: position to start from
    :return (value, bytes read)
    """
    result = 0
    l = len(rec)
    while pos <= l:
        limb = int(rec[pos])
        pos += 1
        result <<= 7
        result |= limb & 0x7f
        if limb & 0x80:
            result += 1
        else:
            break
    return result, pos


def decode_rec(rec: bytes, bk_no: int) -> object:
    """
    Decode record.
    (see bitoin-core, chain.h, class CDiskBlockIndex)
    :param rec: record to decode
    :param bk_no: block no
    :return: LdbRec object
    """
    LdbRec.v, i = __get_varint(rec)  # version
    LdbRec.h, i = __get_varint(rec, i)  # height
    if LdbRec.h != bk_no:
        eprint("Bk # %d: wrong height (%d)" % (bk_no, LdbRec.h))
    LdbRec.s, i = __get_varint(rec, i)  # status
    LdbRec.t, i = __get_varint(rec, i)  # nTx
    LdbRec.f = LdbRec.d = LdbRec.u = None
    if LdbRec.s & (BLOCK_HAVE_DATA | BLOCK_HAVE_UNDO):
        LdbRec.f, i = __get_varint(rec, i)  # File no
        if LdbRec.f > MAXBKNO:
            eprint("Bk # %d: file no is too big: %d" % (bk_no, LdbRec.f))
    if LdbRec.s & BLOCK_HAVE_DATA:
        LdbRec.d, i = __get_varint(rec, i)  # blk*.dat offset
        if LdbRec.d > TOOBIGOFFSET:
            eprint("Bk # %d: data offset is too big: %d" % (bk_no, LdbRec.d))
        elif LdbRec.d < 8:
            eprint("Bk # %d: data offset cannot be < 8: %d" % (bk_no, LdbRec.d))
    if LdbRec.s & BLOCK_HAVE_UNDO:
        LdbRec.u, i = __get_varint(rec, i)  # rev*.dat offset
        if LdbRec.u > TOOBIGOFFSET:
            eprint("Bk # %d: undo offset is too big: %d" % (bk_no, LdbRec.u))
        elif LdbRec.u < 8:
            eprint("Bk # %d: undo offset cannot be < 8: %d" % (bk_no, LdbRec.u))
    if i != len(rec):
        eprint("Bk # %d: bytes decoded (%d) != record len (%d)." % (bk_no, i, len(rec)))
    return LdbRec


def get_ldbrec(hash_b: bytes) -> bytes:
    """
    Get LevelDB's record of bk. Excluding bk head (last 80 bytes)
    :param hash_b: block record to find
    :return: raw block record
    """
    global db
    k = b'\x62' + hash_b
    retvalue = None
    ''' leveldb
    try:
        v = db.Get(k)  # bytearray; TODO: try...except(EOFError)
    except KeyError:
        eprint("Hash {} not found.".format(bytes2hex(hash_b)))
    else:
        retvalue = bytes(v[:-80])
    '''
    v = db.get(k)
    if not v:
        eprint("Hash {} not found.".format(bytes2hex(hash_b)))
    else:
        retvalue = v[:-80]
    return retvalue


def dump_rec(height: int, rec: bytes):
    """
    Save block record to file (to debug).
    :param height: block no
    :param rec: raw block record
    :return:
    """
    with open("ldb/%06d.ldb" % height, "wb") as f:
        f.write(rec)
        f.close()


def print_rec(ldbrec):
    """
    Print block record (to debug).
    :param ldbrec: block record to print
    :return: None
    """
    print("%d\t%d\t%x\t%d" %
          (ldbrec.v, ldbrec.h, ldbrec.s, ldbrec.t), end='\t')
    if ldbrec.f is None:
        print("-----", end='\t')
    else:
        print("%05d" % ldbrec.f, end='\t')
    if ldbrec.d:
        print(ldbrec.d, end='\t')
    else:
        print("-----", end='\t')
    if ldbrec.u:
        print(ldbrec.u)
    else:
        print("-----")


def walk(sfn: str, dfn: str, ldb: str, verbose: bool) -> int:
    """
    @param sfn: input file name (hashes hex strings (64 per line))
    @param dfn: output file name (2x4 bytes per block)
    @param ldb: LevelDB directory
    @param verbose: no comments
    @return: None
    """
    global db
    # 0. prepare
    sf = open(sfn, "rt")
    if not sf:
        eprint("Can't open %s to read" % sfn)
        return 1
    df = open(dfn, "wb")
    if not df:
        eprint("Can't open %s to write" % dfn)
        return 1
    # db = leveldb.LevelDB(ldb, create_if_missing=False)
    db = plyvel.DB(ldb, create_if_missing=False)
    if verbose:  # header
        print("Ver\tHeight\tStatus\tnTx\tFile\tDoffset\tUoffset")
        print("===\t======\t======\t===\t====\t=======\t=======")
    bk_no = 0
    for line in sf:
        # hash_b = sf.read(32)[::-1]  # 1. get bk hash
        hash_b = hex2bytes(line.rstrip('\n'))
        if not hash_b:
            break
        rec = get_ldbrec(hash_b)  # 2. get ldb rec; FIXME: error handle
        if not rec:
            break
        r = decode_rec(rec, bk_no)  # 3. decode it
        df.write(pack("<I", r.f))  # 4. write_db
        df.write(pack("<I", r.d))
        if TODUMP:
            dump_rec(bk_no, rec)
        if verbose:
            print_rec(r)
        bk_no += 1
    sf.close()
    df.close()
    if verbose:  # footer
        print("===\t======\t======\t===\t====\t=======\t=======")
    eprint("%s records ok." % bk_no)
    return 0


def init_cli():
    """
    Handle CLI
    """
    parser = argparse.ArgumentParser(description='Hash to file-offset.')
    parser.add_argument('ldb', metavar='<dir>', type=str, nargs=1,
                        help='LevelDB dir')
    parser.add_argument('outfile', metavar='<file>', type=str, nargs=1,
                        help='Bk locs file')
    parser.add_argument('-i', '--infile', metavar='<file>', type=str, nargs=1, default=None,
                        help='Input file')
    parser.add_argument('-v', '--verbose', action='store_true',
                        help='Debug process (default=false)')
    return parser


def main():
    parser = init_cli()
    args = parser.parse_args()
    # print(args.infile, type(args.infile))
    if (not args.infile) or (not args.outfile) or (not args.ldb):
        parser.print_help()
    else:
        ifile = args.infile[0]
        ofile = args.outfile[0]
        ldir = args.ldb[0]
        if not os.path.isfile(ifile):
            eprint("Infile '{}' not exist or is not file.".format(ifile))
        elif not os.path.isdir(ldir):
            eprint("LevelDB '{}' not exist or is not dir.".format(ldir))
        else:
            return walk(ifile, ofile, ldir, args.verbose)


if __name__ == '__main__':
    main()
