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


set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake)
include(ProtobufGenerateGrpc)

find_package(Protobuf CONFIG REQUIRED)
find_package(gRPC CONFIG REQUIRED)
message(STATUS "Using gRPC ${gRPC_DIR} (Version ${gRPC_VERSION})")


#Redis libs
find_library(HIREDIS_LIB hiredis)
find_library(REDIS_PLUS_PLUS_LIB redis++)
find_package(Threads REQUIRED)

# Proto file
add_library(hw_grpc_proto OBJECT proto/CLS.proto proto/CLS_srv.proto)
target_include_directories(hw_grpc_proto PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(hw_grpc_proto PUBLIC gRPC::grpc++)
protobuf_generate_cpp_grpc(TARGET hw_grpc_proto)

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
    ${HIREDIS_LIB}
    ${REDIS_PLUS_PLUS_LIB}
     gRPC::grpc++
      )
endforeach()

