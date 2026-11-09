add_executable(editor
    main.cpp
)

target_link_libraries(editor
    PRIVATE
        k_engine
)

# Las pruebas del editor deben registrarse aquí con add_test().
