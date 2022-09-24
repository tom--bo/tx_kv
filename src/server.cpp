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
  reply->set_error_code(0);
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
    if(ret.error_no == KEY_NOT_FOUND) {
      reply->set_error_code(1); // TBD: key not found
    } else {
      reply->set_error_code(0);
      reply->set_val(ret.val);
    }
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
