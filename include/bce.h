#ifndef BCE_H
#define BCE_H

// TODO: stacktrace - file, byte, bk[, tx[, vin+n|vout+n]

#include <iostream>
#include <iomanip>
#include <ctime>
#include <fstream>
#include <array>

#include "uintxxx.h"
#include "kv.h"

enum    DBG_LVL_T
{
    DBG_NONE,
    DBG_MIN,
    DBG_MID,
    DBG_MAX
};

struct  OPT_T       ///< program CLI options
{
    string      datdir;
    string      cachedir;
    int         from;
    uint32_t    num = 1;
    bool        out = false;
    bool        cash = false;
    bool        inmem = false;
    DBG_LVL_T   verbose = DBG_NONE;
};

struct  COUNT_T     ///< through counters
{
    uint32_t    bk = 0;
    uint32_t    tx = 0;     // session if no cache
    uint32_t    addr = 0;
};

struct  STAT_T      ///< session counters
{
    uint32_t    max_txs = 0;
    uint32_t    vins = 0;
    uint32_t    max_vins = 0;
    uint32_t    vouts = 0;
    uint32_t    max_vouts = 0;
    uint32_t    addrs = 0;
    uint32_t    max_addrs = 0;
};

struct  LOCAL_T     ///< per parent counters
{
    uint32_t    tx = 0;
    uint32_t    vin = 0;
    uint32_t    vout = 0;
    uint32_t    wit = 0;
};

struct  BUSY_T
{
    bool    bk = false;
    bool    tx = false;
    bool    vin = false;
    bool    vout = false;
};

struct  BK_HEAD_T   ///< bk header, 80 bytes
{
    uint32_t    ver;    // real head start (80 bytes)
    uint256_t   p_hash;
    uint256_t   mroot;
    uint32_t    time;
    uint32_t    bits;
    uint32_t    nonce;  // real head end
};

struct  BK_T        ///< whole of bk data w/o txs
{
    BK_HEAD_T   *head_ptr;
    uint32_t    txs;
    uint256_t   hash;
};

struct  TX_T        ///< tx variables w/o vins/vouts
{
    uint32_t    ver;        // FIXME: * (for hash)
    uint32_t    vins;
    uint32_t    vouts;
    //uint32_t    wits;
    uint32_t    locktime;   // FIXME: * (for hash)
    uint256_t   hash;
    bool        segwit;
};

struct  VIN_T       ///< vin data
{
    uint256_t   *txid;
    uint64_t    txno;       // ?
    uint32_t    vout;
    uint32_t    ssize;      // vint
    uint8_t     *script;
    uint32_t    seq;
};

struct  VOUT_T      ///< vout data
{
    uint64_t    satoshi;
    uint32_t    ssize;      // vint
    uint8_t     *script;
};

struct  BUFFER_T    ///< loaded block (raw)
{
    char        *beg = nullptr;
};

union   UNIPTR_T    ///< Universal ptr
{
    void        *v_ptr;
    uint8_t     *u8_ptr;
    uint16_t    *u16_ptr;
    uint32_t    *u32_ptr;
    uint64_t    *u64_ptr;
    uint256_t   *u256_ptr;
};

extern OPT_T    OPTS;
extern DBG_LVL_T DBG_LVL;
extern COUNT_T  COUNT;
extern STAT_T   STAT;
extern LOCAL_T  LOCAL;
extern BUSY_T   BUSY;
extern BK_T     CUR_BK;
extern TX_T     CUR_TX;
extern VIN_T    CUR_VIN;
extern VOUT_T   CUR_VOUT;
extern UNIPTR_T CUR_PTR;
extern BUFFER_T BUFFER;
extern KV_T     *TxDB, *AddrDB;

extern time_t   start_time;
extern long     start_mem;

inline bool     kv_mode(void) {return (OPTS.cash or OPTS.inmem);}

const string TAB = "\t";
const uint32_t COINBASE_vout = 0xFFFFFFFF;

#endif // BCE_H