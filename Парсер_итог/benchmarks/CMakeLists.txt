# Benchmarks CMakeLists.txt

# Создание исполняемого файла для бенчмарков
add_executable(parser_benchmark
    benchmark_main.cpp
    benchmark_parser.cpp
    benchmark_memory.cpp
)

# Линковка с библиотекой парсера
target_link_libraries(parser_benchmark
    PRIVATE
    jsonparser_lib
    Threads::Threads
)

# Добавление цели для запуска бенчмарков
add_custom_target(run_benchmarks
    COMMAND $<TARGET_FILE:parser_benchmark>
    DEPENDS parser_benchmark
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Running benchmarks..."
)
