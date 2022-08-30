#ifndef TX_KV__LOCKMANAGER_H_
#define TX_KV__LOCKMANAGER_H_

#include "LockDef.h"
#include "TxManager.h"
#include "common.h"
#include <atomic>
#include <unordered_map>

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
 public:
  LockReply Lock() {
    return LOCK_OK;
  }
  bool Unlock() {
    return true;
  }
  bool UnlockAll(TxCB *txcb) {

  }

};

#endif//TX_KV__LOCKMANAGER_H_
