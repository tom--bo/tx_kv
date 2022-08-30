#include "MapStore.h"

ReturnVal MapStore::get(TxCB *txcb, ulong key) {
  auto itr = store.find(key);
  if (itr != store.end()) {
    // found
    return ReturnVal {itr->second, NO_ERROR};
  }
  return ReturnVal {0, KEY_NOT_FOUND};
}

ErrorNo MapStore::put(TxCB *txcb, ulong key, ulong value) {
  store[key] = value;
  return NO_ERROR;
}

ErrorNo MapStore::del(TxCB *txcb, ulong key) {
  auto itr = store.find(key);
  if (itr != store.end()) {
    store.erase(itr);
  }
  return NO_ERROR;
}

ErrorNo MapStore::commit_tx(TxCB *txcb) {

  return NO_ERROR;
}

ErrorNo MapStore::rollback_tx(TxCB *txcb) {

  return NO_ERROR;
}

