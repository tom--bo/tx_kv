cmake_minimum_required(VERSION 3.23)
project(tx_kv)

set(CMAKE_CXX_STANDARD 17)

add_executable(tx_kv src/main.cpp src/KVserver.cpp src/KVserver.h src/KVclient.cpp src/KVclient.h src/TxManager.cpp src/TxManager.h src/common.h src/MapStore.cpp src/MapStore.h src/LockManager.cpp src/LockManager.h src/LockDef.h)
target_link_libraries(tx_kv PUBLIC "-pthread")
