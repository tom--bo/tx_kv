project(server)
cmake_minimum_required(VERSION 3.19)
set(CMAKE_CXX_STANDARD 17)

# set(BOOST_ENABLE_CMAKE on)
# set(BoostRoot ${PROJECT_SOURCE_DIR}/clion-cmake/_deps/boost-src)

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
    src/kv/KVserver.cpp
    src/kv/KVclient.cpp
    src/kv/TxManager.cpp
    src/kv/MapStore.cpp
    src/kv/LockManager.cpp
    src/grpc/kv.grpc.pb.cc
    src/grpc/kv.pb.cc
)

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
    src/grpc/kv.grpc.pb.cc
    src/grpc/kv.pb.cc
    src/mmap_kv/mmap_test.cpp
)

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
    src/kv/KVserver.cpp
    src/kv/KVclient.cpp
    src/kv/TxManager.cpp
    src/kv/MapStore.cpp
    src/kv/LockDef.h
    src/kv/LockManager.cpp
    src/grpc/kv.grpc.pb.cc
    src/grpc/kv.pb.cc
    src/server_test.cpp
)

target_link_libraries(test PUBLIC "-pthread" grpc++)

################
# mmap_kv
################

# project(mmap_kv)
#
# cmake_minimum_required(VERSION 3.19)
# set(CMAKE_CXX_STANDARD 17)
#
# set(ABSL_PROPAGATE_CXX_STD on)
#
# add_executable(mmap_kv
#     src/mmap_kv/KVserver.cpp
#     src/mmap_kv/TxManager.cpp
#     src/mmap_kv/MapStore.cpp
#     src/mmap_kv/LockManager.cpp
#     src/mmap_kv/mmap_test.cpp
# )
#
# target_link_libraries(mmap_kv PUBLIC "-pthread")
