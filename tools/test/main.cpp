#include <iostream>
#include <string>
#include <vector>

#include "ast.h"


using namespace std;
using namespace ns_ast;

int main(int argc, char** argv)
{
    if (argc < 2)
        return -1;

    const char* src_path = argv[1];
    int cli_args_num = argc - 2;
    const char** cli_args = (cli_args_num > 0) ? (const char**)(argv + 2) : nullptr;

    // AST ast = AST(src_path, {"-xc++", "-std=c++11", "-O0"});
    AST ast = AST(src_path, {cli_args, cli_args + cli_args_num});
    cout << ast << endl;

    return 0;
}
