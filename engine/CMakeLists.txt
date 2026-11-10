find_package(Python3 COMPONENTS Interpreter REQUIRED)

set(K_ENGINE_SCHEMA_DIR "${CMAKE_CURRENT_SOURCE_DIR}/schema")
set(K_ENGINE_GENERATED_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated")
set(K_ENGINE_GENERATED_CPP_DIR "${K_ENGINE_GENERATED_DIR}/cpp")
set(K_ENGINE_GENERATED_PYTHON_DIR "${K_ENGINE_GENERATED_DIR}/python")
set(K_ENGINE_FLATBUFFERS_STAMP "${K_ENGINE_GENERATED_DIR}/flatbuffers.stamp")

add_library(k_engine STATIC
    src/version.cpp
    src/stb_impl.cpp
    src/resource/crc32.cpp
    src/resource/pak_reader.cpp
)

target_include_directories(k_engine
    PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}"
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

target_link_libraries(k_engine
    PRIVATE
        stb
)

# Runtime Linux: ventana X11, presentación y carga de contenido de T2.
add_executable(k_engine_runtime
    src/runtime/main.cpp
    src/runtime/vulkan_runtime.cpp
    src/runtime/content_loader.cpp
    src/runtime/input_state.cpp
    src/runtime/sprite_renderer.cpp
)

target_include_directories(k_engine_runtime
    PRIVATE
        include
        "${K_ENGINE_GENERATED_CPP_DIR}"
)

target_link_libraries(k_engine_runtime
    PRIVATE
        k_engine
        flatbuffers
        stb
        glfw
        Vulkan::Vulkan
)

# FlatBuffers: generación de código y pruebas del motor.
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

# Shaders del sprite: GLSL -> SPIR-V -> cabecera C++ embebida.
find_program(GLSLC_EXECUTABLE NAMES glslc HINTS "$ENV{VULKAN_SDK}/bin")
if(NOT GLSLC_EXECUTABLE)
    message(FATAL_ERROR
        "No se encontró glslc. Carga setup-env.sh del Vulkan SDK antes de configurar."
    )
endif()

set(K_ENGINE_SHADER_DIR "${CMAKE_CURRENT_SOURCE_DIR}/shaders")
set(K_ENGINE_SHADER_OUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/shaders")
set(K_ENGINE_SPRITE_VERT_SPV "${K_ENGINE_SHADER_OUT_DIR}/sprite.vert.spv")
set(K_ENGINE_SPRITE_FRAG_SPV "${K_ENGINE_SHADER_OUT_DIR}/sprite.frag.spv")
set(K_ENGINE_SPRITE_SHADERS_HEADER "${K_ENGINE_GENERATED_CPP_DIR}/sprite_shaders.h")

add_custom_command(
    OUTPUT "${K_ENGINE_SPRITE_VERT_SPV}" "${K_ENGINE_SPRITE_FRAG_SPV}"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${K_ENGINE_SHADER_OUT_DIR}"
    COMMAND "${GLSLC_EXECUTABLE}" --target-env=vulkan1.3 -fshader-stage=vert
            -o "${K_ENGINE_SPRITE_VERT_SPV}" "${K_ENGINE_SHADER_DIR}/sprite.vert"
    COMMAND "${GLSLC_EXECUTABLE}" --target-env=vulkan1.3 -fshader-stage=frag
            -o "${K_ENGINE_SPRITE_FRAG_SPV}" "${K_ENGINE_SHADER_DIR}/sprite.frag"
    DEPENDS
        "${K_ENGINE_SHADER_DIR}/sprite.vert"
        "${K_ENGINE_SHADER_DIR}/sprite.frag"
    COMMENT "Compilando shaders GLSL a SPIR-V"
    VERBATIM
)

add_custom_command(
    OUTPUT "${K_ENGINE_SPRITE_SHADERS_HEADER}"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${K_ENGINE_GENERATED_CPP_DIR}"
    COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/tools/embed_spirv.py"
            "${K_ENGINE_SPRITE_SHADERS_HEADER}"
            "${K_ENGINE_SPRITE_VERT_SPV}"
            "${K_ENGINE_SPRITE_FRAG_SPV}"
    DEPENDS
        "${K_ENGINE_SPRITE_VERT_SPV}"
        "${K_ENGINE_SPRITE_FRAG_SPV}"
        "${CMAKE_CURRENT_SOURCE_DIR}/tools/embed_spirv.py"
    COMMENT "Embebiendo SPIR-V en la cabecera C++"
    VERBATIM
)

add_custom_target(k_engine_shaders ALL
    DEPENDS "${K_ENGINE_SPRITE_SHADERS_HEADER}"
)

add_dependencies(k_engine_runtime k_engine_flatbuffers k_engine_shaders)

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

# Fixture del runtime T2: paquete válido y variantes inválidas del contrato.
set(K_ENGINE_T2_FIXTURE_DIR "${CMAKE_CURRENT_BINARY_DIR}/test_runtime_fixture")

add_test(
    NAME k_engine_t2_fixture_generate
    COMMAND ${CMAKE_COMMAND} -E env
            "PYTHONDONTWRITEBYTECODE=1"
            "PYTHONPATH=${K_ENGINE_GENERATED_PYTHON_DIR}:${CMAKE_SOURCE_DIR}/third_party/flatbuffers/python"
            "${Python3_EXECUTABLE}"
            "${CMAKE_CURRENT_SOURCE_DIR}/test/generate_runtime_fixture.py"
            "${K_ENGINE_T2_FIXTURE_DIR}"
)
set_tests_properties(k_engine_t2_fixture_generate PROPERTIES
    FIXTURES_SETUP k_engine_t2_fixture
)

add_executable(k_engine_crc32_test
    test/crc32_test.cpp
)
target_link_libraries(k_engine_crc32_test
    PRIVATE
        k_engine
        doctest::doctest
)
add_test(
    NAME k_engine_crc32_test
    COMMAND k_engine_crc32_test
)

add_executable(k_engine_pak_reader_test
    test/pak_reader_test.cpp
)
target_link_libraries(k_engine_pak_reader_test
    PRIVATE
        k_engine
        doctest::doctest
)
add_test(
    NAME k_engine_pak_reader_test
    COMMAND k_engine_pak_reader_test "${K_ENGINE_T2_FIXTURE_DIR}"
)
set_tests_properties(k_engine_pak_reader_test PROPERTIES
    FIXTURES_REQUIRED k_engine_t2_fixture
)

add_executable(k_engine_runtime_content_test
    test/runtime_content_test.cpp
    src/runtime/content_loader.cpp
)
add_dependencies(k_engine_runtime_content_test k_engine_flatbuffers)
target_include_directories(k_engine_runtime_content_test
    PRIVATE
        include
        "${K_ENGINE_GENERATED_CPP_DIR}"
)
target_link_libraries(k_engine_runtime_content_test
    PRIVATE
        k_engine
        flatbuffers
        stb
        doctest::doctest
)
add_test(
    NAME k_engine_runtime_content_test
    COMMAND k_engine_runtime_content_test "${K_ENGINE_T2_FIXTURE_DIR}"
)
set_tests_properties(k_engine_runtime_content_test PROPERTIES
    FIXTURES_REQUIRED k_engine_t2_fixture
)
