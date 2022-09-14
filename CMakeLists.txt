cmake_minimum_required(VERSION 3.23)
project(tx_kv_server)

set(CMAKE_CXX_STANDARD 17)

set(ABSL_PROPAGATE_CXX_STD on)
include(FetchContent)
FetchContent_Declare(
  gRPC
  GIT_REPOSITORY https://github.com/grpc/grpc
  GIT_TAG        v1.48.1  # e.g v1.28.0
)
set(FETCHCONTENT_QUIET OFF)
FetchContent_MakeAvailable(gRPC)

add_executable(tx_kv_server src/main.cpp
        src/kv/KVserver.cpp src/kv/KVserver.h
        src/kv/KVclient.cpp src/kv/KVclient.h
        src/kv/TxManager.cpp src/kv/TxManager.h
        src/kv/common.h
        src/kv/MapStore.cpp src/kv/MapStore.h
        src/kv/LockDef.h
        src/kv/LockManager.cpp src/kv/LockManager.h
        src/grpc/kv.grpc.pb.cc src/grpc/kv.grpc.pb.h
        src/grpc/kv.pb.cc src/grpc/kv.pb.h
        src/server.h)

target_link_libraries(tx_kv_server PUBLIC "-pthread" grpc++)


# client

project(tx_kv_cli)

set(CMAKE_CXX_STANDARD 17)

set(ABSL_PROPAGATE_CXX_STD on)
include(FetchContent)
FetchContent_Declare(
  gRPC
  GIT_REPOSITORY https://github.com/grpc/grpc
  GIT_TAG        v1.48.1  # e.g v1.28.0
)
set(FETCHCONTENT_QUIET OFF)
FetchContent_MakeAvailable(gRPC)

add_executable(tx_kv_cli src/client.cpp
        # src/kv/KVserver.cpp src/kv/KVserver.h
        # src/kv/KVclient.cpp src/kv/KVclient.h
        # src/kv/TxManager.cpp src/kv/TxManager.h
        # src/kv/common.h
        # src/kv/MapStore.cpp src/kv/MapStore.h
        # src/kv/LockDef.h
        # src/kv/LockManager.cpp src/kv/LockManager.h
        src/grpc/kv.grpc.pb.cc src/grpc/kv.grpc.pb.h
        src/grpc/kv.pb.cc src/grpc/kv.pb.h
        )

target_link_libraries(tx_kv_cli PUBLIC "-pthread" grpc++)
