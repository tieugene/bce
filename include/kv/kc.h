#ifndef KV_KC_H
#define KV_KC_H

#include <kchashdb.h>
#include "kv/base.h"

// HashDB
const std::string TxFileName = "tx.kch";
const std::string AddrFileName = "addr.kch";
// StashDB
const std::string TxMemName = ":";
const std::string AddrMemName = ":";

class KV_KC_T : public KV_BASE_T {
private:
    kyotocabinet::HashDB     db;
    bool        opened = false;
public:
    bool        init(const std::string &);
    bool        close(void);
    void        clear(void);
    uint32_t    count(void);
    uint32_t    add(std::string_view key);
    uint32_t    add(const uint256_t &key)
                { return add(std::string_view(reinterpret_cast<const char *>(std::data(key)), sizeof(uint256_t))); }
    uint32_t    get(std::string_view key);
    uint32_t    get(const uint256_t &key)
                { return get(std::string_view(reinterpret_cast<const char *>(std::data(key)), sizeof(uint256_t))); }
    uint32_t    get_or_add(std::string_view key);
};

#endif // KV_KC_H
