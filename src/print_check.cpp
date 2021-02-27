#include "include/ast.hpp"
#include <fstream>

int main()
{
    const Program *ast=parseAST();
    Context context;
    std::ofstream myfile;
    myfile.open("output.txt");

    ast->generate(myfile,"$v0",&context);
    myfile.close();
    ast->print(std::cout);
    std::cout<<std::endl;
    

    delete ast;

    return 0;
}
