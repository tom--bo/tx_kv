project(server)
cmake_minimum_required(VERSION 3.19)
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

add_executable(server src/grpc_server.cpp
        src/kv/KVserver.cpp src/kv/KVserver.h
        src/kv/KVclient.cpp src/kv/KVclient.h
        src/kv/TxManager.cpp src/kv/TxManager.h
        src/kv/common.h
        src/kv/MapStore.cpp src/kv/MapStore.h
        src/kv/LockDef.h
        src/kv/LockManager.cpp src/kv/LockManager.h
        src/grpc/kv.grpc.pb.cc src/grpc/kv.grpc.pb.h
        src/grpc/kv.pb.cc src/grpc/kv.pb.h
        src/grpc_server.h src/kv/undo.h)

target_link_libraries(server PUBLIC "-pthread" grpc++)


################
# client
################

project(cli)
cmake_minimum_required(VERSION 3.19)

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

add_executable(cli src/cli_client/client.cpp
        src/grpc/kv.grpc.pb.cc src/grpc/kv.grpc.pb.h
        src/grpc/kv.pb.cc src/grpc/kv.pb.h src/cli_client/grpc_client.h)

target_link_libraries(cli PUBLIC "-pthread" grpc++)

################
# test
################

project(test)

cmake_minimum_required(VERSION 3.19)
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

add_executable(test
        src/kv/KVserver.cpp src/kv/KVserver.h
        src/kv/KVclient.cpp src/kv/KVclient.h
        src/kv/TxManager.cpp src/kv/TxManager.h
        src/kv/common.h
        src/kv/MapStore.cpp src/kv/MapStore.h
        src/kv/LockDef.h
        src/kv/LockManager.cpp src/kv/LockManager.h
        src/grpc/kv.grpc.pb.cc src/grpc/kv.grpc.pb.h
        src/grpc/kv.pb.cc src/grpc/kv.pb.h
        src/grpc_server.h
        src/server_test.cpp)

target_link_libraries(test PUBLIC "-pthread" grpc++)


