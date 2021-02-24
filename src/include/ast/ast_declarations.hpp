#ifndef COMPILER_AST_DECLARATIONS_HPP
#define COMPILER_AST_DECLARATIONS_HPP

#include "src/include/ast.hpp"

class DeclareVariable : public Program {
    private:
        std::string type;
        std::string id;
        std::string init="";
    public:
        DeclareVariable(std::string _type, std::string _id, std::string _init) : type(_type), id(_id), init(_init)  {}

        DeclareVariable(std::string _type, std::string _id) : type(_type),id(_id)   {}

        virtual void print(std::ostream &dst) const override    {
            dst<<type<<" "<<id;
            if(init!="")    {
                dst<<"="<<init;
            }
        }
};

#endif