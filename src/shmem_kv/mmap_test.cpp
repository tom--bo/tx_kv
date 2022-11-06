#include "KVserver.h"
#include <algorithm>
#include <cstring>
#include <iostream>
#include <sstream>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>

#define IPC_ERROR (-1)

using namespace std;

typedef struct  {
  std::atomic<TxID> global_txid;
  char used[BIT_ARR_SIZE];
  ulong data[DATA_CAPACITY];
  LockHead lockarray[DATA_CAPACITY];

  pthread_mutex_t lockRequest_mutex;
  bool lockReqUsed[DATA_CAPACITY];
  LockRequest lockReqPool[LOCK_REQUEST_CAPACITY];

  pthread_mutex_t txcb_mutex;
  bool txcbUsed[TXCB_CAPACITY];
  TxCB txcbPool[TXCB_CAPACITY];
} shared_data;

int main(int argc, char *argv[]) {
  int shared_block_id;
  void *shared_ptr;
  string in, tmp;

  /* create, attach shared memory */
  key_t key;
  key = ftok("/tmp/tx_kv", 1);
  if(key == IPC_ERROR) {
    cout << "[Error] ftok" << endl;
    return 1;
  }
  shared_block_id = shmget(key, sizeof(shared_data), 0644 | IPC_CREAT);

  cout << "total size: " << sizeof(shared_data) / 1024 << " kib" << endl;
  cout << "used: " << sizeof(char)*BIT_ARR_SIZE << " byte" << endl;
  cout << "data: " << sizeof(ulong)*DATA_CAPACITY << " byte" << endl;
  cout << "lock_array: " << sizeof(LockHead)*DATA_CAPACITY << " byte" << endl;
  cout << "pthread_mutex: " << sizeof(pthread_mutex_t) << " byte" << endl;
  cout << "lockReqUsed: " << sizeof(bool) * 100 << " byte" << endl;
  cout << "LockRequest: " << sizeof(LockRequest) << " * " << DATA_CAPACITY << " byte" << endl;
  cout << "TxCB: " << sizeof(TxCB) << " * " << TXCB_CAPACITY << " byte" << endl;

  shared_ptr = shmat(shared_block_id, nullptr, 0);
  if(shared_ptr == (void*)IPC_ERROR) {
    cout << "[Error] shmat" << endl;
    return 1;
  }

  shared_data *d = (shared_data *)shared_ptr;

  // 引数が１つでもあればそれがinitialize
  if(argc > 1) {
    cout << "[Info] initializing shared memory..." << endl;

    // get shared_obj
    d->global_txid = 1;
    memset(d->used, 0, sizeof(char) * BIT_ARR_SIZE);
    memset(d->data, 0, sizeof(ulong) * DATA_CAPACITY);
    for(int i = 0; i < DATA_CAPACITY; i++) {
      new(&d->lockarray[i]) LockHead();
    }
    pthread_mutex_init(&d->lockRequest_mutex, nullptr);
    memset(d->lockReqUsed, 0, sizeof(bool) * DATA_CAPACITY);
    for(int i = 0; i < LOCK_REQUEST_CAPACITY; i++) {
      new(&d->lockReqPool[i]) LockRequest();
    }
    pthread_mutex_init(&d->txcb_mutex, nullptr);
    memset(d->txcbUsed, 0, sizeof(bool) * TXCB_CAPACITY);
    for(int i = 0; i < TXCB_CAPACITY; i++) {
      new(&d->txcbPool[i]) TxCB();
    }
  }


  LockManager *lm = new LockManager(d->lockarray, d->lockRequest_mutex, d->lockReqUsed, d->lockReqPool);
  ArrayStore *store = new ArrayStore(lm, d->used, d->data);
  TxManager *tm = new TxManager(store, &d->global_txid, &d->txcb_mutex, d->txcbUsed, d->txcbPool);
  KVserver *server = new KVserver(store, tm);

  std::cout << "Server start!" << std::endl;

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

  shmdt(shared_ptr);
  // shmctl(shared_ptr, IPC_RMID, nullptr);

  return 0;
}