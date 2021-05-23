/**
 * Order:
 * - Create items ("fast walk")
 * - Parse (w/ hashes (bk, tx); maybe multithread)
 * - Resolve from k-v [+ vvv]
 * - Save to k-v
 * - Print
 */
#ifndef BK_H
#define BK_H

#include <string_view>
#include <vector>
#include "common.h"
#include "addr.h"

class VIN_T {
private:
  uint32_t no, tx_no, bk_no;
  const uint256_t  *tx_hash;
  uint32_t          vout;
  std::string_view  script;
  uint32_t          seq;
  uint32_t          tx_id;  // resolving
  friend void out_vin(const VIN_T &);
  friend void prn_vin(const VIN_T &);
public:
  VIN_T(UNIPTR_T &, const uint32_t, const uint32_t, const uint32_t);
  bool parse(void);
  bool resolve(void);
};

class VOUT_T {
private:
  uint32_t no, tx_no, bk_no;
  uint64_t  satoshi;
  uint32_t addr_id = MAX_UINT32;  // aka NOT_FOUND
  std::string_view script;
  ADDR_BASE_T *addr = nullptr;
  friend void out_vout(const VOUT_T &);
  friend void prn_vout(const VOUT_T &);
public:
  VOUT_T(UNIPTR_T &, const uint32_t, const uint32_t, const uint32_t);
  bool parse(void);
  bool resolve(void);
};

class WIT_T {
private:
  uint32_t no, tx_no, bk_no;
public:
  WIT_T(UNIPTR_T &, const uint32_t, const uint32_t, const uint32_t);
};

class TX_T {
private:
  uint32_t no, bk_no;
  std::string_view data;  // for hash calc
  uint256_t hash;
  uint32_t ver;
  bool segwit;
  uint32_t wit_offset; ///< cut off for hash calc
  std::vector<VIN_T> vins;
  std::vector<VOUT_T> vouts;
  std::vector<WIT_T> wits;
  void mk_hash(void);
  friend void out_tx(const TX_T &);
  friend void prn_tx(const TX_T &);
public:
  TX_T(UNIPTR_T &, const uint32_t, const uint32_t);
  bool parse(void);
  bool resolve(void);
};

class BK_T {
private:
  uint32_t height;
  uint32_t time;
  uint256_t hash;
  std::string_view data;
  std::vector<TX_T> txs;
  void mk_hash(void);
  friend void out_bk(const BK_T &);
  friend void prn_bk(const BK_T &);
public:
  // TODO: delete data in destructor
  BK_T(std::string_view, const uint32_t);
  bool parse(void);
  bool resolve(void);
};

#endif // BK_H