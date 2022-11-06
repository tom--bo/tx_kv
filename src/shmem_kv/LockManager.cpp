#include "LockManager.h"
#include "TxManager.h"

/* LockClass and timeout is not impl yet */
/* use LockArray */
LockReply LockManager::Lock(TxCB *me, ulong key, LockMode mode) {
  LockHead *head; /* *lock in original TP-book */
  LockRequest *request, *now, *prev;
  int condwait_ret;
  struct timespec ts;

  head = &lock_array[key];
  if(head == nullptr) {
    // error
    return LOCK_NOT_LOCKED;
  }
  /* get LockRequest
  pthread_mutex_lock(lockRequest_mutex); /* lock 00 */

  pthread_mutex_unlock(lockRequest_mutex); /* unlock 00 */

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
            /* Actually this branch is unreachable now, because now txkv has onlyS and X */
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
              clock_gettime(CLOCK_MONOTONIC, &ts);
              ts.tv_sec += 5;
              condwait_ret = pthread_cond_timedwait(&head->cond, &head->mu, &ts); // lock-conversion wait
              if(condwait_ret == ETIMEDOUT) {
                  now->convert_mode = LOCK_FREE;
                  delete(request);
                  pthread_mutex_unlock(&head->mu);
                  return LOCK_TIMEOUT;
              } else if(condwait_ret != 0) {
                // TBD
                exit(1);
              }
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
        clock_gettime(CLOCK_MONOTONIC, &ts);
        ts.tv_sec += 5;
        condwait_ret = pthread_cond_timedwait(&head->cond, &head->mu, &ts);
        if(condwait_ret == ETIMEDOUT) {
          request->status = LOCK_TIMED_OUT; /* same process as if this thread can get lock */
          me->wait = nullptr;
          if(me->anchor == nullptr) {
            me->lock_head = request;
          } else {
            me->anchor->tran_next = request;
          }
          me->anchor = request;
          pthread_mutex_unlock(&head->mu); /* unlock 01 */
          return LOCK_TIMEOUT;
        } else if(condwait_ret != 0) {
          exit(1);
        }
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
    /* Currently do not release lockhead */
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
    /* Find converting-lock request, seek only the first one */
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
    /* Check whether this key has waiting lock */
    now = (lockhead->queue);
    while(now != nullptr) {
      // TBD: optimize this loop to eliminate
      if(now->status == LOCK_WAITING) {
        lockhead->waiting = true;
        break;
      }
      now = now->next;
    }
    if(now == nullptr) {
      lockhead->waiting = false;
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
