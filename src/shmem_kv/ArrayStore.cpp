#include "ArrayStore.h"
#include "LockManager.h"
#include "TxManager.h"
#include "undo.h"

bool ArrayStore::is_used(ulong key) {
  int div = key / 8;
  int mod_minus = key % 8 -1;
  return used[div] && (1 << mod_minus);
}

void ArrayStore::set_used(ulong key) {
  int div = key / 8;
  int mod_minus = key % 8 -1;
  used[div] |= (1 << mod_minus);
  return;
}

void ArrayStore::set_not_used(ulong key) {
  int div = key / 8;
  int mod_minus = key % 8 -1;
  used[div] &= ~(1 << mod_minus);
  return;
}


void AddUndoRecord(TxCB *txcb, UndoRecord *u) {
  if(txcb->undo_head == nullptr) {
    txcb->undo_head = u;
  } else {
    u->next = txcb->undo_head;
    txcb->undo_head = u;
  }
  return;
}

ReturnVal ArrayStore::get(TxCB *txcb, ulong key) {
  LockReply reply = lock_manager->Lock(txcb, key, LOCK_S);
  if(reply != LOCK_OK) {
      return ReturnVal{0, TIMEOUT};
  }
  if (is_used(key)) {
    // found
    return ReturnVal{data[key], NO_ERROR};
  }
  return ReturnVal{0, KEY_NOT_FOUND};
}


ErrorNo ArrayStore::put(TxCB *txcb, ulong key, ulong value) {
  LockReply reply = lock_manager->Lock(txcb, key, LOCK_X);
  if(reply != LOCK_OK) {
      return TIMEOUT;
  }
  // prepare undo
  UndoRecord *undo = new UndoRecord();
  undo->key = key;
  undo->op_type = PUT;
  if (is_used(key)) {
    undo->is_null = false;
    undo->val_before = data[key];
  } else {
    undo->is_null = true;
  }
  undo->val_after = value;
  AddUndoRecord(txcb, undo);

  // modify store data
  data[key] = value;
  set_used(key);
  return NO_ERROR;
}

ErrorNo ArrayStore::del(TxCB *txcb, ulong key) {
  LockReply reply = lock_manager->Lock(txcb, key, LOCK_X);
  if(reply != LOCK_OK) {
      return TIMEOUT;
  }
  // prepare undo
  UndoRecord *undo = new UndoRecord();
  undo->key = key;
  undo->op_type = DEL;
  if (is_used(key)) {
    undo->is_null = false;
    undo->val_before = data[key];
    // modify store data
    set_not_used(key);
  } else {
    undo->is_null = true;
  }
  AddUndoRecord(txcb, undo);
  return NO_ERROR;
}

ErrorNo ArrayStore::commit_tx(TxCB *txcb) {
  lock_manager->UnlockAll(txcb);
  return NO_ERROR;
}

ErrorNo ArrayStore::rollback_tx(TxCB *txcb) {
  UndoRecord *u = txcb->undo_head;
  while(u != nullptr) {
    if(u->is_null) {
      set_not_used(u->key);
    } else {
      data[u->key] = u->val_before;
      set_used(u->key);
    }
    u = u->next;
  }

  lock_manager->UnlockAll(txcb);
  return NO_ERROR;
}

