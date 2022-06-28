add_executable(test
    ast.cpp
    main.cpp
)

target_link_libraries(test
    clang
    stdc++fs
)
