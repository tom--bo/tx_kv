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
using txkv::ErrorReply;
using txkv::GetReply;
using txkv::ConnectionReply;

class TxKVClient {
 public:
  TxKVClient(std::shared_ptr<Channel> channel)
      : stub_(MyKV::NewStub(channel)) {}
 private:
  unique_ptr<MyKV::Stub> stub_;
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

  bool Begin() {
    ClientContext context;
    BaseRequest *req = new BaseRequest();
    req->set_cid(cid);
    ErrorReply reply;

    Status status = stub_->Begin(&context, *req, &reply);
    if(!status.ok()) {
      cout << "Request failed" << endl;
      cout << status.error_code() << ", " << status.error_details() << ", " << status.error_message() << endl;
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
      cout << "grpc canceled!" << endl;
    }
  }

  bool Rollback() {
    cout << "rollback is not implemented yet" << endl;
    return false;
  }

  void Get(uint64_t key, GetReply *reply) {
    ClientContext context;
    KeyRequest *req = new KeyRequest();
    req->set_cid(cid);
    req->set_key(key);

    Status status = stub_->Get(&context, *req, reply);
    if(!status.ok()) {
      cout << "grpc canceled!" << endl;
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
      cout << "grpc canceled!" << endl;
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
      cout << "grpc canceled!" << endl;
    }
    return true;
  }
};

int main() {
  string in, tmp;
  bool ret;
  TxKVClient *cli = new TxKVClient(
      grpc::CreateChannel("127.0.0.1:8000", grpc::InsecureChannelCredentials())
  );
  bool ok = cli->Connect();
  if(!ok) {
    cout << "Connection failed." << endl;
    exit(1);
  }
  cout << "connection_id: " << cli->get_cid() << endl;

  // command input loop
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
      GetReply *getReply = new GetReply();
      iss >> tmp;
      uint64_t key = stoull(tmp, nullptr, 10);
      cli->Get(key, getReply);
      if(getReply->error_code() == 0) {
        cout << getReply->val() << endl;
      } else if(getReply->error_code() == 1) {
        cout << "Key Not Found" << endl;
      } else {
        cout << "Unknown Error Code" << endl;
      }
      delete(getReply);
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

