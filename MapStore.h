#ifndef TX_KV__MAPSTORE_H_
#define TX_KV__MAPSTORE_H_

#include <limits.h>
#include <unordered_map>
#include "common.h"
#include "LockManager.h"

class MapStore {
public:
  ReturnVal get(TxID txid, ulong key);
  ErrorNo put(TxID txid, ulong key, ulong value);
  ErrorNo del(TxID txid, ulong key);
  ErrorNo commit_tx(TxID txid);
  ErrorNo rollback_tx(TxID txid);
  MapStore() {
    store = std::unordered_map<ulong, ulong>();
    lock_manager = new LockManager();
  }
  ~MapStore() {
    store.clear();
  }

private:
  std::unordered_map<ulong, ulong> store;
  LockManager *lock_manager;
};

#endif//TX_KV__MAPSTORE_H_
