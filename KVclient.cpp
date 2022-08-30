#include "KVclient.h"
#include "KVserver.h"

void KVclient::start_tx() {
  own_txid = srv->start_tx();
}

ReturnVal KVclient::get(ulong key) {
  return srv->get(own_txid, key);
}

ErrorNo KVclient::put(ulong key, ulong value) {
  return srv->put(own_txid, key, value);
}

ErrorNo KVclient::del(ulong key) {
  return srv->del(own_txid, key);
}

ErrorNo KVclient::commit_tx() {
  return srv->commit_tx(own_txid);
}

ErrorNo KVclient::rollback_tx() {
  return srv->rollback_tx(own_txid);
}

void KVclient::print_own_txid() {
  std::cout << own_txid << std::endl;
}

