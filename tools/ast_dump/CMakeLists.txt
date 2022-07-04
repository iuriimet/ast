add_executable(ast_dump
    ast.cpp
    main.cpp
)

target_link_libraries(ast_dump
    clang
    stdc++fs
)
