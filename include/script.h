/// Decoding vout's scripts

#ifndef SCRIPT_H
#define SCRIPT_H

#include <vector>
#include <array>
#include <string_view>
#include "uintxxx.h"

typedef std::vector<std::string> string_list;

enum    SCTYPE {    // script (address) type
    PUBKEYu,        // P2PKu (65)
    PUBKEYc,        // P2PKc (33)
    PUBKEYHASH,     // P2PKH (20)
    SCRIPTHASH,     // P2SH (20)
    MULTISIG,       // P2MS, PUBKEYx[]
    W0KEYHASH,      // P2WPKH? (20)
    W0SCRIPTHASH,   // P2WSH (32)
    NULLDATA,
    NONSTANDARD
};

class ADDR_FOUND_T {
    // add addr: hash160 (P2PK, P2MS) / memcpy
    // get addr: for KC.add()/get()
private:
    uint32_t    id;
    SCTYPE      type;
    uint8_t     qty;
    uint16_t    len;
    union   {
        u8_t      u8[16*sizeof(uint160_t)];    // max P2MS size
        std::array<uint160_t, 16>   u160;
        uint256_t *u256;
    } buffer;
public:
    void                reset(void);
    inline void         set_id(const uint32_t v) { id = v; }
    inline uint32_t     get_id(void) { return id; }
    inline SCTYPE       get_type(void) { return type; }
    const char         *get_type_name(void);
    inline uint8_t      get_qty(void) { return qty; }
    const string_list   get_strings(void);
    void                add_data(const SCTYPE, const u8_t *);
    inline uint16_t     get_len(void) { return len; }
    inline u8_t        *get_data() { return buffer.u8; }
    std::string_view    get_view(void) { return std::string_view((char *) buffer.u8, len); }
    void                sort_multisig(void);
};

bool        script_decode(const u8_t *, const uint32_t);

extern ADDR_FOUND_T CUR_ADDR;

#endif // SCRIPT_H
