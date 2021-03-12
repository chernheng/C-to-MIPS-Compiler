#include "include/ast.hpp"
#include <fstream>

int main(int argc, char** argv)
{   


    std::ofstream myfile;
    std::string out_file = argv[4];
    myfile.open(out_file);
    const Program *ast=parseAST(argv[2]);
    Context context;
    myfile<<".abicalls"<<std::endl;
    myfile<<".text"<<std::endl;
    ast->generate(myfile,"$v0",&context);
    std::cout<<"done compiling"<<std::endl;
    myfile.close();
    ast->print(std::cout);
    std::cout<<std::endl;
    

    delete ast;

    return 0;
}
//global var macros
    // myfile<<"   .globl  b"<<std::endl;
    // myfile<<"   .type   b, @object"<<std::endl;
    // myfile<<"   .size   b, 4"<<std::endl;
    // myfile<<"b:"<<std::endl;
    // myfile<<"   .word   35"<<std::endl;