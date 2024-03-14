cmake_minimum_required(VERSION 3.8)
project(asnc_gRPC_test)

find_package(Threads REQUIRED)

option(protobuf_MODULE_COMPATIBLE TRUE)
find_package(Protobuf REQUIRED)
set(_PROTOBUF_LIBPROTOBUF protobuf::libprotobuf)
message(STATUS "Using protobuf ${Protobuf_VERSION}")

find_package(PkgConfig REQUIRED)
pkg_search_module(GRPC REQUIRED grpc)
pkg_search_module(GRPCPP REQUIRED grpc++>=1.22.0)
list(APPEND _GRPC_GRPCPP ${GRPCPP_LIBRARIES})
message(STATUS "Using gRPC libs: ${_GRPC_GRPCPP} ")

find_program(_PROTOBUF_PROTOC protoc)
find_program(_GRPC_CPP_PLUGIN_EXECUTABLE grpc_cpp_plugin)

# Proto file
get_filename_component(hw_proto "../../test1/protos/CustomerLimitStorageRPC.proto" ABSOLUTE)
get_filename_component(hw_proto_path "${hw_proto}" PATH)

# Generated sources
set(hw_proto_srcs "${CMAKE_CURRENT_BINARY_DIR}/CustomerLimitStorageRPC.pb.cc")
set(hw_proto_hdrs "${CMAKE_CURRENT_BINARY_DIR}/CustomerLimitStorageRPC.pb.h")
set(hw_grpc_srcs "${CMAKE_CURRENT_BINARY_DIR}/CustomerLimitStorageRPC.grpc.pb.cc")
set(hw_grpc_hdrs "${CMAKE_CURRENT_BINARY_DIR}/CustomerLimitStorageRPC.grpc.pb.h")
add_custom_command(
      OUTPUT "${hw_proto_srcs}" "${hw_proto_hdrs}" "${hw_grpc_srcs}" "${hw_grpc_hdrs}"
      COMMAND ${_PROTOBUF_PROTOC}
      ARGS --grpc_out "${CMAKE_CURRENT_BINARY_DIR}"
        --cpp_out "${CMAKE_CURRENT_BINARY_DIR}"
        -I "${hw_proto_path}"
        --plugin=protoc-gen-grpc="${_GRPC_CPP_PLUGIN_EXECUTABLE}"
        "${hw_proto}"
      DEPENDS "${hw_proto}")

# Include generated *.pb.h files
include_directories("${CMAKE_CURRENT_BINARY_DIR}")

# hw_grpc_proto
add_library(hw_grpc_proto
  ${hw_grpc_srcs}
  ${hw_grpc_hdrs}
  ${hw_proto_srcs}
  ${hw_proto_hdrs})
target_link_libraries(hw_grpc_proto
  ${_GRPC_GRPCPP}
  ${_PROTOBUF_LIBPROTOBUF})

# Targets greeter_[async_](client|server)
foreach(_target
  async_server )
  add_executable(${_target} "${_target}.cpp")
  target_link_libraries(${_target}
    hw_grpc_proto
    ${_GRPC_GRPCPP}
    ${_PROTOBUF_LIBPROTOBUF})
endforeach()

