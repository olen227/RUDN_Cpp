cmake_minimum_required(VERSION 3.14)
project(JsonParser VERSION 1.0 LANGUAGES CXX)

# Требуем C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Опции проекта
option(BUILD_TESTS "Build unit and integration tests" ON)
option(BUILD_BENCHMARKS "Build performance benchmarks" ON)
option(ENABLE_PROFILING "Enable profiling instrumentation" OFF)

# Добавляем флаги компиляции
if(MSVC)
    add_compile_options(/W4 /utf-8)
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /O2")
else()
    add_compile_options(-Wall -Wextra -pedantic)
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -DNDEBUG")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -O0")
endif()

# Поддержка многопоточности
find_package(Threads REQUIRED)

# ============================================================================
# Библиотека JSON парсера
# ============================================================================

set(PARSER_SOURCES
    src/Lexer.cpp
    src/Parser.cpp
    src/JsonValue.cpp
    src/Serializer.cpp
    src/Generator.cpp
    src/Validator.cpp
    src/ParallelProcessor.cpp
)

set(PARSER_HEADERS
    include/JsonValue.hpp
    include/Lexer.hpp
    include/Parser.hpp
    include/Serializer.hpp
    include/Generator.hpp
    include/Validator.hpp
    include/ParallelProcessor.hpp
    include/SystemInfo.hpp
    include/ProgressBar.hpp
)

# Создание статической библиотеки для переиспользования в тестах
add_library(jsonparser_lib STATIC ${PARSER_SOURCES} ${PARSER_HEADERS})
target_include_directories(jsonparser_lib PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(jsonparser_lib PUBLIC Threads::Threads)

# Включение профилирования если требуется
if(ENABLE_PROFILING)
    target_compile_definitions(jsonparser_lib PUBLIC ENABLE_PROFILING)
    if(NOT MSVC)
        target_compile_options(jsonparser_lib PUBLIC -pg)
        target_link_options(jsonparser_lib PUBLIC -pg)
    endif()
endif()

# ============================================================================
# Основной исполняемый файл
# ============================================================================

add_executable(jsonparser src/main.cpp)
target_link_libraries(jsonparser PRIVATE jsonparser_lib)

# ============================================================================
# Google Test
# ============================================================================

if(BUILD_TESTS)
    # Загрузка Google Test через FetchContent
    include(FetchContent)
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.14.0
    )
    # Для Windows: предотвращение переопределения родительских настроек компилятора
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)

    # Включение тестирования
    enable_testing()

    # Unit и интеграционные тесты
    add_subdirectory(tests)
endif()

# ============================================================================
# Benchmarks
# ============================================================================

if(BUILD_BENCHMARKS)
    add_subdirectory(benchmarks)
endif()

# ============================================================================
# Информация о сборке
# ============================================================================

message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Build tests: ${BUILD_TESTS}")
message(STATUS "Build benchmarks: ${BUILD_BENCHMARKS}")
message(STATUS "Enable profiling: ${ENABLE_PROFILING}")
