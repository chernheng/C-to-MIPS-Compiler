#include "ast.hpp"

int main()
{
    const Program *ast=parseAST();

    ast->print(std::cout);
    std::cout<<std::endl;

    return 0;
}