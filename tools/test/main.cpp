#include <iostream>

#include "ast.h"


using namespace ns_ast;

int main(int argc, char** argv)
{
    if (argc < 2)
        return -1;

    AST ast = AST(argv[1], {"-xc++", "-std=c++11", "-O0"});
    std::cout << ast << std::endl;

    return 0;
}
