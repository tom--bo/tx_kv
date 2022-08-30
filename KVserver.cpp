#include "KVserver.h"

TxCB *KVserver::start_tx() {
  return txManager->start_tx();
}

ReturnVal KVserver::get(TxCB *txcb, ulong key){
  return store->get(txcb, key);
}

ErrorNo KVserver::put(TxCB *txcd, ulong key, ulong value){
  return store->put(txcd, key, value);
}

ErrorNo KVserver::del(TxCB *txcd, ulong key){
  return store->del(txcd, key);
}

ErrorNo KVserver::commit_tx(TxCB *txcd){
  return txManager->commit_tx(txcd);
}

ErrorNo KVserver::rollback_tx(TxCB *txcd) {
  return txManager->rollback_tx(txcd);
}


