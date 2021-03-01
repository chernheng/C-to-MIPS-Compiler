#include "include/ast.hpp"
#include <fstream>

int main(int argc, char** argv)
{   
    const Program *ast=parseAST(argv[2]);
    Context context;
    std::ofstream myfile;
    std::string out_file = argv[4];
    myfile.open(out_file);

    ast->generate(myfile,"$v0",&context);
    myfile.close();
    ast->print(std::cout);
    std::cout<<std::endl;
    

    delete ast;

    return 0;
}
