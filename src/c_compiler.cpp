#include "include/ast.hpp"
#include <fstream>

int main(int argc, char** argv)
{   


    std::ofstream myfile;
    std::string out_file = argv[4];
    myfile.open(out_file);
    const Program *ast=parseAST(argv[2]);
    Context context;

    context.typeTable.insert(std::pair<std::string,typeInfo>("int",{"",4,0}));  // insert int type into typeTable
    context.typeTable.insert(std::pair<std::string,typeInfo>("char",{"",1,0}));  // insert char type into typeTable
    context.typeTable.insert(std::pair<std::string,typeInfo>("float",{"",4,0}));  // insert float type into typeTable
    context.typeTable.insert(std::pair<std::string,typeInfo>("double",{"",8,0}));  // insert double type into typeTable
    context.typeTable.insert(std::pair<std::string,typeInfo>("unsigned",{"",4,0}));  // insert unsigned type into typeTable (unsigned: unsigned int)

    myfile<<".abicalls"<<std::endl;
    // myfile<<".text"<<std::endl;
    ast->print(std::cout);
    ast->generate(myfile,"$v0",&context);
    std::cout<<"done compiling"<<std::endl;
    myfile.close();
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