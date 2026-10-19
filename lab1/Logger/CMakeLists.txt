# SPDX-FileCopyrightText: 2026 Magomed Gadzhiumarov
# SPDX-License-Identifier: GPL-3.0-or-later

add_library(logger
    Logger.h
)

set_target_properties(logger PROPERTIES LINKER_LANGUAGE CXX)
target_include_directories(logger PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}>
)
