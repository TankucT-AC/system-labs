# SPDX-FileCopyrightText: 2026 Magomed Gadzhiumarov
# SPDX-License-Identifier: GPL-3.0-or-later

add_library(utils
    utils.cpp
    utils.h
)

target_link_libraries(program_a PUBLIC utils)
target_link_libraries(program_b PUBLIC utils)
target_link_libraries(program_c PUBLIC utils)
target_include_directories(utils PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}>
)
