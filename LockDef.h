#ifndef TX_KV__LOCKDEF_H_
#define TX_KV__LOCKDEF_H_

#include "common.h"
#include <pthread.h>

typedef enum {
  LOCK_FREE,
  LOCK_S,
  LOCK_X,
  LOCK_U,
  LOCK_IS,
  LOCK_IX,
  LOCK_SIX,
  LOCK_WAIT
} LockMode;

typedef enum {
  LOCK_GRANTED,
  LOCK_CONVERTING,
  LOCK_WAITING,
  LOCK_DENIED
} LockStatus;

typedef enum {
  LOCK_OK,
  LOCK_TIMEOUT,
  LOCK_DEADLOCK,
  LOCK_NOT_LOCKED
} LockReply;

typedef struct _LockRequest{
  _LockRequest *next;  /* 'queue' in TP-book */
  // LockHead *head;  /* need not maybe */
  LockStatus status;
  LockMode mode;      /* requested mode */
  LockMode convert_mode;
  // int count;       /* need not maybe */
  // LockClass class; /* not impl yet */
  // PCB process;     /* not impl yet*/
  TxID txid; /* Instead of 'TransCB tran;' in TP-book */
  // LockRequest *tran_prev; /* not impl yet */
  // LockRequest *tran_next; /* not impl yet */
} LockRequest;

/* TBD: Need LockHead-pool */
struct LockHead {
 public:
  pthread_mutex_t mu;
  // LockHead *chain; /* Need this after impl hash-chain */
  // char *lock_name; /* Need this after impl hash-chain */
  LockRequest *queue;
  LockMode granted_mode;
  bool waiting;
  LockHead() {
    pthread_mutex_init(&mu, NULL);
    queue = nullptr;
    granted_mode = LOCK_FREE;
    waiting = false;
  }
};

#endif//TX_KV__LOCKDEF_H_