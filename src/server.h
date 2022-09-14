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

class TxKVImpl final : public txkv::MyKV::Service {
 public:
  explicit TxKVImpl(){}
  ~TxKVImpl(){}

  Status Begin(ServerContext* ctx, const google::protobuf::Empty*, TxReply *reply) override {
    reply->set_tid(100);
    return Status::OK;
  }

  Status Commit(ServerContext* ctx, const TxRequest *req, google::protobuf::Empty* res) override {
    return Status::OK;
  }

  Status Rollback(ServerContext* ctx, const TxRequest *req, google::protobuf::Empty* res) override {
    return Status::OK;
  }

  Status Get(ServerContext* ctx, const KeyRequest *req, ValReply *reply) override {
    return Status::OK;
  }

  Status Put(ServerContext* ctx, const WriteRequest *wreq, google::protobuf::Empty* res) override {
    return Status::OK;
  }

  Status Del(ServerContext* ctx, const KeyRequest *req, google::protobuf::Empty* res) override {
    return Status::OK;
  }
};


#endif//TX_KV_SRC_SERVER_H_
