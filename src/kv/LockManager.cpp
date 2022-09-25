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
  request = new LockRequest{nullptr, head, LOCK_GRANTED, mode, mode, me}; /* initialize at first */
  pthread_mutex_lock(&head->mu); /* lock here 01 */
  if(head->queue == nullptr) { /* equivalent to L:11 in lock() in TP-book(jp) p570 */
    head->queue = request;
    head->granted_mode = mode;
    if(me->anchor == nullptr) {
      me->listhead = request;
    } else {
      me->anchor->tran_next = request;
    }
    me->anchor = request;
    pthread_mutex_unlock(&head->mu); /* unlock here 01 */
    return LOCK_OK; /* equivalent to L:17 in lock() in TP-book(jp) p570 */
  } else { // has locks anyway L:18
    last = head->queue;
    while(last->next != nullptr) {
      /* skip checking lock-conversion, and lock for same key from 1 tx */
      last = last->next;
    }
    if(!head->waiting && LockCompat(mode, head->granted_mode)) { /* L:25 This key has no waiting thread and current lock-req is also granted */
      head->granted_mode = GrantGroup(mode, head->granted_mode); /* lock_max() in original */
      last->next = request;
      if(me->anchor == nullptr) {
        me->listhead = request;
      } else {
        me->anchor->tran_next = request;
      }
      me->anchor = request;
      pthread_mutex_unlock(&head->mu); /* unlock here 01 */
      return LOCK_OK; /* equivalent to L:28 in lock() in TP-book(jp) p570 */
    } else { /* L:30 This lock req should be queued because there is waiting thread or this lock-req is not compatible with granted lock mode */
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
      } else {
        me->anchor->tran_next = request;
      }
      me->anchor = request;
      pthread_mutex_unlock(&head->mu); /* unlock here 01 */
      return LOCK_OK; /* equivalent to L:28 in lock() in TP-book(jp) p570 */
    }
  }
}
bool LockManager::Unlock(LockRequest *req) {
  LockRequest *now, *priv;
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
      // The second and subsequent in lockhead->queue is 'req'
      now = lockhead->queue;
      while(now != nullptr && now != req) {
        priv = now;
        now = now->next;
      }
      if(now == nullptr) {
        // TBD: error
        exit(2);
      }
      priv->next = now->next; /* delete req node from list */
    }
    /* re-calc granted_mode */
    lockhead->granted_mode = LOCK_FREE;
    now = (lockhead->queue);
    while(now != nullptr && now->status == LOCK_GRANTED) {
      /* granted_mode is not updated correctly */
      lockhead->granted_mode = GrantGroup(now->mode, lockhead->granted_mode);
      now = now->next;
    }
    /* Unlock other locks according to granted_mode */
    while(now != nullptr && LockCompat(now->mode, lockhead->granted_mode)) {
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
  while (req != nullptr) {
    next = req->tran_next;
    Unlock(req);
    delete (req);
    req = next;
  }
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
