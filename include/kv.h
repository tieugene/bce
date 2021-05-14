/** Key-value storage */
#ifndef KV_H
#define KV_H

#include <map>
#ifdef USE_KC
#include "kv/kc.h"
#endif
#ifdef USE_TK
#include "kv/tk.h"
#endif

/// K-V storage engine type
enum KVNGIN_T {
  KVTYPE_NONE
#ifdef USE_KC
  ,KVTYPE_KCFILE
  ,KVTYPE_KCMEM
#endif
#ifdef USE_TK
  ,KVTYPE_TKFILE
  ,KVTYPE_TKMEM
#endif
};

/**
 * @brief Set up K-V storages
 * @return True on success
 */
bool    set_cache(void);
/// Reset k-v storages
void    stop_cache(void);

#endif // KV_H
