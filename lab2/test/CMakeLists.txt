add_executable(test_calc test_calc.c)

target_link_libraries(test_calc PRIVATE calclib)

add_test(NAME test_calc COMMAND test_calc)
