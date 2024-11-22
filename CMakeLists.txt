cmake_minimum_required(VERSION 3.31.0)
project(virtualfpu VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON) 

add_executable(virtualfpu main.cpp virtualfpu.cpp)

add_executable(vfpu_test ./tests/tests/tests.cpp virtualfpu.cpp)
target_include_directories(vfpu_test PRIVATE .)

include(CTest)
enable_testing()

# define tests
add_test(
  NAME vfpu_test
  COMMAND $<TARGET_FILE:vfpu_test>
  )