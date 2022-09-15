#include <grpcpp/channel.h>
#include <grpcpp/client_context.h>
#include <grpcpp/create_channel.h>
#include <grpcpp/security/credentials.h>
#include <iostream>
#include <sstream>
#include <algorithm>

#include "grpc/kv.grpc.pb.h"

using namespace std;
using namespace txkv;
using grpc::Channel;
using txkv::MyKV;
using grpc::Status;
using grpc::ClientContext;
using txkv::TxReply;


class TxKVClient {
 public:
  TxKVClient(std::shared_ptr<Channel> channel)
      : stub_(MyKV::NewStub(channel)) {}
 private:
  unique_ptr<MyKV::Stub> stub_;
  uint64_t tid = 0;

 public:
  bool Begin() {
    TxReply txReply;
    ClientContext context;
    google::protobuf::Empty dummy;

    Status status = stub_->Begin(&context, dummy, &txReply);
    if(!status.ok()) {
      cout << "Request failed" << endl;
      cout << status.error_code() << ", " << status.error_details() << ", " << status.error_message() << endl;
      return false;
    } else {
      tid = txReply.tid();
    }
    return true;
  }

  bool Commit() {
    ClientContext context;
    TxRequest *req = new TxRequest();
    req->set_tid(tid);
    google::protobuf::Empty dummy;
    Status status = stub_->Commit(&context, *req, &dummy);
  }

  bool Rollback() {
    cout << "rollback is not implemented yet" << endl;
    return false;
  }

  void Get(uint64_t key, ValReply *val) {
    ClientContext context;
    KeyRequest *req = new KeyRequest();
    req->set_tid(tid);
    req->set_key(key);
    Status status = stub_->Get(&context, *req, val);
    delete(req);
    return;
  }

  uint64_t Put(uint64_t key, uint64_t val) {
    ClientContext context;
    WriteRequest *req = new WriteRequest();
    req->set_tid(tid);
    req->set_key(key);
    req->set_val(val);
    google::protobuf::Empty dummy;
    Status status = stub_->Put(&context, *req, &dummy);
    return true;
  }

  uint64_t Del(uint64_t key) {
    ClientContext context;
    KeyRequest *req = new KeyRequest();
    req->set_tid(tid);
    req->set_key(key);
    google::protobuf::Empty dummy;
    Status status = stub_->Del(&context, *req, &dummy);
    return true;
  }
};

int main() {
  string in, tmp;
  bool ret;
  TxKVClient *cli = new TxKVClient(
      grpc::CreateChannel("127.0.0.1:8000", grpc::InsecureChannelCredentials())
  );

  while(true) {
    cout << "cmd> ";
    // read one line
    getline(cin, in);
    if(in.empty()) {
      continue;
    }

    // split token
    istringstream iss(in);
    iss >> tmp;
    transform(tmp.begin(), tmp.end(), tmp.begin(), ::tolower);

    // switch by cmd
    if(tmp == "begin") {
      ret = cli->Begin();
      if(!ret) {
        break;
      }
    } else if(tmp == "commit") {
      cli->Commit();
    } else if(tmp == "rollback") {
      cout << "rollback" << endl;
      cli->Rollback();
    } else if(tmp == "get") {
      ValReply *valReply = new ValReply();
      iss >> tmp;
      uint64_t key = stoull(tmp, nullptr, 10);
      cli->Get(key, valReply);
      if(valReply->error_code() == 0) {
        cout << valReply->val() << endl;
      } else if(valReply->error_code() == 1) {
        cout << "Key Not Found" << endl;
      } else {
        cout << "Unknown Error Code" << endl;
      }
      delete(valReply);
    } else if(tmp == "put") {
      iss >> tmp; // key
      uint64_t key = stoull(tmp, nullptr, 10);
      iss >> tmp; // val
      uint64_t val = stoull(tmp, nullptr, 10);
      cli->Put(key, val);
    } else if(tmp == "del") {
      iss >> tmp;
      uint64_t key = stoull(tmp, nullptr, 10);
      cli->Del(key);
    } else if(tmp == "help") {
      cout << "[command list]" << endl;
      cout << " begin" << endl;
      cout << " commit" << endl;
      cout << " rolback (not implemented yet)" << endl;
      cout << " get {key}" << endl;
      cout << " put {key} {val}" << endl;
      cout << " del {key}" << endl;
      cout << " help" << endl;
    } else {
      cout << "Bad command." << endl;
    }
  }
  return 0;
}

