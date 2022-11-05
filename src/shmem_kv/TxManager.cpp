#include "TxManager.h"
#include "MapStore.h"

ErrorNo TxManager::commit_tx(TxCB *txcb) {
  return store->commit_tx(txcb);
};
ErrorNo TxManager::rollback_tx(TxCB *txcb) {
  return store->rollback_tx(txcb);
};