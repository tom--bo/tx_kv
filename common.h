#ifndef TX_KV__COMMON_H_
#define TX_KV__COMMON_H_

typedef unsigned long int ulong;
typedef unsigned short int ushort;
typedef unsigned long int TxID;


typedef enum {
  NO_ERROR,
  KEY_NOT_FOUND,
} ErrorNo;

typedef struct {
  ulong val;
  ErrorNo error_no;
} ReturnVal;

#endif//TX_KV__COMMON_H_
