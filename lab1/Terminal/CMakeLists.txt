# SPDX-FileCopyrightText: 2026 Magomed Gadzhiumarov
# SPDX-License-Identifier: GPL-3.0-or-later

add_library(terminal
    Terminal.cpp
    Terminal.h
)

target_link_libraries(program_a PUBLIC terminal)
target_link_libraries(program_b PUBLIC terminal)
target_include_directories(terminal PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}>
)
