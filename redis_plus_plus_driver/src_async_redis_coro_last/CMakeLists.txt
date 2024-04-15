cmake_minimum_required(VERSION 3.8)
#add_compile_options(-Wall -Wextra -Wpedantic)
add_compile_options(-Wall -std=c++20 -O3)
project(asnc_gRPC_test)

find_package(Threads REQUIRED)


#Coro lib
#find_package(cppcoro REQUIRED)
#set(_COROLIB cppcoro::cppcoro)
#message(STATUS "Using Coro library ${_COROLIB}")

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake)
include(CPM)
include(ProtobufGenerateGrpc)

find_package(Protobuf CONFIG REQUIRED)
find_package(gRPC CONFIG REQUIRED)
message(STATUS "Using gRPC ${gRPC_DIR} (Version ${gRPC_VERSION})")


#Redis libs
find_library(HIREDIS_LIB hiredis)
find_library(REDIS_PLUS_PLUS_LIB redis++)
find_package(Threads REQUIRED)

# Proto file
add_library(hw_grpc_proto OBJECT proto/CLS.proto )
target_include_directories(hw_grpc_proto PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(hw_grpc_proto PUBLIC gRPC::grpc++)
protobuf_generate_cpp_grpc(TARGET hw_grpc_proto)


# Include generated *.pb.h files
include_directories("${CMAKE_CURRENT_BINARY_DIR}")

# Targets greeter_[async_](client|server)
foreach(_target
  async_read async_create2
  #corolib_server 
  callback_server
  read_one 
  corostd_server
  corostd2_server
  grpc_callback_server
  grpc_callback_client )
  add_executable(${_target} "${_target}.cpp")
  target_link_libraries(${_target}
    hw_grpc_proto
    ${_GRPC_GRPCPP}
    ${_PROTOBUF_LIBPROTOBUF}
    ${HIREDIS_LIB}
    ${REDIS_PLUS_PLUS_LIB}
    ${_COROLIB})
endforeach()

