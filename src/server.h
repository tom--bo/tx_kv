#ifndef TX_KV_SRC_SERVER_H_
#define TX_KV_SRC_SERVER_H_

#include <algorithm>
#include <chrono>
#include <cmath>
#include <iostream>
#include <memory>
#include <string>

#include <grpc/grpc.h>
#include <grpcpp/security/server_credentials.h>
#include <grpcpp/server.h>
#include <grpcpp/server_builder.h>
#include <grpcpp/server_context.h>
#include "grpc/kv.grpc.pb.h"
#include "kv/KVclient.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::ServerReader;
using grpc::ServerReaderWriter;
using grpc::ServerWriter;
using grpc::Status;
using txkv::TxRequest;
using txkv::TxReply;
using txkv::KeyRequest;
using txkv::ValReply;
using txkv::WriteRequest;

class MyKVImpl final : public txkv::MyKV::Service {
 private:
  std::unordered_map<uint64_t, TxCB *> TxMap;
 public:
  explicit MyKVImpl(std::unordered_map<uint64_t, TxCB *> txmap){
    TxMap = txmap;
  }
  ~MyKVImpl(){}

  Status Begin(ServerContext* ctx, const google::protobuf::Empty*, TxReply *reply) override;
  Status Commit(ServerContext* ctx, const TxRequest *req, google::protobuf::Empty* res) override;
  Status Rollback(ServerContext* ctx, const TxRequest *req, google::protobuf::Empty* res) override;
  Status Get(ServerContext* ctx, const KeyRequest *req, ValReply *reply) override;
  Status Put(ServerContext* ctx, const WriteRequest *wreq, google::protobuf::Empty* res) override;
  Status Del(ServerContext* ctx, const KeyRequest *req, google::protobuf::Empty* res) override;
};


#endif//TX_KV_SRC_SERVER_H_
