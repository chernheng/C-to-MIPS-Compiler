#ifndef COMPILER_AST_FUNCTIONS_HPP
#define COMPILER_AST_FUNCTIONS_HPP

#include "variable_table.hpp"
std::string makeLabel(const char* _name);

class Function : public Program {   // function call 
    private:
        std::string id;
    public:
        Function(std::string *_id) : id(*_id)   {
            delete _id;
        }
        
        virtual void print(std::ostream &dst) const override    {
            dst<<id<<"();"<<std::endl;
        }
};

class FunctionDef : public Program {    // function definition 
    private:
        std::string type; // return type of function
        std::string id; // name of function
        ProgramPtr action; //the scope of the function
    public:
        FunctionDef(std::string *_type, std::string *_id, ProgramPtr _action) : type(*_type), id(*_id), action(_action)    {
            delete _type;
            delete _id;
        }  

        ~FunctionDef() {
            delete action;
        }

        ProgramPtr getAction() const    {
            return action;
        }

        std::string getID() const    {
            return id;
        }

        std::string getType() const    {
            return type;
        }

        // virtual long spaceRequired() const override {   // 5 ints needed: fp, a0, a1, a2 and a3
        //     return 20;
        // }

        void print(std::ostream &dst) const override    {
            dst<<getType()<<" "<<getID()<<"() ";
            getAction()->print(dst);
        }

        virtual void generate(std::ofstream &file, const char* destReg, Context *context) const override    {
            long initSP = context->stack.size;
            long preSpace=4;                                    // space (in bytes) needed for FP and arguments
            std::string initFuncEnd = context->FuncRetnPoint;
            std::string returnPoint = makeLabel("func_end");
            context->FuncRetnPoint = returnPoint;
            context->isFunc=1;
            file<<getID()<<":"<<std::endl;          // start of function
            context->stack.size += preSpace;                 // allocate space for $fp (and arguments too [implement later])
            file<<"addiu $sp, $sp, -"<<preSpace<<std::endl;
            long initFP = context->stack.slider;
            context->stack.slider+=4;
            file<<"sw $fp, "<<(context->stack.size - initFP)<<"($sp)"<<std::endl;   // store $fp
            file<<"move $fp, $sp"<<std::endl;
            action->generate(file, destReg, context);   // run action
            file<<"move $sp, $fp"<<std::endl;
            file<<"lw $fp, "<<(context->stack.size - initFP)<<"($sp)"<<std::endl;
            file<<"addiu $sp, $sp, "<<preSpace<<std::endl;     // deallocate space used for FP and arguments
            file<<"jr $ra"<<std::endl;                         // end of function, return to caller 
            file<<"nop"<<std::endl;
            context->stack.slider = initFP;               // dealloc initFP
            context->FuncRetnPoint = initFuncEnd;
            context->stack.size = initSP;
        }
};

#endif