cmake_minimum_required(VERSION 3.23)
project(netstack)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(netstack)
target_sources(netstack PRIVATE src/main.cpp src/tap.cpp src/ether.cpp
               src/arp.cpp src/netdevice.cpp src/ip.cpp
               PRIVATE FILE_SET HEADERS BASE_DIRS src FILES src/tap.hpp
               src/ether.hpp src/arp.hpp src/netdevice.hpp src/ip.hpp)

target_compile_options(netstack PRIVATE -Wall -Wextra -fsanitize=address,undefined)
target_link_options(netstack PRIVATE -fsanitize=address,undefined)

include(CTest)
if(BUILD_TESTING)
  include(FetchContent)
  FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.18.0.zip
  )
  # For Windows: Prevent overriding the parent project's compiler/linker settings
  set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
  FetchContent_MakeAvailable(googletest)

  add_executable(netstack_tests tests/ip_checksum.cpp src/ip.cpp)
  target_include_directories(netstack_tests PRIVATE src)
  target_link_libraries(netstack_tests PRIVATE GTest::gtest_main)
  # fno-sanitize-recover tells ubsan to fail if it found ub.
  target_compile_options(netstack_tests PRIVATE -fsanitize=address,undefined -fno-sanitize-recover=undefined)
  target_link_options(netstack_tests PRIVATE -fsanitize=address,undefined)

  include(GoogleTest)
  gtest_discover_tests(netstack_tests)
endif()
