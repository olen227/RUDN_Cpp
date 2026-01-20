# Integration Tests CMakeLists.txt

set(INTEGRATION_TEST_SOURCES
    test_large_files.cpp
    test_parallel_processing.cpp
)

# Создание исполняемого файла для интеграционных тестов
add_executable(jsonparser_integration_tests ${INTEGRATION_TEST_SOURCES})

# Линковка с библиотекой парсера и Google Test
target_link_libraries(jsonparser_integration_tests
    PRIVATE
    jsonparser_lib
    GTest::gtest
    GTest::gtest_main
    Threads::Threads
)

# Добавление тестов в CTest
include(GoogleTest)
gtest_discover_tests(jsonparser_integration_tests)
