#ifndef TX_KV__MAPSTORE_H_
#define TX_KV__MAPSTORE_H_

#include "common.h"
#include <limits.h>
#include <unordered_map>

#define DATA_CAPACITY 100
#define BIT_ARR_SIZE 13 /* DATA_CAPACITY/8+1  */

class TxCB;
class LockManager;

class ArrayStore {
 private:
  char used[BIT_ARR_SIZE] = {};
  ulong data[DATA_CAPACITY] = {};
  LockManager *lock_manager;
 public:
  bool is_used(ulong key);
  void set_used(ulong key);
  void set_not_used(ulong key);
  ReturnVal get(TxCB *txcb, ulong key);
  ErrorNo put(TxCB *txcb, ulong key, ulong value);
  ErrorNo del(TxCB *txcb, ulong key);
  ErrorNo commit_tx(TxCB *txcb);
  ErrorNo rollback_tx(TxCB *txcb);
  ArrayStore(LockManager* lm) {
    lock_manager = lm;
  }
};

#endif//TX_KV__MAPSTORE_H_
