#ifndef TX_KV__MAPSTORE_H_
#define TX_KV__MAPSTORE_H_

#include <limits.h>
#include <unordered_map>
#include "common.h"
#include "LockManager.h"
#include "TxManager.h"

class MapStore {
 public:
  ReturnVal get(TxCB *txcb, ulong key);
  ErrorNo put(TxCB *txcb, ulong key, ulong value);
  ErrorNo del(TxCB *txcb, ulong key);
  ErrorNo commit_tx(TxCB *txcb);
  ErrorNo rollback_tx(TxCB *txcb);
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
