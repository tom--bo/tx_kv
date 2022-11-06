#ifndef TX_KV__LOCKMANAGER_H_
#define TX_KV__LOCKMANAGER_H_

#include "common.h"
#include "LockDef.h"
#include <atomic>
#include <unordered_map>

struct TxCB;

class LockManager {
 private:
  LockHead *lock_array;
  pthread_mutex_t *lockRequest_mutex;
  bool *LockReqUsed;
  LockRequest *LockRequestPool;
 public:
  LockManager(LockHead* la, pthread_mutex_t *mu, bool *used, LockRequest *pool)
      :lock_array{la}, lockRequest_mutex{mu}, LockReqUsed{used}, LockRequestPool{pool} {}
  /* LockClass and timeout are not impl yet */
  LockReply Lock(TxCB *me, ulong key, LockMode mode);
  bool Unlock(LockRequest *req);
  bool UnlockAll(TxCB *txcb);
  /* TP-book(jp) p494 */
  bool LockCompat(LockMode requested, LockMode granted);
  LockMode GrantGroup(LockMode requested, LockMode granted);
};

#endif//TX_KV__LOCKMANAGER_H_
