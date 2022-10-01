#include <iostream>

#include "../grpc/kv.grpc.pb.h"
#include "grpc_client.h"

using namespace std;

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
    if(cin.eof()) {
      break;
    } else if(in.empty()) {
      continue;
    }

    // split token
    istringstream iss(in);
    iss >> tmp;
    transform(tmp.begin(), tmp.end(), tmp.begin(), ::tolower);

    // switch by cmd
    if(tmp == "begin" || tmp == "b") {
      ret = cli->Begin();
      if(!ret) {
        break;
      }
    } else if(tmp == "commit" || tmp == "c") {
      cli->Commit();
    } else if(tmp == "rollback" || tmp == "r") {
      cli->Rollback();
    } else if(tmp == "get" || tmp == "g") {
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
    } else if(tmp == "put" || tmp == "p") {
      iss >> tmp; // key
      uint64_t key = stoull(tmp, nullptr, 10);
      iss >> tmp; // val
      uint64_t val = stoull(tmp, nullptr, 10);
      cli->Put(key, val);
    } else if(tmp == "del" || tmp == "d") {
      iss >> tmp;
      uint64_t key = stoull(tmp, nullptr, 10);
      cli->Del(key);
    } else if(tmp == "help" || tmp == "h") {
      cout << "[command list]" << endl;
      cout << " - begin" << endl;
      cout << " - commit" << endl;
      cout << " - rolback (not implemented yet)" << endl;
      cout << " - get {key}" << endl;
      cout << " - put {key} {val}" << endl;
      cout << " - del {key}" << endl;
      cout << " - help" << endl;
      cout << "(Omit command is possible, use the first one character (ex) 'begin' => 'b')" << endl;
    } else if(tmp == "close") {
      cli->Close();
      break;
    } else {
      cout << "Bad command." << endl;
    }
  }
  return 0;
}

