#include <iostream>
#include <grpcpp/channel.h>
#include <grpcpp/client_context.h>
#include <grpcpp/create_channel.h>
#include <grpcpp/security/credentials.h>

#include "grpc/kv.grpc.pb.h"

using namespace std;
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

 public:
  void Begin() {
    TxReply txReply;
    ClientContext context;
    google::protobuf::Empty dummy;

    Status status = stub_->Begin(&context, dummy, &txReply);
    if(!status.ok()) {
      cout << "Request failed" << endl;
      cout << status.error_code() << ", " << status.error_details() << ", " << status.error_message() << endl;
    } else {
      cout << "ok" << endl;
      cout << "Succeeded, txid = " << txReply.tid() << endl;
    }
  }
};


int main() {
  TxKVClient cli(
      grpc::CreateChannel("127.0.0.1:50051",
                          grpc::InsecureChannelCredentials())
  );

  std::cout << "-------------- begin --------------" << std::endl;
  cli.Begin();

  return 0;
}

