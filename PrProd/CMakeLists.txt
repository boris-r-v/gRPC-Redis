cmake_minimum_required(VERSION 3.8)
#add_compile_options(-Wall -Wextra -Wpedantic)
add_compile_options(-Wall -std=c++23 )
project(PreProd_CLS)

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

#Redis libs
find_library(HIREDIS_LIB hiredis)
find_library(REDIS_PLUS_PLUS_LIB redis++)

# Proto file
get_filename_component(hw_proto "proto/CLS.proto" ABSOLUTE)
get_filename_component(hw_proto_path "${hw_proto}" PATH)

# Generated sources
set(hw_proto_srcs "${CMAKE_CURRENT_BINARY_DIR}/CLS.pb.cc")
set(hw_proto_hdrs "${CMAKE_CURRENT_BINARY_DIR}/CLS.pb.h")
set(hw_grpc_srcs "${CMAKE_CURRENT_BINARY_DIR}/CLS.grpc.pb.cc")
set(hw_grpc_hdrs "${CMAKE_CURRENT_BINARY_DIR}/CLS.grpc.pb.h")
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
include_directories("cls_core")
include_directories("cls_bl")

# hw_grpc_proto
add_library(hw_grpc_proto
  ${hw_grpc_srcs}
  ${hw_grpc_hdrs}
  ${hw_proto_srcs}
  ${hw_proto_hdrs})
target_link_libraries(hw_grpc_proto
  ${_GRPC_GRPCPP}
  ${_PROTOBUF_LIBPROTOBUF})

file(GLOB sources "cls_core/*.h" "cls_core/*.cpp" "cls_core/*.hpp" "main.cpp" "cls_bl/*.h" "cls_bl/*.cpp")

# Targets greeter_[async_](client|server)
add_executable(cls_server ${sources})
target_link_libraries(cls_server
    hw_grpc_proto
    ${_GRPC_GRPCPP}
    ${_PROTOBUF_LIBPROTOBUF}
    ${HIREDIS_LIB}
    ${REDIS_PLUS_PLUS_LIB} )


foreach(_target
    set_sync_one
    set_async
   )
  add_executable(${_target} "${_target}.cpp")
  target_link_libraries(${_target}
  hw_grpc_proto
  ${_GRPC_GRPCPP}
  ${_PROTOBUF_LIBPROTOBUF}
  ${HIREDIS_LIB}
  ${REDIS_PLUS_PLUS_LIB} )
endforeach()


