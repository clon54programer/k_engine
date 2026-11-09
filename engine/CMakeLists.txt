add_library(k_engine STATIC
    src/version.cpp
    src/stb_impl.cpp
)

target_include_directories(k_engine
    PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}"
)

target_link_libraries(k_engine
    PRIVATE
        stb
)

# FlatBuffers: generación de código y pruebas del motor.
find_package(Python3 COMPONENTS Interpreter REQUIRED)

set(K_ENGINE_SCHEMA_DIR "${CMAKE_CURRENT_SOURCE_DIR}/schema")
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

# Pruebas del motor.
add_executable(k_engine_version_test
    test/version_test.cpp
)
target_link_libraries(k_engine_version_test
    PRIVATE
        k_engine
)
add_test(
    NAME k_engine_version_test
    COMMAND k_engine_version_test
)

add_executable(k_engine_flatbuffers_roundtrip_test
    test/flatbuffers_roundtrip_test.cpp
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
            "PYTHONPATH=${K_ENGINE_GENERATED_PYTHON_DIR}:${CMAKE_SOURCE_DIR}/third_party/flatbuffers/python"
            "${Python3_EXECUTABLE}"
            "${CMAKE_CURRENT_SOURCE_DIR}/test/generate_level_buffer.py"
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

add_executable(k_engine_dependencies_smoke_test
    test/dependencies_smoke_test.cpp
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
    test/glfw_vulkan_build_test.cpp
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
