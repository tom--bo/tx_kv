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

struct Response {
  int code;
  std::string msg;
};

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

  Response Connect() {
    Response res;
    ClientContext context;
    google::protobuf::Empty dummy;
    ConnectionReply reply;
    Status status = stub_->Connect(&context, dummy, &reply);
    if(status.ok()) {
      cid = reply.cid();
      res.code = 0;
      return res;
    }
    cid = 0; // reset
    res.code = -1;
    res.msg = "grpc status is not ok";
    return res;
  }

  Response Close() {
    Response res;
    ClientContext context;
    BaseRequest *req = new BaseRequest();
    req->set_cid(cid);
    ConnectionReply reply;
    Status status = stub_->Close(&context, *req, &reply);
    if(status.ok()) {
      cid = 0; // reset
      res.code = 0;
      return res;
    }
    res.code = 1;
    res.msg = "grpc status is not ok";
    return res;
  }

  Response Begin() {
    Response res;
    ClientContext context;
    BaseRequest *req = new BaseRequest();
    req->set_cid(cid);
    ErrorReply reply;

    Status status = stub_->Begin(&context, *req, &reply);
    if(!status.ok()) {
      std::cout << "Request failed" << std::endl;
      std::cout << status.error_code() << ", " << status.error_details() << ", " << status.error_message() << std::endl;
      delete(req);
      res.code = 1;
      res.msg = "grpc status is not ok";
      return res;
    }
    delete(req);
    res.code = 0;
    return res;
  }

  Response Commit() {
    Response res;
    ClientContext context;
    BaseRequest *req = new BaseRequest();
    req->set_cid(cid);
    ErrorReply reply;
    req->set_cid(cid);
    Status status = stub_->Commit(&context, *req, &reply);
    delete(req);
    if(!status.ok()) {
      std::cout << "grpc canceled!" << std::endl;
      res.code = 1;
      res.msg = "grpc status is not ok";
      return res;
    }
    res.code = 0;
    return res;
  }

  Response Rollback() {
    Response res;
    ClientContext context;
    BaseRequest *req = new BaseRequest();
    req->set_cid(cid);
    ErrorReply reply;
    req->set_cid(cid);
    Status status = stub_->Rollback(&context, *req, &reply);
    delete(req);
    if(!status.ok()) {
      std::cout << "grpc canceled!" << std::endl;
      res.code = 1;
      res.msg = "grpc status is not ok";
      return res;
    }
    res.code = 0;
    return res;
  }

  Response Get(uint64_t key, GetReply *reply) {
    Response res;
    ClientContext context;
    KeyRequest *req = new KeyRequest();
    req->set_cid(cid);
    req->set_key(key);

    Status status = stub_->Get(&context, *req, reply);
    if(!status.ok()) {
      std::cout << "grpc canceled!" << std::endl;
      res.code = 1;
      res.msg = "grpc status is not ok";
      return res;
    }
    if(reply->error_code() == 2) { /* timeout */
      res.code = 2;
      res.msg = "timed out and rollbacked";
      return res;
    }
    res.code = 0;
    return res;
  }

  Response Put(uint64_t key, uint64_t val) {
    Response res;
    ClientContext context;
    WriteRequest *req = new WriteRequest();
    req->set_cid(cid);
    req->set_key(key);
    req->set_val(val);
    ErrorReply reply;

    Status status = stub_->Put(&context, *req, &reply);
    if(!status.ok()) {
      std::cout << "grpc canceled!" << std::endl;
      res.code = 1;
      res.msg = "grpc status is not ok";
      return res;
    }
    if(reply.error_code() == 2) { /* timeout */
      res.code = 2;
      res.msg = "timed out and rollbacked";
      return res;
    }
    res.code = 0;
    return res;
  }

  Response Del(uint64_t key) {
    Response res;
    ClientContext context;
    KeyRequest *req = new KeyRequest();
    req->set_cid(cid);
    req->set_key(key);
    ErrorReply reply;

    Status status = stub_->Del(&context, *req, &reply);
    if(!status.ok()) {
      std::cout << "grpc canceled!" << std::endl;
      res.code = 1;
      res.msg = "grpc status is not ok";
      return res;
    }
    if(reply.error_code() == 2) { /* timeout */
      res.code = 2;
      res.msg = "timed out and rollbacked";
      return res;
    }
    res.code = 0;
    return res;
  }
};

#endif//TEST_SRC_CLI_CLIENT_GRPC_CLIENT_H_
