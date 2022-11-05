#include "KVserver.h"

TxCB *KVserver::start_tx() {
  return txManager->start_tx();
}

ReturnVal KVserver::get(TxCB *txcb, ulong key){
  ReturnVal ret;
  ret = store->get(txcb, key);
  if(ret.error_no == TIMEOUT) {
      txManager->rollback_tx(txcb);
  }
  return ret;
}

ErrorNo KVserver::put(TxCB *txcb, ulong key, ulong value){
  ErrorNo ret;
  ret = store->put(txcb, key, value);
  if(ret == TIMEOUT) {
     txManager->rollback_tx(txcb);
  }
  return ret;
}

ErrorNo KVserver::del(TxCB *txcb, ulong key){
  ErrorNo ret;
  ret = store->del(txcb, key);
  if(ret == TIMEOUT) {
      txManager->rollback_tx(txcb);
  }
  return ret;
}

ErrorNo KVserver::commit_tx(TxCB *txcb){
  return txManager->commit_tx(txcb);
}

ErrorNo KVserver::rollback_tx(TxCB *txcb) {
  return txManager->rollback_tx(txcb);
}


