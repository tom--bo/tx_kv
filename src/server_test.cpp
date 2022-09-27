#include "kv/KVserver.h"
#include <iostream>
#include <pthread.h>
#include <unistd.h>
#include "grpc_server.h"

using namespace std;

KVserver *server;

void *th1(void *r) {
  int *ip = (int *)r;
  KVclient *c1 = new KVclient(server);
  c1->start_tx();
  // c1->print_own_txid();

  c1->put(1, 10);
  sleep(2);
  ErrorNo en = c1->commit_tx();
  if(en == NO_ERROR) {
    *ip = 0;
  } else {
    *ip = 1;
  }

  // cout << "c1 committed" << endl;
  return nullptr;
}

void *th2(void *r) {
  int *ip = (int *)r;
  sleep(1);
  KVclient *c2 = new KVclient(server);
  c2->start_tx();
  // c2->print_own_txid();

  auto ret = c2->get(1); // should not see val=10
  if(ret.error_no != NO_ERROR) {
    cout << "Hit some error, error_no = " << ret.error_no << endl;
    *ip = 1;
    c2->rollback_tx();
  } else {
    // cout << ret.val << endl;
    if(ret.val != 10) {
      *ip = 1;
    } else {
      *ip = 0;
    }
  }

  c2->commit_tx();
  // cout << "c2 committed" << endl;
  return nullptr;
}

int case1() {
  /*
  *   --- tx test ---
  * |   c1      |   c2             |
  * |           | sleep(1)         |
  * | begin     |                  |
  * | put(1,10) |                  |
  * | sleep(2)  |                  |
  * |           | begin            |
  * |           | get(1) *blocked* |
  * | commit    |                  |
  * |           | commit           |
  */
  int hasErr1 = 0;
  int hasErr2 = 0;
  pthread_t t1, t2;
  pthread_create(&t1, NULL, th1, &hasErr1);
  pthread_create(&t2, NULL, th2, &hasErr2);

  pthread_join(t1, NULL);
  pthread_join(t2, NULL);

  if(hasErr1 || hasErr2) {
    return 1;
  }
  return 0;
}

int main() {
  std::cout << "Test Start!" << std::endl;

  server = new KVserver();
  server->db_init();

  if(case1()) {
    cout << "[Fail] case1" << endl;
  } else {
    cout << "[Success] case1" << endl;
  }

  return 0;
}