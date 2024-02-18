set(TEST_SOURCES test_main.c)

add_executable(tests ${TEST_SOURCES})

target_link_libraries(tests check)

target_include_directories(tests PRIVATE ${CMAKE_SOURCE_DIR}/src)
