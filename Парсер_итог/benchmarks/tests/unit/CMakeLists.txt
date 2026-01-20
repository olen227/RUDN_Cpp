# Unit Tests CMakeLists.txt

set(UNIT_TEST_SOURCES
    test_lexer.cpp
    test_parser.cpp
    test_validator.cpp
    test_jsonvalue.cpp
)

# Создание исполняемого файла для unit тестов
add_executable(jsonparser_unit_tests ${UNIT_TEST_SOURCES})

# Линковка с библиотекой парсера и Google Test
target_link_libraries(jsonparser_unit_tests
    PRIVATE
    jsonparser_lib
    GTest::gtest
    GTest::gtest_main
    Threads::Threads
)

# Добавление тестов в CTest
include(GoogleTest)
gtest_discover_tests(jsonparser_unit_tests)
