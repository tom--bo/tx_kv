#include "kv/KVserver.h"
#include <iostream>
#include "server.h"

using namespace std;

KVserver *server;

Status MyKVImpl::Connect(ServerContext* ctx, const google::protobuf::Empty*, ConnectionReply *reply) {
  ReturnVal ret = server->connect();
  if(ret.error_no == NO_ERROR) {
    reply->set_cid(ret.val);
    return Status::OK;
  }
  return Status::CANCELLED;
}

Status MyKVImpl::Begin(ServerContext* ctx, const BaseRequest *req, ErrorReply *reply) {
  uint64_t cid = req->cid();
  auto itr = ConnMap.find(cid);
  if (itr != ConnMap.end()) {
    // TBD: must implicit commit and release TxCB
    cout << "[DEBUG] implicit commit" << endl;
    TxCB *txcb = itr->second;
    server->commit_tx(txcb);
  }
  TxCB *txcb = server->start_tx();
  ConnMap[req->cid()] = txcb;
  reply->set_error_code(0);
  return Status::OK;
}

Status MyKVImpl::Commit(ServerContext* ctx, const BaseRequest *req, ErrorReply *reply) {
  uint64_t cid = req->cid();
  auto itr = ConnMap.find(cid);
  if (itr != ConnMap.end()) {
    TxCB *txcb = itr->second;
    server->commit_tx(txcb);
  } else {
    // TBD: error
    return Status::CANCELLED;
  }
  ConnMap.erase(cid); // TBD: should delete all txcb-related instances
  return Status::OK;
}

Status MyKVImpl::Rollback(ServerContext* ctx, const BaseRequest *req, ErrorReply *reply) {
  uint64_t cid = req->cid();
  auto itr = ConnMap.find(cid);
  if (itr != ConnMap.end()) {
    TxCB *txcb = itr->second;
    // TBD: impl rollback process
  } else {
    // TBD: error
    return Status::CANCELLED;
  }
  ConnMap.erase(cid); // TBD: should delete all txcb-related instances
  return Status::OK;
}

Status MyKVImpl::Get(ServerContext* ctx, const KeyRequest *req, GetReply *reply) {
  uint64_t cid = req->cid();
  uint64_t key = req->key();
  TxCB *txcb;
  bool singleSTMT = false;
  // find a TxCB instance
  auto itr = ConnMap.find(cid);
  if (itr != ConnMap.end()) {
    txcb = itr->second;
  } else {
    // start transaction for this stmt
    singleSTMT = true;
    txcb = server->start_tx();
  }
  ReturnVal ret = server->get(txcb, key);
  if(ret.error_no == KEY_NOT_FOUND) {
    reply->set_error_code(1); // TBD: key not found
  } else {
    reply->set_error_code(0);
    reply->set_val(ret.val);
  }
  if (singleSTMT) {
    server->commit_tx(txcb);
  }
  return Status::OK;
}

Status MyKVImpl::Put(ServerContext* ctx, const WriteRequest *wreq, ErrorReply *reply) {
  uint64_t cid = wreq->cid();
  bool singleSTMT = false;
  TxCB *txcb;
  // find a TxCB instance
  auto itr = ConnMap.find(cid);
  if (itr != ConnMap.end()) {
    txcb = itr->second;
  } else {
    // start transaction for this stmt
    singleSTMT = true;
    txcb = server->start_tx();
  }
  server->put(txcb, wreq->key(), wreq->val());
  if(singleSTMT) {
    server->commit_tx(txcb);
  }
  return Status::OK;
}

Status MyKVImpl::Del(ServerContext* ctx, const KeyRequest *req, ErrorReply *reply) {
  uint64_t cid = req->cid();
  bool singleSTMT = false;
  TxCB *txcb;
  // find a TxCB instance
  auto itr = ConnMap.find(cid);
  if (itr != ConnMap.end()) {
    txcb = itr->second;
  } else {
    singleSTMT = true;
    txcb = server->start_tx();
  }
  server->del(txcb, req->key());
  if(singleSTMT) {
    server->commit_tx(txcb);
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
