#include "include/ast.hpp"
#include <fstream>

int main(int argc, char** argv)
{   


    std::ofstream myfile;
    std::string out_file = argv[4];
    myfile.open(out_file);
    myfile << "   .text"<<std::endl;
    myfile << "   .globl	fn1"<<std::endl;
    myfile << "   .ent	fn1"<<std::endl;
    myfile << "   .type	fn1, @function"<<std::endl;
    const Program *ast=parseAST(argv[2]);
    Context context;
    ast->generate(myfile,"$v0",&context);
    myfile << "    .end     fn1"<<std::endl;
    myfile.close();
    ast->print(std::cout);
    std::cout<<std::endl;
    

    delete ast;

    return 0;
}
