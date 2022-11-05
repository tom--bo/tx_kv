#ifndef TX_KV__KVSERVER_H_
#define TX_KV__KVSERVER_H_

#include "ArrayStore.h"
#include "LockManager.h"
#include "TxManager.h"
#include "common.h"
#include <atomic>
#include <iostream>

// Server Program (Tp-Monitor and Application) in TP book
class KVserver {
 private:
  TxManager *txManager;
  ArrayStore *store;
  std::atomic<uint64_t> connection_id = 1;

 public:
  TxCB *start_tx();
  ReturnVal get(TxCB *txcb, ulong key);
  ErrorNo put(TxCB *txcb, ulong key, ulong value);
  ErrorNo del(TxCB *txcb, ulong key);
  ErrorNo commit_tx(TxCB *txcb);
  ErrorNo rollback_tx(TxCB *txcb);

  bool db_init() {
    LockManager *lm = new LockManager();
    store = new ArrayStore(lm);
    txManager = new TxManager(store);
    return true;
  }

  KVserver() {
    txManager = nullptr;
    store = nullptr;
  }
};


#endif//TX_KV__KVSERVER_H_
