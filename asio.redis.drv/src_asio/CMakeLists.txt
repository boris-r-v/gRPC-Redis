cmake_minimum_required(VERSION 3.8)
project(asio_gRPC_Redis_test)

find_package(Threads REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(Boost 1.84.0 REQUIRED COMPONENTS headers system )

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



# Targets greeter_[async_](client|server)
foreach(_target
  sync_server )
  add_executable(${_target} "${_target}.cpp")
  target_link_libraries(${_target}
    boost_compiled
    ${Boost_LIBRARIES}
    ssl
    crypto)
endforeach()

