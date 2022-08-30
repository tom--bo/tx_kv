#ifndef TX_KV__TXMANAGER_H_
#define TX_KV__TXMANAGER_H_

#include "LockDef.h"
#include "common.h"
#include <atomic>

typedef struct {
  TxID txid;
  LockRequest *last; /* 'locks' in TP-book explained as the anchor of tx-lock-list */
  LockRequest *wait;
  // TxCB *cycle; /* Not impl yet */
} TxCB;

class TxManager {
 public:
  std::atomic<TxID> global_txid;
  TxManager() {
    global_txid = 1;
  }
  TxCB *start_tx() {
    TxCB *txcb = new TxCB();
    txcb->txid = global_txid.fetch_add(1);
    txcb->last = nullptr;
    txcb->wait = nullptr;
    return txcb;
  }
  ErrorNo commit_tx(TxCB *txcb) {
      return NO_ERROR;
  };
  ErrorNo rollback_tx(TxCB *txcb) {
      return NO_ERROR;
  };
};

#endif//TX_KV__TXMANAGER_H_
