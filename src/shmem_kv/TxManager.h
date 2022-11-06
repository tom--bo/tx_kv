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
  std::atomic<TxID> *global_txid;
  ArrayStore *store;
  pthread_mutex_t *txcbpool_mutex_shared;
  bool *txcbUsed;
  TxCB *txcbPool;
  int txcb_id;
 public:
  TxManager(ArrayStore *as, std::atomic<TxID> *gt, pthread_mutex_t *mu, bool *used, TxCB *pool)
      :store{as}, global_txid{gt}, txcbpool_mutex_shared{mu}, txcbUsed{used}, txcbPool{pool} {}

  TxCB *start_tx() {
    TxCB *txcb = nullptr;
    pthread_mutex_lock(txcbpool_mutex_shared); /* lock 01 */
    for(int i = 0; i < TXCB_CAPACITY; i++) {
      if(!txcbUsed[i]) {
        txcb_id = i;
        txcb = &txcbPool[i];
        txcbUsed[i] = true;
        break;
      }
    }
    pthread_mutex_unlock(txcbpool_mutex_shared); /* lock 01 */
    txcb->txid = global_txid->fetch_add(1);
    txcb->lock_head = nullptr;
    txcb->anchor = nullptr;
    txcb->wait = nullptr;
    return txcb;
  }
  ErrorNo commit_tx(TxCB *txcb);
  ErrorNo rollback_tx(TxCB *txcb);
};

#endif//TX_KV__TXMANAGER_H_
