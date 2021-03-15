set(TEST_NAME koral_test)

enable_testing()
FIND_PACKAGE(OpenCV REQUIRED)

add_executable(koral_test src/main.cpp)

add_dependencies(koral_test koral)

target_link_libraries(koral_test PRIVATE koral)
target_link_libraries(koral_test PRIVATE ${OpenCV_LIBS})
