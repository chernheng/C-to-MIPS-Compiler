#ifndef COMPILER_AST_PROGRAM_HPP
#define COMPILER_AST_PROGRAM_HPP

// #include "src/include/ast.hpp"
#include "variable_table.hpp"

class Program;
typedef const Program *ProgramPtr;

class Program {
    public:
        virtual ~Program()  {};

        virtual long getOffset(Context *context) const  {   // for assigning to variables
            return 0;
        }

        virtual std::string getVarType(Context *context) const  {   // for assigning to variables
            return "";
        }

        virtual int getPointer(Context *context) const {
            return 0;
        }

        virtual long spaceRequired(Context *context) const  {
            return 0;
        }

        virtual void print(std::ostream &dst) const =0;

        virtual void comparison(std::ofstream &file, const char* srcReg, Context *context) const   { // for switch case
        }
        
        // Implement generate function to generate code
        virtual void generate(std::ofstream &file, const char*destReg, Context *context) const   {     // consider changing bindings to a struct containing the var and fn LUTs
            throw std::runtime_error("Not yet implemented"); 
        }
};

class Command : public Program { //each line of a program is a command, it is wrapper to make things into a linked list
    private:
        ProgramPtr action; //the code to be performed
        ProgramPtr next=nullptr;//next element to the list
    public:
        Command(ProgramPtr _action, ProgramPtr _next) : action(_action), next(_next)    {}

        ~Command()  {
            delete action;
            delete next;
        }

        virtual long spaceRequired(Context *context) const override {
            long tmp = action->spaceRequired(context);
            if(next!=nullptr)   {
                tmp += next->spaceRequired(context);
            }
            return tmp;
        }

        virtual void print(std::ostream &dst) const override    {
            action->print(dst);
            dst<<std::endl;
            if(next!=nullptr)   {
                next->print(dst);
            }
        }

        virtual void generate(std::ofstream &file, const char* destReg, Context *context) const override {
            if(context->stack.lut.size()==0) {
                std::unordered_map<std::string,varInfo> tmp;
                context->stack.lut.push_back(tmp);
                long stackSize = spaceRequired(context);
                context->stack.size = stackSize;
                file<<"addiu $sp, $sp, -"<<stackSize<<std::endl;
            }
            action->generate(file, destReg, context);
            if(next!=nullptr)   {
                next->generate(file, destReg, context);
            }
        }
};

class Scope : public Program {
    private:
        ProgramPtr action=nullptr;
    public:
        Scope(ProgramPtr _action) : action(_action) {}

        ~Scope()    {
            delete action;
        }

        virtual void print(std::ostream &dst) const override    { //dont need to include curly brackets in parser, does the frame pointer
            dst<<"{"<<std::endl;
            if(action!=nullptr) {
                action->print(dst);
            }
            dst<<std::endl<<"}"<<std::endl;
        }

        virtual void generate(std::ofstream &file, const char* destReg, Context *context) const override {
            if(action!=nullptr) {
                std::unordered_map<std::string,varInfo> tmp;
                int isFunc = context->isFunc;
                context->isFunc=0;
                long initStackSize = context->stack.size;
                long initSliderVal=context->stack.slider;
                context->stack.slider=context->stack.size;
                long delta=action->spaceRequired(context);
                context->stack.size+=delta;
                if(delta>0) {
                    file<<"addiu $sp, $sp, -"<<delta<<std::endl;
                }
                context->stack.lut.push_back(tmp);
                action->generate(file, destReg, context);          // run scope contents
                if(isFunc==1)   {
                    file<<context->FuncRetnPoint<<":"<<std::endl;
                }
                if(delta>0)    {
                    file<<"addiu $sp, $sp, "<<delta<<std::endl;    // shift down the stack pointer (always move sp by 4 to maintain word alignment)
                }
                context->stack.size=initStackSize;
                context->stack.slider=initSliderVal;
                context->stack.lut.pop_back();
            }            
        }
};

#endif