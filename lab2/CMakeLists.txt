cmake_minimum_required(VERSION 3.20)
project(system_lab2)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_C_EXTENSIONS OFF)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_subdirectory(src)

add_executable(calc_app main.c)
target_link_libraries(calc_app PRIVATE
  calclib
)

enable_testing()
add_subdirectory(test)
