#ifndef TX_KV__KVSERVER_H_
#define TX_KV__KVSERVER_H_

#include <iostream>
#include "common.h"
#include "TxManager.h"
#include "MapStore.h"


// Server Program (Tp-Monitor and Application) in TP book
class KVserver {
private:
  TxManager *txManager;
  MapStore *store;
public:
  TxID start_tx();
  ReturnVal get(TxID txid, ulong key);
  ErrorNo put(TxID txid, ulong key, ulong value);
  ErrorNo del(TxID txid, ulong key);
  ErrorNo commit_tx(TxID txid);
  ErrorNo rollback_tx(TxID txid);

  bool db_init() {
    txManager = new TxManager();
    store = new MapStore();
  }

  KVserver() {
    txManager = nullptr;
    store = nullptr;
  }
};


#endif//TX_KV__KVSERVER_H_
