#ifndef TEST_SRC_CLI_CLIENT_GRPC_CLIENT_H_
#define TEST_SRC_CLI_CLIENT_GRPC_CLIENT_H_

#include <grpcpp/channel.h>
#include <grpcpp/client_context.h>
#include <grpcpp/create_channel.h>
#include <grpcpp/security/credentials.h>
#include <iostream>
#include <sstream>
#include <algorithm>
#include "../grpc/kv.grpc.pb.h"

using namespace txkv;
using grpc::Channel;
using txkv::MyKV;
using grpc::Status;
using grpc::ClientContext;
using txkv::ErrorReply;
using txkv::GetReply;
using txkv::ConnectionReply;

class TxKVClient {
 public:
  TxKVClient(std::shared_ptr<Channel> channel)
      : stub_(MyKV::NewStub(channel)) {}
 private:
  std::unique_ptr<MyKV::Stub> stub_;
  uint64_t cid = 0;

 public:
  uint64_t get_cid() {
    return cid;
  }

  bool Connect() {
    ClientContext context;
    google::protobuf::Empty dummy;
    ConnectionReply reply;
    Status status = stub_->Connect(&context, dummy, &reply);
    if(status.ok()) {
      cid = reply.cid();
      return true;
    }
    cid = 0; // reset
    return false;
  }

  bool Close() {
    ClientContext context;
    BaseRequest *req = new BaseRequest();
    req->set_cid(cid);
    ConnectionReply reply;
    Status status = stub_->Close(&context, *req, &reply);
    if(status.ok()) {
      cid = 0; // reset
      return true;
    }
    return false;
  }

  bool Begin() {
    ClientContext context;
    BaseRequest *req = new BaseRequest();
    req->set_cid(cid);
    ErrorReply reply;

    Status status = stub_->Begin(&context, *req, &reply);
    if(!status.ok()) {
      std::cout << "Request failed" << std::endl;
      std::cout << status.error_code() << ", " << status.error_details() << ", " << status.error_message() << std::endl;
      delete(req);
      return false;
    }
    delete(req);
    return true;
  }

  bool Commit() {
    ClientContext context;
    BaseRequest *req = new BaseRequest();
    req->set_cid(cid);
    ErrorReply reply;
    req->set_cid(cid);
    Status status = stub_->Commit(&context, *req, &reply);
    delete(req);
    if(!status.ok()) {
      std::cout << "grpc canceled!" << std::endl;
    }
    return true;
  }

  bool Rollback() {
    ClientContext context;
    BaseRequest *req = new BaseRequest();
    req->set_cid(cid);
    ErrorReply reply;
    req->set_cid(cid);
    Status status = stub_->Rollback(&context, *req, &reply);
    delete(req);
    if(!status.ok()) {
      std::cout << "grpc canceled!" << std::endl;
    }
    return false;
  }

  void Get(uint64_t key, GetReply *reply) {
    ClientContext context;
    KeyRequest *req = new KeyRequest();
    req->set_cid(cid);
    req->set_key(key);

    Status status = stub_->Get(&context, *req, reply);
    if(!status.ok()) {
      std::cout << "grpc canceled!" << std::endl;
    }
    return;
  }

  uint64_t Put(uint64_t key, uint64_t val) {
    ClientContext context;
    WriteRequest *req = new WriteRequest();
    req->set_cid(cid);
    req->set_key(key);
    req->set_val(val);
    ErrorReply reply;

    Status status = stub_->Put(&context, *req, &reply);
    if(!status.ok()) {
      std::cout << "grpc canceled!" << std::endl;
    }
    return true;
  }

  uint64_t Del(uint64_t key) {
    ClientContext context;
    KeyRequest *req = new KeyRequest();
    req->set_cid(cid);
    req->set_key(key);
    ErrorReply reply;

    Status status = stub_->Del(&context, *req, &reply);
    if(!status.ok()) {
      std::cout << "grpc canceled!" << std::endl;
    }
    return true;
  }
};

#endif//TEST_SRC_CLI_CLIENT_GRPC_CLIENT_H_
