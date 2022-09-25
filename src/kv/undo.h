#ifndef TEST_SRC_KV_UNDO_H_
#define TEST_SRC_KV_UNDO_H_

#include "common.h"

typedef enum {
  PUT,
  DEL
} OperationType;

typedef struct _UndoRecord {
  OperationType op_type;
  bool is_null = false;
  ulong key;
  ulong val_before;
  ulong val_after;
  _UndoRecord *next;
} UndoRecord;

#endif//TEST_SRC_KV_UNDO_H_
