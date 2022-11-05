#include "TxManager.h"
#include "ArrayStore.h"

ErrorNo TxManager::commit_tx(TxCB *txcb) {
  ErrorNo err = store->commit_tx(txcb);
  delete(txcb);
  return err;
};
ErrorNo TxManager::rollback_tx(TxCB *txcb) {
  ErrorNo err = store->rollback_tx(txcb);
  delete(txcb);
  return err;
};