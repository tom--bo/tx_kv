#ifndef TX_KV__LOCKMANAGER_H_
#define TX_KV__LOCKMANAGER_H_

#include "common.h"
#include <atomic>
#include <unordered_map>

class LockHead {
  pthread_mutex_t mu;
};

class LockHash {
  pthread_mutex_t lock_head_mutex;
  std::unordered_map<ulong, LockHead> store;

  LockHash() {
    pthread_mutex_init(&lock_head_mutex, NULL);
    store = std::unordered_map<ulong, LockHead>();
  }
};

class LockManager {
};

#endif//TX_KV__LOCKMANAGER_H_
