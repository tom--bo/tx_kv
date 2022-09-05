#include "KVclient.h"
#include "KVserver.h"

void KVclient::start_tx() {
  txcb = srv->start_tx();
}

ReturnVal KVclient::get(ulong key) {
  return srv->get(txcb, key);
}

ErrorNo KVclient::put(ulong key, ulong value) {
  return srv->put(txcb, key, value);
}

ErrorNo KVclient::del(ulong key) {
  return srv->del(txcb, key);
}

ErrorNo KVclient::commit_tx() {
  return srv->commit_tx(txcb);
}

ErrorNo KVclient::rollback_tx() {
  return srv->rollback_tx(txcb);
}

void KVclient::print_own_txid() {
  std::cout << "My txid: " << txcb->txid << std::endl;
}

