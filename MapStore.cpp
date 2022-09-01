#include "MapStore.h"
#include "LockManager.h"

ReturnVal MapStore::get(TxCB *txcb, ulong key) {
  lock_manager->Lock(txcb, key, LOCK_S);
  auto itr = store.find(key);
  if (itr != store.end()) {
    // found
    return ReturnVal {itr->second, NO_ERROR};
  }
  return ReturnVal {0, KEY_NOT_FOUND};
}

ErrorNo MapStore::put(TxCB *txcb, ulong key, ulong value) {
  lock_manager->Lock(txcb, key, LOCK_X);
  store[key] = value;
  return NO_ERROR;
}

ErrorNo MapStore::del(TxCB *txcb, ulong key) {
  lock_manager->Lock(txcb, key, LOCK_X);
  auto itr = store.find(key);
  if (itr != store.end()) {
    store.erase(itr);
  }
  return NO_ERROR;
}

ErrorNo MapStore::commit_tx(TxCB *txcb) {
  lock_manager->UnlockAll(txcb);
  return NO_ERROR;
}

ErrorNo MapStore::rollback_tx(TxCB *txcb) {
  lock_manager->UnlockAll(txcb);
  return NO_ERROR;
}

