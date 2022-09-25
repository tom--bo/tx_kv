#include "kv/KVserver.h"
#include <iostream>
#include <pthread.h>
#include <unistd.h>
#include "grpc_server.h"

using namespace std;

KVserver *server;

void *th1(void *) {
  KVclient *c1 = new KVclient(server);
  c1->start_tx();
  c1->print_own_txid();

  c1->put(1, 10);
  sleep(2);
  c1->commit_tx();
  cout << "c1 committed" << endl;
  return nullptr;
}

void *th2(void *) {
  sleep(1);
  KVclient *c2 = new KVclient(server);
  c2->start_tx();
  c2->print_own_txid();

  auto ret = c2->get(1); // should not see val=10
  if(ret.error_no != NO_ERROR) {
    cout << "Hit some error, error_no = " << ret.error_no << endl;
    c2->rollback_tx();
  } else {
    cout << ret.val << endl;
  }

  c2->commit_tx();
  cout << "c2 committed" << endl;
  return nullptr;
}

int main() {
  std::cout << "Start!" << std::endl;

  server = new KVserver();
  server->db_init();

  cout << "start tx test ---" << endl;
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

  pthread_t t1, t2;
  pthread_create(&t1, NULL, th1, NULL);
  pthread_create(&t2, NULL, th2, NULL);

  pthread_join(t1, NULL);
  pthread_join(t2, NULL);

  return 0;
}