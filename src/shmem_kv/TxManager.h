#ifndef TX_KV__TXMANAGER_H_
#define TX_KV__TXMANAGER_H_

#include "LockDef.h"
#include "common.h"
#include "undo.h"
#include <atomic>

class ArrayStore;

class TxCB {
 public:
  TxID txid;
  LockRequest *lock_head;
  LockRequest *anchor; /* 'locks' in TP-book explained as the anchor of tx-lock-list */
  LockRequest *wait;
  UndoRecord  *undo_head;
  // TxCB *cycle; /* Not impl yet */
};

class TxManager {
 private:
  std::atomic<TxID> global_txid;
  ArrayStore *store;
 public:
  TxManager(ArrayStore *map_store) {
    global_txid = 1;
    store = map_store;
  }
  TxCB *start_tx() {
    TxCB *txcb = new TxCB();
    txcb->txid = global_txid.fetch_add(1);
    txcb->lock_head = nullptr;
    txcb->anchor = nullptr;
    txcb->wait = nullptr;
    return txcb;
  }
  ErrorNo commit_tx(TxCB *txcb);
  ErrorNo rollback_tx(TxCB *txcb);
};

#endif//TX_KV__TXMANAGER_H_
