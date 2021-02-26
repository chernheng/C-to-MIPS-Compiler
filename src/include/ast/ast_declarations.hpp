#ifndef COMPILER_AST_DECLARATIONS_HPP
#define COMPILER_AST_DECLARATIONS_HPP

#include "src/include/ast.hpp"

class DeclareVariable : public Program {
    private:
        std::string type;
        std::string id;
        std::string init=""; //int x = 5;
    public:
        DeclareVariable(std::string *_type, std::string *_id, std::string *_init) : type(*_type), id(*_id), init(*_init)  {
            delete _type;
            delete _id;
            delete _init;
        }

        DeclareVariable(std::string *_type, std::string *_id) : type(*_type),id(*_id)   {
            delete _type;
            delete _id;
        }

        virtual void print(std::ostream &dst) const override    {
            dst<<type<<" "<<id;
            if(init!="")    {
                dst<<"="<<init;
            }
            dst<<";";
        }
};

class DeclareFunction : public Program {
    private:
        std::string type;
        std::string id;
    public:
        DeclareFunction(std::string *_type, std::string *_id) : type(*_type), id(*_id)  {
            delete _type;
            delete _id;
        }

        virtual void print(std::ostream &dst) const override    {
            dst<<type<<" "<<id<<"()"; //int f();
            dst<<";";
        }
};

#endif