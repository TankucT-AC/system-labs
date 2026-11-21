add_library(calclib STATIC
  calc.c
  calc.h
)

target_include_directories(calclib PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}>
)
