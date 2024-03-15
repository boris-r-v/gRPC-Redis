cmake_minimum_required(VERSION 3.8)
project(asio_gRPC_Redis_test)
#add_compile_options(-Wall -Wextra -Wpedantic -std=c++20)
add_compile_options( --std=c++20)

find_package(Threads REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(Boost 1.84.0 REQUIRED COMPONENTS headers system coroutine)

IF(Boost_FOUND)
  INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})
  LINK_DIRECTORIES(${Boost_LIBRARY_DIRS})
ENDIF(Boost_FOUND)
message(STATUS "Boost include dir: ${Boost_INCLUDE_DIRS} ")
message(STATUS "Boost library dir: ${Boost_LIBRARY_DIRS} ")
message(STATUS "Link with folowing boost libs: ${Boost_LIBRARIES} ")

add_library(
    boost_compiled
    STATIC
    boost_asio.cpp
    boost_redis.cpp
)
target_compile_definitions(
    boost_compiled
    PUBLIC
    BOOST_ASIO_SEPARATE_COMPILATION
)
find_package(Protobuf REQUIRED)
set(PROTOBUF_LIB protobuf::libprotobuf)
message(STATUS "Protobuf version ${Protobuf_VERSION}")

find_package(PkgConfig REQUIRED)
pkg_search_module(GRPC REQUIRED grpc)
pkg_search_module(GRPCPP REQUIRED grpc++>=1.22.0)
list(APPEND GRPC_GRPCPP ${GRPCPP_LIBRARIES})
message(STATUS "Using gRPC libs: ${GRPC_GRPCPP} ")
find_program(PROTOBUF_PROTOC protoc)
find_program(GRPC_CPP_PLUGIN_EXECUTABLE grpc_cpp_plugin)
# Proto file
get_filename_component(hw_proto "../../protos/CustomerLimitStorageRPC.proto" ABSOLUTE)
get_filename_component(hw_proto_path "${hw_proto}" PATH)
message(STATUS "Use Proto file: ${hw_proto_path} ")
# Generated sources
set(hw_proto_srcs "${CMAKE_CURRENT_BINARY_DIR}/CustomerLimitStorageRPC.pb.cc")
set(hw_proto_hdrs "${CMAKE_CURRENT_BINARY_DIR}/CustomerLimitStorageRPC.pb.h")
set(hw_grpc_srcs "${CMAKE_CURRENT_BINARY_DIR}/CustomerLimitStorageRPC.grpc.pb.cc")
set(hw_grpc_hdrs "${CMAKE_CURRENT_BINARY_DIR}/CustomerLimitStorageRPC.grpc.pb.h")
add_custom_command(
      OUTPUT "${hw_proto_srcs}" "${hw_proto_hdrs}" "${hw_grpc_srcs}" "${hw_grpc_hdrs}"
      COMMAND ${PROTOBUF_PROTOC}
      ARGS --grpc_out "${CMAKE_CURRENT_BINARY_DIR}"
        --cpp_out "${CMAKE_CURRENT_BINARY_DIR}"
        -I "${hw_proto_path}"
        --plugin=protoc-gen-grpc="${GRPC_CPP_PLUGIN_EXECUTABLE}"
        "${hw_proto}"
      DEPENDS "${hw_proto}")
# Include generated *.pb.h files
include_directories("${CMAKE_CURRENT_BINARY_DIR}")
add_library(hw_grpc_proto STATIC
  ${hw_grpc_srcs}
  ${hw_grpc_hdrs}
  ${hw_proto_srcs}
  ${hw_proto_hdrs}  )

target_link_libraries(hw_grpc_proto
  ${GRPC_GRPCPP}
  ${PROTOBUF_LIB})

#Redis libs
find_library(HIREDIS_LIB hiredis)
find_library(REDIS_PLUS_PLUS_LIB redis++)


# Targets greeter_[async_](client|server)
foreach(_target
  sync_server
  boost_spawn 
  callback_server)
  add_executable(${_target} "${_target}.cpp")
  target_link_libraries(${_target}
    boost_compiled
    hw_grpc_proto
    ssl
    crypto    
    ${Boost_LIBRARIES}
    ${GRPC_GRPCPP}
    ${PROTOBUF_LIB} 
    ${HIREDIS_LIB}
    ${REDIS_PLUS_PLUS_LIB}
      )
endforeach()

