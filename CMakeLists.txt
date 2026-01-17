cmake_minimum_required(VERSION 3.10)
project(JsonParser VERSION 1.0 LANGUAGES CXX)

# Требуем C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Добавляем флаги компиляции
if(MSVC)
    add_compile_options(/W4 /utf-8)
else()
    add_compile_options(-Wall -Wextra -pedantic)
endif()

# Поддержка многопоточности
find_package(Threads REQUIRED)

# Исходные файлы
set(SOURCES
    src/Lexer.cpp
    src/Parser.cpp
    src/JsonValue.cpp
    src/Serializer.cpp
    src/Generator.cpp
    src/Validator.cpp
    src/ParallelProcessor.cpp
    src/main.cpp
)

# Заголовочные файлы
set(HEADERS
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

# Исполняемый файл
add_executable(jsonparser ${SOURCES} ${HEADERS})

# Путь к заголовочным файлам
target_include_directories(jsonparser PRIVATE ${CMAKE_SOURCE_DIR}/include)

# Линковка с pthread
target_link_libraries(jsonparser PRIVATE Threads::Threads)

# Информация о сборке
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
