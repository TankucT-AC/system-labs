# SPDX-FileCopyrightText: 2026 Magomed Gadzhiumarov
# SPDX-License-Identifier: GPL-3.0-or-later

cmake_minimum_required(VERSION 3.20)
project(system_lab1)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_subdirectory(program_a)
add_subdirectory(program_b)
add_subdirectory(program_c)
add_subdirectory(Terminal)
add_subdirectory(Logger)
add_subdirectory(utils)