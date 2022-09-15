#include "kv/KVclient.h"
#include "kv/KVserver.h"
#include <iostream>
#include <pthread.h>
#include <unistd.h>
#include "server.h"

using namespace std;

KVserver *server;

Status MyKVImpl::Begin(ServerContext* ctx, const google::protobuf::Empty*, TxReply *reply) {
  TxCB *txcb = server->start_tx();
  TxMap[txcb->txid] = txcb;
  reply->set_tid(txcb->txid);
  return Status::OK;
}

Status MyKVImpl::Commit(ServerContext* ctx, const TxRequest *req, google::protobuf::Empty* res) {
  uint64_t txid = req->tid();
  // find a TxCB instance
  auto itr = TxMap.find(txid);
  if (itr != TxMap.end()) {
    TxCB *txcb = itr->second;
    server->commit_tx(txcb);
  } else {
    // TBD: error
  }
  return Status::OK;
}

Status MyKVImpl::Rollback(ServerContext* ctx, const TxRequest *req, google::protobuf::Empty* res) {
  // TBD: impl
  return Status::OK;
}

Status MyKVImpl::Get(ServerContext* ctx, const KeyRequest *req, ValReply *reply) {
  uint64_t txid = req->tid();
  uint64_t key = req->key();
  // find a TxCB instance
  auto itr = TxMap.find(txid);
  if (itr != TxMap.end()) {
    TxCB *txcb = itr->second;
    ReturnVal ret = server->get(txcb, key);
    reply->set_val(ret.val);
  } else {
    // TBD: error
  }
  return Status::OK;
}

Status MyKVImpl::Put(ServerContext* ctx, const WriteRequest *wreq, google::protobuf::Empty* res) {
  uint64_t txid = wreq->tid();
  // find a TxCB instance
  auto itr = TxMap.find(txid);
  if (itr != TxMap.end()) {
    TxCB *txcb = itr->second;
    server->put(txcb, wreq->key(), wreq->val());
  } else {
    // TBD: error
  }
  return Status::OK;
}

Status MyKVImpl::Del(ServerContext* ctx, const KeyRequest *req, google::protobuf::Empty* res) {
  uint64_t txid = req->tid();
  // find a TxCB instance
  auto itr = TxMap.find(txid);
  if (itr != TxMap.end()) {
    TxCB *txcb = itr->second;
    server->del(txcb, req->key());
  } else {
    // TBD: error
  }
  return Status::OK;
}


int main() {
  std::cout << "Server Start!" << std::endl;

  // initialize server
  server = new KVserver();
  server->db_init();

  // initialize grpc server
  std::unordered_map<uint64_t, TxCB *> txMap;
  string server_address("0.0.0.0:8000");
  MyKVImpl *txkvImpl = new MyKVImpl(txMap);
  ServerBuilder builder;
  builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
  builder.RegisterService(txkvImpl);
  std::unique_ptr<Server> server(builder.BuildAndStart());

  //  grpc server start
  std::cout << "Server listening on " << server_address << std::endl;
  server->Wait();
}

// ----------------
// main dummy below
// ----------------

void *th1(void *) {
  KVclient *c1 = new KVclient(server);
  c1->start_tx();
  c1->print_own_txid();

  c1->put(1, 10);
  sleep(2);
  c1->commit_tx();
  cout << "c1 committed" << endl;
  return nullptr;
}

void *th2(void *) {
  sleep(1);
  KVclient *c2 = new KVclient(server);
  c2->start_tx();
  c2->print_own_txid();

  auto ret = c2->get(1); // should not see val=10
  if(ret.error_no != NO_ERROR) {
    cout << "Hit some error, error_no = " << ret.error_no << endl;
    c2->rollback_tx();
  } else {
    cout << ret.val << endl;
  }

  c2->commit_tx();
  cout << "c2 committed" << endl;
  return nullptr;
}

int main_dummy() {
  std::cout << "Start!" << std::endl;

  server = new KVserver();
  server->db_init();

  cout << "start tx test ---" << endl;
  /*
   *   --- tx test ---
   * |   c1      |   c2             |
   * |           | sleep(1)         |
   * | begin     |                  |
   * | put(1,10) |                  |
   * | sleep(2)  |                  |
   * |           | begin            |
   * |           | get(1) *blocked* |
   * | commit    |                  |
   * |           | commit           |
   */

  pthread_t t1, t2;
  pthread_create(&t1, NULL, th1, NULL);
  pthread_create(&t2, NULL, th2, NULL);

  pthread_join(t1, NULL);
  pthread_join(t2, NULL);

  return 0;
}
