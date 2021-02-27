#ifndef COMPILER_AST_DECLARATIONS_HPP
#define COMPILER_AST_DECLARATIONS_HPP

// #include "src/include/ast.hpp"
#include "variable_table.hpp"

class DeclareVariable : public Program {
    private:
        std::string type;
        std::string id;
        ProgramPtr init=nullptr; //int x = 5;
    public:
        DeclareVariable(std::string *_type, std::string *_id, ProgramPtr _init) : type(*_type), id(*_id), init(_init)  {
            delete _type;
            delete _id;
        }

        DeclareVariable(std::string *_type, std::string *_id) : type(*_type),id(*_id)   {
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

        virtual void print(std::ostream &dst) const override    {
            dst<<type<<" "<<id;
            if(init!=nullptr)    {
                dst<<"=";
                init->print(dst);
            }
            dst<<";";
        }

        virtual void generate(std::ofstream &file, const char* destReg, Context *context) const override {
            std::unordered_map<std::string,varInfo>::iterator it;
            it=context->stack.lut.back().find(getID());
            if(it==context->stack.lut.back().end())  {   // if variable is not found in latest variable scope
                long offset = context->stack.size;
                varInfo vf;
                vf.offset=offset;
                vf.length=1;
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
                context->stack.lut.back().insert(std::pair<std::string,varInfo>(getID(),vf));
                if(init!=nullptr)   {
                    init->generate(file, "$t7", context);
                    if(stackInc==4) {
                        file<<"sw $t7, 0($sp)"<<std::endl;
                    }
                    else if(stackInc==1)    {
                        file<<"sb $t7, 0($sp)"<<std::endl;
                    }
                }
                file<<"addiu $sp, $sp, -4"<<std::endl;     // bring $sp to top of stack (always move by 4 bytes to maintain word alignment)
                context->stack.size+=4;
                file<<"li "<<std::string(destReg)<<", 1"<<std::endl;
                return;
            }   
            else    {                                     // if variable is found in latest variable scope
                if(init!=nullptr)   {
                    long offset = context->stack.size - it->second.offset;
                    init->generate(file, "$t7", context);
                    if(it->second.numBytes==4)  {
                        file<<"sw $t7, "<<offset<<"($t7)"<<std::endl;
                    }
                    else if(it->second.numBytes==1) {
                        file<<"sb $t7, "<<offset<<"($t7)"<<std::endl;
                    }
                }
            }
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