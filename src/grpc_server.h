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
using txkv::BaseRequest;
using txkv::ConnectionReply;
using txkv::ErrorReply;
using txkv::KeyRequest;
using txkv::GetReply;
using txkv::WriteRequest;

class TPMonitor final : public txkv::MyKV::Service {
 private:
  std::unordered_map<uint64_t, TxCB *> ConnMap;
 public:
  explicit TPMonitor(std::unordered_map<uint64_t, TxCB *> cmap){
    ConnMap = cmap;
  }
  ~TPMonitor(){}

  Status Connect(ServerContext* ctx, const google::protobuf::Empty*, ConnectionReply *reply) override;
  Status Begin(ServerContext* ctx, const BaseRequest *req, ErrorReply *reply) override;
  Status Commit(ServerContext* ctx, const BaseRequest *req, ErrorReply *reply) override;
  Status Rollback(ServerContext* ctx, const BaseRequest *req, ErrorReply *reply) override;
  Status Get(ServerContext* ctx, const KeyRequest *req, GetReply *reply) override;
  Status Put(ServerContext* ctx, const WriteRequest *wreq, ErrorReply *reply) override;
  Status Del(ServerContext* ctx, const KeyRequest *req, ErrorReply *reply) override;
  Status Close(ServerContext* ctx, const BaseRequest *req, ConnectionReply *reply) override;
};


#endif//TX_KV_SRC_SERVER_H_
