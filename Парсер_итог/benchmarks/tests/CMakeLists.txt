# Unit tests
add_subdirectory(unit)

# Integration tests
add_subdirectory(integration)

# Общая цель для запуска всех тестов
add_custom_target(run_all_tests
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure --verbose
    DEPENDS jsonparser_unit_tests jsonparser_integration_tests
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Running all tests..."
)
