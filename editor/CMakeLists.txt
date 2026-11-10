find_package(Python3 COMPONENTS Interpreter REQUIRED)

find_program(GLSLC_EXECUTABLE NAMES glslc HINTS "$ENV{VULKAN_SDK}/bin")
if(NOT GLSLC_EXECUTABLE)
    message(FATAL_ERROR
        "No se encontró glslc. Carga setup-env.sh del Vulkan SDK antes de configurar."
    )
endif()

set(K_EDITOR_SHADER_DIR "${CMAKE_CURRENT_SOURCE_DIR}/shaders")
set(K_EDITOR_SHADER_OUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/shaders")
set(K_EDITOR_GENERATED_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated")
set(K_EDITOR_QUAD_VERT_SPV "${K_EDITOR_SHADER_OUT_DIR}/quad.vert.spv")
set(K_EDITOR_QUAD_FRAG_SPV "${K_EDITOR_SHADER_OUT_DIR}/quad.frag.spv")
set(K_EDITOR_SHADERS_HEADER "${K_EDITOR_GENERATED_DIR}/editor_shaders.h")

add_library(k_editor_core STATIC
    src/document.cpp
    src/validation.cpp
    src/project_io.cpp
    src/tile_tool.cpp
    src/animation_player.cpp
    src/cli.cpp
)

target_include_directories(k_editor_core
    PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

target_link_libraries(k_editor_core
    PUBLIC
        k_engine
        nlohmann_json::nlohmann_json
        stb
)

add_custom_command(
    OUTPUT "${K_EDITOR_QUAD_VERT_SPV}" "${K_EDITOR_QUAD_FRAG_SPV}"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${K_EDITOR_SHADER_OUT_DIR}"
    COMMAND "${GLSLC_EXECUTABLE}" --target-env=vulkan1.3 -fshader-stage=vert
            -o "${K_EDITOR_QUAD_VERT_SPV}" "${K_EDITOR_SHADER_DIR}/quad.vert"
    COMMAND "${GLSLC_EXECUTABLE}" --target-env=vulkan1.3 -fshader-stage=frag
            -o "${K_EDITOR_QUAD_FRAG_SPV}" "${K_EDITOR_SHADER_DIR}/quad.frag"
    DEPENDS
        "${K_EDITOR_SHADER_DIR}/quad.vert"
        "${K_EDITOR_SHADER_DIR}/quad.frag"
    COMMENT "Compilando shaders del editor a SPIR-V"
    VERBATIM
)

add_custom_command(
    OUTPUT "${K_EDITOR_SHADERS_HEADER}"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${K_EDITOR_GENERATED_DIR}"
    COMMAND "${Python3_EXECUTABLE}"
            "${CMAKE_CURRENT_SOURCE_DIR}/../engine/tools/embed_spirv.py"
            "${K_EDITOR_SHADERS_HEADER}"
            "${K_EDITOR_QUAD_VERT_SPV}"
            "${K_EDITOR_QUAD_FRAG_SPV}"
            "k_engine::editor::shaders"
            "QUAD"
    DEPENDS
        "${K_EDITOR_QUAD_VERT_SPV}"
        "${K_EDITOR_QUAD_FRAG_SPV}"
        "${CMAKE_CURRENT_SOURCE_DIR}/../engine/tools/embed_spirv.py"
    COMMENT "Embebiendo shaders del editor en la cabecera C++"
    VERBATIM
)

add_custom_target(k_editor_shaders ALL
    DEPENDS "${K_EDITOR_SHADERS_HEADER}"
)

add_executable(editor
    main.cpp
    src/app.cpp
    src/gfx/vulkan_context.cpp
    src/gfx/quad_renderer.cpp
    src/gfx/texture_cache.cpp
)

target_include_directories(editor
    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
        "${K_EDITOR_GENERATED_DIR}"
)

target_link_libraries(editor
    PRIVATE
        k_editor_core
        k_engine
        nlohmann_json::nlohmann_json
        stb
        glfw
        Vulkan::Vulkan
)

add_dependencies(editor k_editor_shaders)

# Las pruebas del editor deben registrarse aquí con add_test().
add_executable(k_editor_document_test
    test/document_test.cpp
)
target_link_libraries(k_editor_document_test
    PRIVATE
        k_editor_core
        doctest::doctest
)
add_test(
    NAME k_editor_document_test
    COMMAND k_editor_document_test
)

add_executable(k_editor_project_io_test
    test/project_io_test.cpp
)
target_link_libraries(k_editor_project_io_test
    PRIVATE
        k_editor_core
        doctest::doctest
)
add_test(
    NAME k_editor_project_io_test
    COMMAND k_editor_project_io_test
)

add_executable(k_editor_level_edit_test
    test/level_edit_test.cpp
)
target_link_libraries(k_editor_level_edit_test
    PRIVATE
        k_editor_core
        doctest::doctest
)
add_test(
    NAME k_editor_level_edit_test
    COMMAND k_editor_level_edit_test
)

add_executable(k_editor_animation_test
    test/animation_test.cpp
)
target_link_libraries(k_editor_animation_test
    PRIVATE
        k_editor_core
        doctest::doctest
)
add_test(
    NAME k_editor_animation_test
    COMMAND k_editor_animation_test
)

add_executable(k_editor_cli_test
    test/cli_test.cpp
)
target_link_libraries(k_editor_cli_test
    PRIVATE
        k_editor_core
        doctest::doctest
)
add_test(
    NAME k_editor_cli_test
    COMMAND k_editor_cli_test
)
