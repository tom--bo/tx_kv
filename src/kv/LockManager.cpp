#include "LockManager.h"
#include "TxManager.h"

/* LockClass and timeout is not impl yet */
/* LockHash is not hashed but just map now */
LockReply LockManager::Lock(TxCB *me, ulong key, LockMode mode) {
  LockHead *head; /* *lock in original TP-book */
  LockRequest *request, *now, *prev;

  head = lockhash->FindLockHead(key);
  if(head == nullptr) {
    head = lockhash->CreateLockHead(key);
  }
  request = new LockRequest{nullptr, head, LOCK_GRANTED, mode, LOCK_FREE, me}; /* initialize at first */
  pthread_mutex_lock(&head->mu); /* lock 01 */
  if(head->queue == nullptr) { /* equivalent to L:11 in lock() in TP-book(jp) p570 */
    head->queue = request;
    head->granted_mode = mode;
    if(me->anchor == nullptr) {
      me->lock_head = request;
    } else {
      me->anchor->tran_next = request;
    }
    me->anchor = request;
    pthread_mutex_unlock(&head->mu); /* unlock 01 */
    return LOCK_OK; /* equivalent to L:17 in lock() in TP-book(jp) p570 */
  } else { // has locks anyway L:18
    now = head->queue;
    /* check lock-conversion, and lock for same key from own tx */
    do {
      if(now->txcb->txid == me->txid) {
        if(now->mode >= mode) {
          // already has strong lock_mode lock
          pthread_mutex_unlock(&head->mu); /* unlock 01 */
          delete(request);
          return LOCK_OK;
        } else {
          // request lock conversion
          // recalc granted_mode without own lock
          LockMode recalcGrantedMode = LOCK_FREE;
          LockRequest *tmpReq = head->queue;
          while(tmpReq != nullptr) {
            if(tmpReq->txcb != me) {
              recalcGrantedMode = GrantGroup(recalcGrantedMode, tmpReq->mode);
            }
            tmpReq = tmpReq->next;
          }
          if(LockCompat(mode, recalcGrantedMode)) { /* Requested lock-mode is compatible with recalculated granted-lock mode */
            now->mode = mode;
            head->granted_mode = GrantGroup(mode, head->granted_mode);  /* lock_max() in original */
            pthread_mutex_unlock(&head->mu); /* unlock 01 */
            delete(request);
            return LOCK_OK;
          } else {
            head->waiting = true;
            now->convert_mode = mode;
            delete(request);
            while(now->convert_mode != LOCK_FREE) {
              pthread_cond_wait(&head->cond, &head->mu);
            }
            pthread_mutex_unlock(&head->mu); /* unlock 01 */
            return LOCK_OK;
          }
        }
      }
      prev = now;
      now = now->next;
    } while(now != nullptr);
    /* End of checking lock-conversion, there is no lock request from same tx */
    now = prev; // reset for next step
    if(!head->waiting && LockCompat(mode, head->granted_mode)) { /* L:25 This key has no waiting thread and current lock-req is also granted */
      head->granted_mode = GrantGroup(mode, head->granted_mode); /* lock_max() in original */
      now->next = request;
      if(me->anchor == nullptr) {
        me->lock_head = request;
      } else {
        me->anchor->tran_next = request;
      }
      me->anchor = request;
      pthread_mutex_unlock(&head->mu); /* unlock 01 */
      return LOCK_OK; /* equivalent to L:28 in lock() in TP-book(jp) p570 */
    } else { /* L:30 This lock req should be queued because there is waiting thread or this lock-req is not compatible with granted lock mode */
      head->waiting = true;
      request->status = LOCK_WAITING;
      now->next = request;
      me->wait = request;
      while(request->status == LOCK_WAITING) {
        pthread_cond_wait(&head->cond, &head->mu);
      }
      me->wait = nullptr;
      if(me->anchor == nullptr) {
        me->lock_head = request;
      } else {
        me->anchor->tran_next = request;
      }
      me->anchor = request;
      pthread_mutex_unlock(&head->mu); /* unlock 01 */
      return LOCK_OK; /* equivalent to L:28 in lock() in TP-book(jp) p570 */
    }
  }
}

bool LockManager::Unlock(LockRequest *req) {
  LockRequest *now, *prev;
  TxCB *convertingTxCB = nullptr;
  LockHead *lockhead = req->head;
  bool hasConvertingLock = false;
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
      // The second and subsequent in lock_head->queue is 'req'
      now = lockhead->queue;
      while(now != nullptr && now != req) {
        prev = now;
        now = now->next;
      }
      if(now == nullptr) {
        // TBD: bug
        exit(2);
      }
      prev->next = now->next; /* delete req node from list */
    }
    /* Find converting-lock request only the first one */
    now = (lockhead->queue);
    while(now != nullptr && now->status == LOCK_GRANTED) {
      if(now->convert_mode != LOCK_FREE) {
        hasConvertingLock = true;
        convertingTxCB = now->txcb;
        break;
      }
      now = now->next;
    }

    /* re-calc granted_mode */
    lockhead->granted_mode = LOCK_FREE;
    now = (lockhead->queue);
    while(now != nullptr && now->status == LOCK_GRANTED) {
      if(now->txcb != convertingTxCB) {
        lockhead->granted_mode = GrantGroup(now->mode, lockhead->granted_mode);
      }
      now = now->next;
    }
    /* Skip this if converting-lock was not found */
    if(hasConvertingLock) {
      /* [1] Unlock waiting convert-lock according to granted_mode */
      // TBD: Fix this loop LOCK_GRANTED locks (for performance)
      now = (lockhead->queue);
      while(now != nullptr && now->status == LOCK_GRANTED) {
        if(now->convert_mode != LOCK_FREE
            && LockCompat(lockhead->granted_mode, now->convert_mode))
        {
          lockhead->granted_mode = GrantGroup(now->convert_mode, lockhead->granted_mode);
          now->mode = now->convert_mode;
          now->convert_mode = LOCK_FREE;
          break;
        }
        now = now->next;
      }
    } else {
      /* [2] Unlock other waiting locks according to granted_mode, this will be skipped if [1] was exist. */
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
  }
  pthread_mutex_unlock(&lockhead->mu); /* unlock 01 */
  pthread_cond_broadcast(&lockhead->cond);
  return true;
}

bool LockManager::UnlockAll(TxCB *txcb) {
  LockRequest *req = txcb->lock_head;
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
      1,1,1,1,1,1,1,
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
