#include "MapStore.h"
#include "LockManager.h"
#include "TxManager.h"
#include "undo.h"

void AddUndoRecord(TxCB *txcb, UndoRecord *u) {
  if(txcb->undo_head == nullptr) {
    txcb->undo_head = u;
  } else {
    txcb->undo_anchor->next = u;
  }
  txcb->undo_anchor = u;
  return;
}

ReturnVal MapStore::get(TxCB *txcb, ulong key) {
  lock_manager->Lock(txcb, key, LOCK_S);
  auto itr = store.find(key);
  if (itr != store.end()) {
    // found
    return ReturnVal {itr->second, NO_ERROR};
  }
  return ReturnVal {0, KEY_NOT_FOUND};
}

ErrorNo MapStore::put(TxCB *txcb, ulong key, ulong value) {
  lock_manager->Lock(txcb, key, LOCK_X);
  // prepare undo
  UndoRecord *undo = new UndoRecord();
  undo->key = key;
  undo->op_type = PUT;
  auto itr = store.find(key);
  if (itr != store.end()) {
    undo->is_null = false;
    undo->val_before = itr->second;
  } else {
    undo->is_null = true;
  }
  undo->val_after = value;
  AddUndoRecord(txcb, undo);

  // modify store data
  store[key] = value;
  return NO_ERROR;
}

ErrorNo MapStore::del(TxCB *txcb, ulong key) {
  lock_manager->Lock(txcb, key, LOCK_X);
  // prepare undo
  UndoRecord *undo = new UndoRecord();
  undo->key = key;
  undo->op_type = DEL;
  auto itr = store.find(key);
  if (itr != store.end()) {
    undo->is_null = false;
    undo->val_before = itr->second;
    // modify store data
    store.erase(itr);
  } else {
    undo->is_null = true;
  }
  AddUndoRecord(txcb, undo);
  return NO_ERROR;
}

ErrorNo MapStore::commit_tx(TxCB *txcb) {
  lock_manager->UnlockAll(txcb);
  return NO_ERROR;
}

ErrorNo MapStore::rollback_tx(TxCB *txcb) {
  UndoRecord *u = txcb->undo_head;
  while(u != nullptr) {
    if(u->is_null) {
      store.erase(u->key);
    } else {
      store[u->key] = u->val_before;
    }
    u = u->next;
  }

  lock_manager->UnlockAll(txcb);
  return NO_ERROR;
}

