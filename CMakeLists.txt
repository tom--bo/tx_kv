cmake_minimum_required(VERSION 3.23)
project(tx_kv)

set(CMAKE_CXX_STANDARD 17)

add_executable(tx_kv main.cpp KVserver.cpp KVserver.h KVclient.cpp KVclient.h TxManager.cpp TxManager.h common.h MapStore.cpp MapStore.h LockManager.cpp LockManager.h LockDef.h)
target_link_libraries(tx_kv PUBLIC "-pthread")
