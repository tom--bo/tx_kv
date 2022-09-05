#include "LockManager.h"
#include "TxManager.h"

/* LockClass and timeout are not impl yet */
/* LockHash is not hashed, so specify key directly now */
LockReply LockManager::Lock(TxCB *me, ulong key, LockMode mode) {
  LockHead *head; /* *lock in original TP-book */
  LockRequest *request, *last;

  head = lockhash->FindLockHead(key);
  if(head == nullptr) {
    head = lockhash->CreateLockHead(key);
  }
  request = new LockRequest{nullptr, head, LOCK_GRANTED, mode, mode, me->txid}; /* initialize at first */
  pthread_mutex_lock(&head->mu); /* lock here 01 */
  if(head->queue == nullptr) { /* equivalent to L:11 in lock() in TP-book(jp) p570 */
    head->queue = request;
    head->granted_mode = mode;
    if(me->anchor == nullptr) {
      me->listhead = request;
      me->anchor = request;
    } else {
      me->anchor->next = request;
      me->anchor = me->anchor->tran_next;
    }
    pthread_mutex_unlock(&head->mu); /* unlock here 01 */
    return LOCK_OK; /* equivalent to L:17 in lock() in TP-book(jp) p570 */
  } else { // has locks anyway L:18
    last = head->queue;
    while(last->next != nullptr) {
      /* skip checking lock-conversion, and lock for same key from 1 tx */
      last = last->next;
    }
    if(!head->waiting && LockCompat(mode, head->granted_mode)) { /* L:25 */
      head->granted_mode = GrantGroup(mode, head->granted_mode); /* lock_max() in original */
      if(me->anchor == nullptr) {
        me->listhead = request;
        me->anchor = request;
      } else {
        me->anchor->tran_next = request;
        me->anchor = me->anchor->tran_next;
      }
      pthread_mutex_unlock(&head->mu); /* unlock here 01 */
      return LOCK_OK; /* equivalent to L:28 in lock() in TP-book(jp) p570 */
    } else { /* L:30 */
      head->waiting = true;
      request->status = LOCK_WAITING;
      last->next = request;
      me->wait = request;
      while(request->status == LOCK_WAITING) {
        pthread_cond_wait(&head->cond, &head->mu);
      }
      me->wait = nullptr;
      if(me->anchor == nullptr) {
        me->listhead = request;
        me->anchor = request;
      } else {
        me->anchor->tran_next = request;
        me->anchor = me->anchor->tran_next;
      }
      pthread_mutex_unlock(&head->mu); /* unlock here 01 */
      return LOCK_OK; /* equivalent to L:28 in lock() in TP-book(jp) p570 */
    }
  }
}
bool LockManager::Unlock(LockRequest *req) {
  LockRequest *now, *next;
  LockHead *lockhead = req->head;
  pthread_mutex_lock(&lockhead->mu); /* lock 01 */
  if(lockhead->queue == req && req->next == nullptr) { /* L:19 in UnLock in TP-book(jp) p572 */
    /* In my impl, do not release lockheader */
    lockhead->queue = nullptr;
    lockhead->waiting = false;
    lockhead->granted_mode = LOCK_FREE;
  } else {
    if(lockhead->queue == req) {
      lockhead->queue = req->next; /* delete req node from list */
    } else {
      now = lockhead->queue;
      next = now->next;
      while(next != req) {
        now = next;
        next = now->next;
      }
      now->next = next->next; /* delete req node from list */
    }
    lockhead->granted_mode = LOCK_FREE; /* re-calc granted_mode */
    now = lockhead->queue;
    while(now->status == LOCK_GRANTED) {
      /* granted_mode is not updated correctly */
      lockhead->granted_mode = GrantGroup(now->mode, lockhead->granted_mode);
      now = now->next;
    }
    while(LockCompat(now->mode, lockhead->granted_mode)) {
      /* granted_mode is not updated correctly */
      lockhead->granted_mode = GrantGroup(now->mode, lockhead->granted_mode);
      now->status = LOCK_GRANTED;
      if(now->next == nullptr) {
        lockhead->waiting = false;
        break;
      } else {
        now = now->next;
      }
    }
  }
  pthread_mutex_unlock(&lockhead->mu); /* unlock 01 */
  pthread_cond_broadcast(&lockhead->cond);
  return true;
}
bool LockManager::UnlockAll(TxCB *txcb) {
  LockRequest *req = txcb->listhead;
  LockRequest *next;
  do {
    next = req->tran_next;
    Unlock(req);
    delete(req);
    req = next;
  } while(next != nullptr);
  return true;
}
/* TP-book(jp) p494 */
bool LockManager::LockCompat(LockMode requested, LockMode granted) {
  if(requested > 6 || granted > 6) {
    return 0; /* more than 6 (LOCK_WAIT) is error actually */
  }
  bool compatibility_matrix[7][7] {
      0,0,0,0,0,0,0, /* LOCK_FREE is error actually */
      1,1,1,1,1,0,0,
      1,1,1,0,0,0,0,
      1,1,0,1,0,0,0,
      1,1,0,0,0,0,0,
      1,0,0,1,0,0,0,
      1,0,0,0,0,0,0
  };
  return compatibility_matrix[requested][granted];
}
LockMode LockManager::GrantGroup(LockMode requested, LockMode granted) { /* lock_max() in original */
  LockMode max = (requested > granted)? requested: granted;
  return max;
}
