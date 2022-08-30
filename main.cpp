#include "KVserver.h"
#include "KVclient.h"
#include <iostream>

using namespace std;

int main() {
  std::cout << "Start!" << std::endl;

  KVserver *server = new KVserver();
  server->db_init();

  KVclient *c1 = new KVclient(server);
  c1->start_tx();
  c1->print_own_txid();

  KVclient *c2 = new KVclient(server);
  c2->start_tx();
  c2->print_own_txid();

  cout << "start tx test ---" << endl;
  /*
   *   --- tx test ---
   * |   c1      |   c2   |
   * | begin     |        |
   * | put(1,10) |        |
   * |           | begin  |
   * |           | get(1) |
   * | commit    |        |
   * |           | commit |
   */
  c1->start_tx();
  c1->put(1, 10);

  c2->start_tx();
  auto ret = c2->get(1); // should not see val=10
  if(ret.error_no != NO_ERROR) {
    cout << "Hit some error, error_no = " << ret.error_no << endl;
    c2->rollback_tx();
  } else {
    cout << ret.val << endl;
  }

  c1->commit_tx();
  c2->commit_tx();




  return 0;
}
