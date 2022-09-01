#ifndef TX_KV__LOCKMANAGER_H_
#define TX_KV__LOCKMANAGER_H_

#include "LockDef.h"
#include "common.h"
#include <atomic>
#include <unordered_map>

struct TxCB;

class LockHash {
 private:
  /* To add lock_head safely */
  pthread_mutex_t lock_head_mutex;
  /* Lock Hash, temporary not Hash but Hashmap */
  std::unordered_map<ulong, LockHead*> lockhash;

 public:
  LockHash() {
    pthread_mutex_init(&lock_head_mutex, NULL);
    lockhash = std::unordered_map<ulong, LockHead*>();
  }

  LockHead *FindLockHead(ulong key) {
    auto itr = lockhash.find(key);
    if(itr != lockhash.end()) {
      return itr->second;
    }
    return nullptr;
  }

  LockHead *CreateLockHead(ulong key) {
    LockHead *lh = new LockHead();
    /* Protect LockHead with lock_head_mutex */
    pthread_mutex_lock(&lock_head_mutex);
    auto itr = lockhash.find(key);
    if(itr != lockhash.end()) {
      delete(lh);
      pthread_mutex_unlock(&lock_head_mutex);
      return itr->second;
    } else {
      lockhash[key] = lh;
    }
    pthread_mutex_unlock(&lock_head_mutex);
    /* [END] Protect LockHead with lock_head_mutex */
    return lh;
  }
};

class LockManager {
 private:
  LockHash *lockhash;
 public:
  LockManager() {
    lockhash = new LockHash();
  }
  /* LockClass and timeout are not impl yet */
  /* LockHash is not hashed, so specify key directly now */
  LockReply Lock(TxCB *me, ulong key, LockMode mode);
  bool Unlock(LockRequest *req);
  bool UnlockAll(TxCB *txcb);
  /* TP-book(jp) p494 */
  bool LockCompat(LockMode requested, LockMode granted);
  LockMode GrantGroup(LockMode requested, LockMode granted);
};

#endif//TX_KV__LOCKMANAGER_H_
