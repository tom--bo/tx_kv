#include "TxManager.h"
#include "ArrayStore.h"

ErrorNo TxManager::commit_tx(TxCB *txcb) {
  ErrorNo err = store->commit_tx(txcb);

  pthread_mutex_lock(txcbpool_mutex_shared); /* lock 01 */
  // todo: init txcb to return pool
  txcb->txid = 0;
  txcb->lock_head = nullptr;
  txcb->anchor = nullptr;
  txcb->wait = nullptr;
  txcb->undo_head = nullptr;
  txcbUsed[txcb_id] = false;
  pthread_mutex_unlock(txcbpool_mutex_shared); /* lock 01 end */

  return err;
};
ErrorNo TxManager::rollback_tx(TxCB *txcb) {
  ErrorNo err = store->rollback_tx(txcb);

  pthread_mutex_lock(txcbpool_mutex_shared); /* lock 01 */
  // todo: init txcb to return pool
  txcb->txid = 0;
  txcb->lock_head = nullptr;
  txcb->anchor = nullptr;
  txcb->wait = nullptr;
  txcb->undo_head = nullptr;
  txcbUsed[txcb_id] = false;
  pthread_mutex_unlock(txcbpool_mutex_shared); /* lock 01 end */

  return err;
};