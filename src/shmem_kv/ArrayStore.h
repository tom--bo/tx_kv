#ifndef TX_KV__MAPSTORE_H_
#define TX_KV__MAPSTORE_H_

#include "common.h"
#include <limits.h>
#include <unordered_map>

class TxCB;
class LockManager;

class ArrayStore {
 private:
  LockManager *lock_manager;
  char *used;
  ulong *data;
 public:
  bool is_used(ulong key);
  void set_used(ulong key);
  void set_not_used(ulong key);
  ReturnVal get(TxCB *txcb, ulong key);
  ErrorNo put(TxCB *txcb, ulong key, ulong value);
  ErrorNo del(TxCB *txcb, ulong key);
  ErrorNo commit_tx(TxCB *txcb);
  ErrorNo rollback_tx(TxCB *txcb);
  ArrayStore(LockManager* lm, char *u, ulong *d): lock_manager{lm}, used{u}, data{d} {}
};

#endif//TX_KV__MAPSTORE_H_
