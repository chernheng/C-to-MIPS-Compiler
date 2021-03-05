#ifndef COMPILER_AST_DECLARATIONS_HPP
#define COMPILER_AST_DECLARATIONS_HPP

// #include "src/include/ast.hpp"
#include "variable_table.hpp"

class DeclareVariable : public Program {
    private:
        std::string type;
        std::string id;
        ProgramPtr init=nullptr; //int x = 5;
        int ptr=0;
    public:
        DeclareVariable(std::string *_type, std::string *_id, ProgramPtr _init, int _ptr) : type(*_type), id(*_id), init(_init), ptr(_ptr)  {
            delete _type;
            delete _id;
        }

        DeclareVariable(std::string *_type, std::string *_id, int _ptr) : type(*_type),id(*_id), ptr(_ptr)   {
            delete _type;
            delete _id;
        }

        ~DeclareVariable() {
            delete init;
        }

        std::string getID() const   {
            return id;
        }

        std::string getType() const {
            return type;
        }
        
        int getPtr() const {
            return ptr;
        }

        virtual void print(std::ostream &dst) const override    {
            dst<<type<<" "<<id;
            if(init!=nullptr)    {
                dst<<"=";
                init->print(dst);
            }
            dst<<";";
        }

        virtual long spaceRequired() const override {
            if(type=="int" || type=="char")  {
                return 4;
            }
            else    {       // temporary, replace later with size requirements of other variable types
                return 0;
            }
        }

        virtual void generate(std::ofstream &file, const char* destReg, Context *context) const override {
            long size = context->stack.lut.size();
            long offset = context->stack.slider;
            varInfo vf;
            vf.offset=offset;
            vf.length=1;
            vf.isPtr =getPtr();
            int stackInc=0;
            if(getType()=="int")    {
                vf.type="int";
                vf.numBytes=4;
                stackInc=4;
            }
            else if(getType()=="char")  {
                vf.type="char";
                vf.numBytes=1;
                stackInc=1;
            }
            if (size == 1){
                file<<"   .globl  "<<getID()<<std::endl;
                file<<"   .type   "<<getID()<<", @object"<<std::endl;
                file<<"   .section        .bss,\"aw\",@nobits"<<std::endl;
                file<<"   .size   "<<getID()<<", "<<vf.numBytes<<std::endl;
                file<<getID()<<":"<<std::endl;
            }
            context->stack.lut.back().insert(std::pair<std::string,varInfo>(getID(),vf));
            if((init==nullptr)& (size==1)){
                file<<"   .space   "<<vf.numBytes<<std::endl;
            }
            if(init!=nullptr)   {
                init->generate(file, "$t7", context);
                if(stackInc==4) {
                    file<<"sw $t7, "<<(context->stack.size - offset)<<"($sp)"<<std::endl;
                }
                else if(stackInc==1)    {
                    file<<"sb $t7, "<<(context->stack.size - offset)<<"($sp)"<<std::endl;
                }
            }
            context->stack.slider+=4;
            file<<"li "<<std::string(destReg)<<", 1"<<std::endl;
            return;                        
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