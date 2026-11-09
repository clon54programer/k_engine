cmake_minimum_required(VERSION 3.28)

project(K_engine VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_library(k_engine STATIC
    engine/src/version.cpp
)

target_include_directories(k_engine
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/engine
)

add_executable(editor
    editor/main.cpp
)

target_link_libraries(editor
    PRIVATE
        k_engine
)

enable_testing()

add_executable(k_engine_version_test
    engine/test/version_test.cpp
)

target_link_libraries(k_engine_version_test
    PRIVATE
        k_engine
)

add_test(
    NAME k_engine_version_test
    COMMAND k_engine_version_test
)
