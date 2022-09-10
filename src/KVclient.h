#ifndef TX_KV__KVCLIENT_H_
#define TX_KV__KVCLIENT_H_

#include "common.h"
#include "KVserver.h"

class KVclient {
private:
  TxCB *txcb; // should be set to 0 when commit or rollback
  KVserver *srv;
public:
  void start_tx();
  ReturnVal get(ulong key);
  ErrorNo put(ulong key, ulong value);
  ErrorNo del(ulong key);
  ErrorNo commit_tx();
  ErrorNo rollback_tx();
  void print_own_txid();
  KVclient(KVserver *s) {
    srv = s;
  }
};

#endif//TX_KV__KVCLIENT_H_
