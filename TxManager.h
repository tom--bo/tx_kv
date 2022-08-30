#ifndef TX_KV__TXMANAGER_H_
#define TX_KV__TXMANAGER_H_

#include "common.h"
#include <atomic>

class TxManager {
public:
  std::atomic<TxID> global_txid;
  TxManager() {
    global_txid = 1;
  }
  TxID start_tx() {
    return global_txid.fetch_add(1);
  }
  ErrorNo commit_tx(TxID txid) {
      return NO_ERROR;
  };
  ErrorNo rollback_tx(TxID txid) {
      return NO_ERROR;
  };
};

#endif//TX_KV__TXMANAGER_H_
