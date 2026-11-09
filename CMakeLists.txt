cmake_minimum_required(VERSION 3.28)

project(K_engine VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_library(k_engine STATIC
    engine/src/version.cpp
    engine/src/stb_impl.cpp
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

# FlatBuffers: compilar flatc y el runtime C++ desde el submodule.
set(FLATBUFFERS_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(FLATBUFFERS_INSTALL OFF CACHE BOOL "" FORCE)
set(FLATBUFFERS_BUILD_FLATC ON CACHE BOOL "" FORCE)
set(FLATBUFFERS_BUILD_FLATLIB ON CACHE BOOL "" FORCE)
set(FLATBUFFERS_BUILD_SHAREDLIB OFF CACHE BOOL "" FORCE)
set(FLATBUFFERS_BUILD_GRPCTEST OFF CACHE BOOL "" FORCE)
set(FLATBUFFERS_BUILD_BENCHMARKS OFF CACHE BOOL "" FORCE)
add_subdirectory(third_party/flatbuffers EXCLUDE_FROM_ALL)

find_package(Python3 COMPONENTS Interpreter REQUIRED)

set(K_ENGINE_SCHEMA_DIR "${CMAKE_CURRENT_SOURCE_DIR}/engine/schema")
set(K_ENGINE_GENERATED_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated")
set(K_ENGINE_GENERATED_CPP_DIR "${K_ENGINE_GENERATED_DIR}/cpp")
set(K_ENGINE_GENERATED_PYTHON_DIR "${K_ENGINE_GENERATED_DIR}/python")
set(K_ENGINE_FLATBUFFERS_STAMP "${K_ENGINE_GENERATED_DIR}/flatbuffers.stamp")

add_custom_command(
    OUTPUT "${K_ENGINE_FLATBUFFERS_STAMP}"
    COMMAND ${CMAKE_COMMAND} -E make_directory
            "${K_ENGINE_GENERATED_CPP_DIR}"
            "${K_ENGINE_GENERATED_PYTHON_DIR}"
    COMMAND $<TARGET_FILE:flatc>
            --cpp -o "${K_ENGINE_GENERATED_CPP_DIR}"
            "${K_ENGINE_SCHEMA_DIR}/level.fbs"
            "${K_ENGINE_SCHEMA_DIR}/animation.fbs"
    COMMAND $<TARGET_FILE:flatc>
            --python -o "${K_ENGINE_GENERATED_PYTHON_DIR}"
            "${K_ENGINE_SCHEMA_DIR}/level.fbs"
            "${K_ENGINE_SCHEMA_DIR}/animation.fbs"
    COMMAND ${CMAKE_COMMAND} -E touch "${K_ENGINE_FLATBUFFERS_STAMP}"
    DEPENDS
        flatc
        "${K_ENGINE_SCHEMA_DIR}/level.fbs"
        "${K_ENGINE_SCHEMA_DIR}/animation.fbs"
    COMMENT "Generando código FlatBuffers C++ y Python"
    VERBATIM
)

add_custom_target(k_engine_flatbuffers ALL
    DEPENDS "${K_ENGINE_FLATBUFFERS_STAMP}"
)

add_executable(k_engine_flatbuffers_roundtrip_test
    engine/test/flatbuffers_roundtrip_test.cpp
)
add_dependencies(k_engine_flatbuffers_roundtrip_test k_engine_flatbuffers)
target_include_directories(k_engine_flatbuffers_roundtrip_test
    PRIVATE "${K_ENGINE_GENERATED_CPP_DIR}"
)
target_link_libraries(k_engine_flatbuffers_roundtrip_test
    PRIVATE flatbuffers
)

set(K_ENGINE_LEVEL_BUFFER "${CMAKE_CURRENT_BINARY_DIR}/test_level.bin")

add_test(
    NAME k_engine_flatbuffers_python_generate
    COMMAND ${CMAKE_COMMAND} -E env
            "PYTHONDONTWRITEBYTECODE=1"
            "PYTHONPATH=${K_ENGINE_GENERATED_PYTHON_DIR}:${CMAKE_CURRENT_SOURCE_DIR}/third_party/flatbuffers/python"
            "${Python3_EXECUTABLE}"
            "${CMAKE_CURRENT_SOURCE_DIR}/packer/tests/generate_level_buffer.py"
            "${K_ENGINE_LEVEL_BUFFER}"
)
set_tests_properties(k_engine_flatbuffers_python_generate PROPERTIES
    FIXTURES_SETUP k_engine_level_buffer
)

add_test(
    NAME k_engine_flatbuffers_python_to_cpp
    COMMAND k_engine_flatbuffers_roundtrip_test "${K_ENGINE_LEVEL_BUFFER}"
)
set_tests_properties(k_engine_flatbuffers_python_to_cpp PROPERTIES
    FIXTURES_REQUIRED k_engine_level_buffer
)

# GLFW: ventana X11 e input. Sin Wayland ni artefactos auxiliares.
set(GLFW_BUILD_X11 ON CACHE BOOL "" FORCE)
set(GLFW_BUILD_WAYLAND OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
add_subdirectory(third_party/glfw EXCLUDE_FROM_ALL)

# nlohmann/json: parser JSON del editor.
set(JSON_BuildTests OFF CACHE BOOL "" FORCE)
add_subdirectory(third_party/nlohmann_json EXCLUDE_FROM_ALL)

# doctest: framework de pruebas C++.
set(DOCTEST_WITH_TESTS OFF CACHE BOOL "" FORCE)
add_subdirectory(third_party/doctest EXCLUDE_FROM_ALL)

# stb: decodificación PNG y rasterización de texto.
add_library(stb INTERFACE)
target_include_directories(stb SYSTEM INTERFACE
    "${CMAKE_CURRENT_SOURCE_DIR}/third_party/stb"
)
target_link_libraries(k_engine PRIVATE stb)

find_package(Vulkan REQUIRED)

add_executable(k_engine_dependencies_smoke_test
    engine/test/dependencies_smoke_test.cpp
)
target_link_libraries(k_engine_dependencies_smoke_test
    PRIVATE
        k_engine
        nlohmann_json::nlohmann_json
        doctest::doctest
        stb
)

add_test(
    NAME k_engine_dependencies_smoke_test
    COMMAND k_engine_dependencies_smoke_test
)

add_executable(k_engine_glfw_vulkan_build_test
    engine/test/glfw_vulkan_build_test.cpp
)
target_link_libraries(k_engine_glfw_vulkan_build_test
    PRIVATE
        glfw
        Vulkan::Vulkan
)

add_test(
    NAME k_engine_glfw_vulkan_build_test
    COMMAND k_engine_glfw_vulkan_build_test
)
