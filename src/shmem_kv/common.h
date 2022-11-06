#ifndef TX_KV__COMMON_H_
#define TX_KV__COMMON_H_

#include <atomic>
#include <csignal>

#define DATA_CAPACITY 100
#define BIT_ARR_SIZE 13 /* DATA_CAPACITY/8+1  */
#define LOCK_REQUEST_CAPACITY 100
#define TXCB_CAPACITY 20

typedef unsigned long int ulong;
typedef unsigned short int ushort;
typedef unsigned long int TxID;

typedef enum {
  NO_ERROR,
  KEY_NOT_FOUND,
  TIMEOUT,
} ErrorNo;

typedef struct {
  ulong val;
  ErrorNo error_no;
} ReturnVal;

#endif//TX_KV__COMMON_H_
