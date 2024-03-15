cmake_minimum_required(VERSION 3.8)
project(corotest_test)
#add_compile_options(-Wall -Wextra -Wpedantic -std=c++20)
add_compile_options( --std=c++20)

find_package(Threads REQUIRED)
FIND_PACKAGE(CURL)
INCLUDE_DIRECTORIES(${CURL_INCLUDE_DIR})


# Targets greeter_[async_](client|server)
foreach(_target
  curl_client
  coro_example
  coro_curl_client)
  add_executable(${_target} "${_target}.cpp")
  target_link_libraries(${_target}
  ${CURL_LIBRARIES}
      )
endforeach()

