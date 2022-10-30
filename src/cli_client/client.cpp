#include <iostream>

#include "../grpc/kv.grpc.pb.h"
#include "grpc_client.h"

using namespace std;

int main() {
  string in, tmp;
  Response ret;
  TxKVClient *cli = new TxKVClient(
      grpc::CreateChannel("127.0.0.1:8000", grpc::InsecureChannelCredentials())
  );
  Response res = cli->Connect();
  if(res.code != 0) {
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
      if(res.code != 0) {
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
      Response res = cli->Get(key, getReply);
      switch(res.code) {
        case 0:
          cout << getReply->val() << endl;
          break;
        case 1:
          cout << "Key Not Found" << endl;
          break;
        case 2:
          cout << res.msg << endl;
          break;
        default:
          cout << "Unknown Error Code" << endl;
          break;
      }
      delete(getReply);
    } else if(tmp == "put" || tmp == "p") {
      iss >> tmp; // key
      uint64_t key = stoull(tmp, nullptr, 10);
      iss >> tmp; // val
      uint64_t val = stoull(tmp, nullptr, 10);
      Response res = cli->Put(key, val);
      switch(res.code) {
        case 0:
          // do nothing
          break;
        case 2:
          cout << res.msg << endl;
          break;
        default:
          cout << "Unknown Error Code" << endl;
          break;
      }
    } else if(tmp == "del" || tmp == "d") {
      iss >> tmp;
      uint64_t key = stoull(tmp, nullptr, 10);
      Response res = cli->Del(key);
      switch(res.code) {
        case 0:
          // do nothing
          break;
        case 2:
          cout << res.msg << endl;
          break;
        default:
          cout << "Unknown Error Code" << endl;
          break;
      }
    } else if(tmp == "exit" || tmp == "e") {
      cli->Close();
      break;
    } else if(tmp == "help" || tmp == "h") {
      cout << "[command list]" << endl;
      cout << " - begin" << endl;
      cout << " - commit" << endl;
      cout << " - rolback (not implemented yet)" << endl;
      cout << " - get {key}" << endl;
      cout << " - put {key} {val}" << endl;
      cout << " - del {key}" << endl;
      cout << " - exit" << endl;
      cout << " - help" << endl;
      cout << "(Omit command is possible, use the first one character (ex) 'begin' => 'b')" << endl;
    } else {
      cout << "Bad command." << endl;
    }
  }
  return 0;
}

