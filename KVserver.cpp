#include "KVserver.h"

ulong KVserver::start_tx() {
  return txManager->start_tx();
}

ReturnVal KVserver::get(TxID txid, ulong key){
  return store->get(txid, key);
}

ErrorNo KVserver::put(TxID txid, ulong key, ulong value){
  return store->put(txid, key, value);
}

ErrorNo KVserver::del(TxID txid, ulong key){
  return store->del(txid, key);
}

ErrorNo KVserver::commit_tx(TxID txid){
  return txManager->commit_tx(txid);
}

ErrorNo KVserver::rollback_tx(TxID txid) {
  return txManager->rollback_tx(txid);
}


