
### test utilities library ###

set(STATIC_TEST_DATA_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/test_data/)
set(GENERATED_TEST_DATA_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/test_data/)
file(MAKE_DIRECTORY ${GENERATED_TEST_DATA_DIRECTORY})


add_subdirectory(test_utilities)

