/*
 * Key-value backend.
 * Tkrzw version.
 */
#ifdef USE_TK
#include "kv/tk.h"
#include "misc.h"

using namespace std;

bool        KV_TK_HASH_T::init(const string &s) {
    if (db.Open(s + ".tkh", true, tkrzw::File::OPEN_TRUNCATE) != tkrzw::Status::SUCCESS)
        cerr << "Can't open db '" << s << "'." << endl;
    return db.IsOpen();
}

bool    KV_TK_HASH_T::close(void) {
    if (db.IsOpen()) {
        db.Synchronize(true);
        db.Close();
    }
    return (!db.IsOpen());
}

void  KV_TK_HASH_T::clear(void) {
  db.Clear();
}

uint32_t    KV_TK_HASH_T::count(void) {
    auto retvalue = db.CountSimple();
    return (retvalue < 0) ? NOT_FOUND_U32 : uint32_t(retvalue);
}

uint32_t    KV_TK_HASH_T::add(string_view key) {
    uint32_t value = count();
    if (value != NOT_FOUND_U32)
        if (!db.Set(key, string_view((const char *)&value, sizeof(uint32_t))).IsOK())
            value = NOT_FOUND_U32;
    return value;
}

uint32_t    KV_TK_HASH_T::get(string_view key) {
    string value;
    if (!db.Get(key, &value).IsOK())    // FXIME: handle errors
        return NOT_FOUND_U32;
    if (value.length() != 4) {
      cerr << "Bad key len: " << value.length() << endl;
      return NOT_FOUND_U32;
    }
    return *((uint32_t *) value.data());
}

uint32_t    KV_TK_HASH_T::get_or_add(std::string_view key) {
  string old_value;
  uint32_t value = count();
  if (value != NOT_FOUND_U32) {
    auto status = db.Set(key, string_view((const char *)&value, sizeof(uint32_t)), false, &old_value);
    if (status == tkrzw::Status::DUPLICATION_ERROR)
      value = *((uint32_t *) old_value.data());
    else if (!status.IsOK()) {
      cerr << "Something wrong with get_or_add" << endl;
      value = NOT_FOUND_U32;
    }
  }
  return value;
}

#endif