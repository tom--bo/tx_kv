#include "KVserver.h"
#include <algorithm>
#include <iostream>
#include <sstream>
#include <unistd.h>

using namespace std;

KVserver *server;

int main() {
  string in, tmp;

  std::cout << "Test Start!" << std::endl;

  server = new KVserver();
  server->db_init();

  /*
  TxCB *txcb = server->start_tx();
  server->put(txcb, 1, 10);
  ReturnVal ret = server->get(txcb, 1);
  if(ret.error_no != NO_ERROR) {
    cout << "failed to get key" << endl;
    return 1;
  }
  cout << ret.val << endl;

  cout << "End." << endl;
  if(case1()) {
    cout << "[Fail] case1" << endl;
  } else {
    cout << "[Success] case1" << endl;
  }
  */

  TxCB *txcb;
  ErrorNo err;

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
      txcb = server->start_tx();
      if(txcb == nullptr) {
        cout << "[Error] failed to start" << endl;
        break;
      }
    } else if(tmp == "commit" || tmp == "c") {
      err = server->commit_tx(txcb);
      if(err != NO_ERROR) {
        cout << "[Error] failed to commit, error_no: " << err << endl;
      }
    } else if(tmp == "rollback" || tmp == "r") {
      err = server->rollback_tx(txcb);
      if(err != NO_ERROR) {
        cout << "[Error] failed to rollback, error_no: " << err << endl;
      }
    } else if(tmp == "get" || tmp == "g") {
      ReturnVal res;
      iss >> tmp;
      uint64_t key = stoull(tmp, nullptr, 10);
      res = server->get(txcb, key);
      switch(res.error_no) {
        case 0:
          cout << res.val << endl;
          break;
        case 1:
          cout << "[Error] Key Not Found" << endl;
          break;
        case 2:
          cout << "[Error] Lock Timeout" << endl;
          break;
        default:
          cout << "[Error] Unknown Error Code" << endl;
          break;
      }
    } else if(tmp == "put" || tmp == "p") {
      iss >> tmp; // key
      uint64_t key = stoull(tmp, nullptr, 10);
      iss >> tmp; // val
      uint64_t val = stoull(tmp, nullptr, 10);
      err = server->put(txcb, key, val);
      switch(err) {
        case 0:
          // do nothing
          break;
        case 2:
          cout << "[Error] Lock Timeout" << endl;
          break;
        default:
          cout << "[Error] Unknown Error Code" << endl;
          break;
      }
    } else if(tmp == "del" || tmp == "d") {
      iss >> tmp;
      uint64_t key = stoull(tmp, nullptr, 10);
      err = server->del(txcb, key);
      switch(err) {
        case 0:
          // do nothing
          break;
        case 2:
          cout << "[Error] Lock Timeout" << endl;
          break;
        default:
          cout << "[Error] Unknown Error Code" << endl;
          break;
      }
    } else if(tmp == "exit" || tmp == "e") {
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