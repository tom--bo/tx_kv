#ifndef TX_KV__MAPSTORE_H_
#define TX_KV__MAPSTORE_H_

#include "kv/common.h"
#include <limits.h>
#include <unordered_map>

class TxCB;
class LockManager;

class MapStore {
 private:
  std::unordered_map<ulong, ulong> store;
  LockManager *lock_manager;
 public:
  ReturnVal get(TxCB *txcb, ulong key);
  ErrorNo put(TxCB *txcb, ulong key, ulong value);
  ErrorNo del(TxCB *txcb, ulong key);
  ErrorNo commit_tx(TxCB *txcb);
  ErrorNo rollback_tx(TxCB *txcb);
  MapStore(LockManager* lm) {
    store = std::unordered_map<ulong, ulong>();
    lock_manager = lm;
  }
  ~MapStore() {
    store.clear();
  }
};

#endif//TX_KV__MAPSTORE_H_
