#ifndef COMPILER_AST_FUNCTIONS_HPP
#define COMPILER_AST_FUNCTIONS_HPP

#include "variable_table.hpp"
std::string makeLabel(const char* _name);

class FunctionArgs : public Program {
    protected:
        ProgramPtr action;
        FunctionArgs *next=nullptr;
    public:
        FunctionArgs(ProgramPtr _action, FunctionArgs *_next) : action(_action), next(_next)   {}

        virtual ~FunctionArgs() {
            delete action;
            delete next;
        }

        virtual long getCount() const   {
            if(next!=nullptr)   {
                return 1+next->getCount();
            }
            else    {
                return 1;
            }
        }

        virtual void print(std::ostream &dst) const override    {
            action->print(dst);
            if(next!=nullptr)   {
                next->print(dst);
            }
        }
};

class FunctionDefArgs : public FunctionArgs {
    private:
        std::string type;
        std::string id;
    public:
        FunctionDefArgs(std::string *_type, std::string *_id, FunctionArgs *_next) : FunctionArgs(nullptr, _next), type(*_type), id(*_id)    {
            delete _type;
            delete _id;
        }

        virtual long spaceRequired() const override {
            long tmp=0;
            if(type=="int" || type=="char")  {
                tmp=4;
            }
            if(next!=nullptr)   {
                tmp+=next->spaceRequired();
            }
            return tmp;
        }

        virtual void print(std::ostream &dst) const override    {
            dst<<type<<" "<<id;
            if(next!=nullptr)   {
                dst<<", ";
                next->print(dst);
            }
        }

        virtual void generate(std::ofstream &file, const char* destReg, Context *context) const override    {
            varInfo vf;
            long delta = context->stack.FP - (4*context->ArgCount);
            vf.offset=delta;
            vf.type=type;
            if(type=="int") {
                vf.length=1;
                vf.numBytes=4;
            }
            else if(type=="char")   {
                vf.length=1;
                vf.numBytes=1;
            }
            context->stack.lut.back().insert(std::pair<std::string,varInfo>(id,vf));
            context->ftEntry->second.argList.push_back(vf);
            if(context->ArgCount<4) {
                file<<"sw $a"<<context->ArgCount<<", "<<(context->stack.size - delta)<<"($sp)"<<std::endl;
            }
            context->ArgCount++;
            if(next!=nullptr)   {
                next->generate(file, destReg,context);
            }
        }
};

class FunctionCallArgs : public FunctionArgs {
    public:
        FunctionCallArgs(ProgramPtr _action, FunctionArgs *_next) : FunctionArgs(_action, _next)   {}

        virtual void print(std::ostream &dst) const override    {
            action->print(dst);
            if(next!=nullptr)   {
                next->print(dst);
            }
        }

        virtual long spaceRequired() const override {
            long tmp = action->spaceRequired();
            if(next!=nullptr)   {
                tmp+=next->spaceRequired();
            }
            return tmp;
        }

        virtual void generate(std::ofstream &file, const char* destReg, Context *context) const override    {
            action->generate(file, "$t5", context);
            if(context->ArgCount<4) {
                file<<"move $a"<<context->ArgCount<<", $t5"<<std::endl;
            }
            else    {
                file<<"sw $t5, "<<(4*context->ArgCount)<<"($sp)"<<std::endl;
            }
            context->ArgCount++;
            if(next!=nullptr)   {
                next->generate(file, destReg, context);
            }
        }
};

class FunctionCall : public Program {   // function call 
    private:
        std::string id;
        FunctionArgs *args=nullptr;
    public:
        FunctionCall(std::string *_id, FunctionArgs *_args) : id(*_id), args(_args)   {
            delete _id;
        }

        virtual ~FunctionCall() {
            delete args;
        }
        
        virtual long spaceRequired() const override {
            if(args!=nullptr)   {
                long count = args->getCount();
                long space = (4*count)+8;
                if(space<24)    {               // set minimum space required to 20 bytes to accomodate $a0 - $a3 and some padding
                    space=24;
                }
                space+=args->spaceRequired();
                return space;
            }
            else    {
                return 24;
            }
        }
        
        virtual void print(std::ostream &dst) const override    {
            dst<<id<<"(";
            if(args!=nullptr)   {
                args->print(dst);
            }
            dst<<");"<<std::endl;
        }

        virtual void generate(std::ofstream &file, const char* destReg, Context *context) const override    {
            long initSL = context->stack.slider;    // store previous context
            long initSP = context->stack.size;
            long RAoffset = context->stack.slider;
            file<<"sw $ra, "<<(context->stack.size - RAoffset)<<"($sp)"<<std::endl;    // store $ra
            file<<"addiu $sp, $sp, -4"<<std::endl;
            context->stack.slider+=4;

            if(args!=nullptr)   {                           // load arguments (if any)
                context->ArgCount=0;
                args->generate(file, "$t0", context);
                context->ArgCount=0;
            }

            file<<".option pic0"<<std::endl;
            file<<"jal "<<id<<std::endl;                    // call function
            file<<"nop"<<std::endl;
            file<<".option pic2"<<std::endl;
            file<<"move "<<std::string(destReg)<<", $v0"<<std::endl;    // store return value into destReg

            context->stack.slider-=4;
            file<<"addiu $sp, $sp, 4"<<std::endl;
            file<<"lw $ra, "<<(context->stack.size - RAoffset)<<"($sp)"<<std::endl;     // retore value of $ra
            context->stack.slider = initSL;         // load previous context
            context->stack.size = initSP;
        }
};

class FunctionDef : public Program {    // function definition 
    private:
        std::string type; // return type of function
        std::string id; // name of function
        FunctionDefArgs *args=nullptr; //function arguments
        ProgramPtr action; //the scope of the function
    public:
        FunctionDef(std::string *_type, std::string *_id, FunctionDefArgs *_args, ProgramPtr _action) : type(*_type), id(*_id), args(_args), action(_action)    {
            delete _type;
            delete _id;
        }  

        virtual ~FunctionDef() {
            delete args;
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

        void print(std::ostream &dst) const override    {
            dst<<getType()<<" "<<getID()<<"(";
            if(args!=nullptr)   {
                args->print(dst);
            }
            dst<<") ";
            getAction()->print(dst);
        }

        virtual void generate(std::ofstream &file, const char* destReg, Context *context) const override    {
            long initSP = context->stack.size;                  // store initial context
            long initSL = context->stack.slider;
            long initFP = context->stack.FP;
            std::string initFuncEnd = context->FuncRetnPoint;
            context->isFunc=1;
            context->stack.FP = context->stack.size;                    // set FP tracker to start of current stack frame

            std::unordered_map<std::string,varInfo> tempScope;
            context->stack.lut.push_back(tempScope);            // create scope on variable table for function arguments

            // file << "   .text"<<std::endl;
            file << "   .align 2"<<std::endl;
            file << "   .globl	"<<getID()<<std::endl;
            file << "   .ent	"<<getID()<<std::endl;
            file << "   .type	"<<getID()<<", @function"<<std::endl;

            context->FuncRetnPoint = makeLabel("func_end");
            file<<getID()<<":"<<std::endl;                      // function start
         
            context->stack.size+=8;                            // allocate 8 bytes for $fp + padding, set context FP to old SP value
            context->stack.slider = context->stack.size;
            file<<"addiu $sp, $sp, -8"<<std::endl;             // allocate stack space for $fp
            file<<"sw $fp, 4($sp)"<<std::endl;
            file<<"move $fp, $sp"<<std::endl;

            std::unordered_map<std::string,functionInfo>::iterator it;  // add function to declared functions table
            it=context->ftable.find(getID());
            if(it==context->ftable.end())   {                           // function not already in table
                functionInfo tmp;
                context->ftable.insert(std::pair<std::string,functionInfo>(getID(),tmp));
                it=context->ftable.find(getID());
                it->second.returnType = getType();
            }            
            context->ftEntry = context->ftable.find(getID());
            if(args!=nullptr)   {                                       // load arguments info into variable scope table (if any)
                context->ArgCount=0;
                it->second.argCount = args->getCount();
                args->generate(file, "$t0", context);
                context->ArgCount=0;
            }

            action->generate(file, destReg, context);           // run function code

            file<<"move $sp, $fp"<<std::endl;                   // deallocate stack space for $fp
            file<<"lw $fp, 4($sp)"<<std::endl;
            file<<"addiu $sp, $sp, 8"<<std::endl;
            file<<"jr $ra"<<std::endl;                          // end of function, return to caller 
            file<<"nop"<<std::endl;

            file << "    .end     "<<getID()<<std::endl;
            file<<".size    "<<getID()<<", .-"<<getID()<<std::endl<<std::endl;
            context->stack.lut.pop_back();                      // clear function argument scope
            context->isFunc=0;                                  // reload iniital context
            context->FuncRetnPoint = initFuncEnd;
            context->stack.slider = initSL;
            context->stack.size = initSP;
            context->stack.FP = initFP;
        }

        // virtual void generate(std::ofstream &file, const char* destReg, Context *context) const override    {
        //     long initSP = context->stack.size;
        //     long initSL = context->stack.slider;
        //     long preSpace=4;                                    // space (in bytes) needed for FP and arguments
        //     std::string initFuncEnd = context->FuncRetnPoint;
        //     std::string returnPoint = makeLabel("func_end");
        //     context->FuncRetnPoint = returnPoint;
        //     context->isFunc=1;
        //     file<<getID()<<":"<<std::endl;                   // start of function
        //     context->stack.slider = context->stack.size;
        //     context->stack.size += preSpace;                 // allocate space for $fp (and arguments too [implement later])
        //     file<<"addiu $sp, $sp, -"<<preSpace<<std::endl;
        //     long initFP = context->stack.slider;
        //     context->stack.slider+=4;
        //     file<<"sw $fp, "<<(context->stack.size - initFP)<<"($sp)"<<std::endl;   // store $fp
        //     file<<"move $fp, $sp"<<std::endl;
        //     action->generate(file, destReg, context);   // run action
        //     file<<"move $sp, $fp"<<std::endl;
        //     file<<"lw $fp, "<<(context->stack.size - initFP)<<"($sp)"<<std::endl;
        //     file<<"addiu $sp, $sp, "<<preSpace<<std::endl;     // deallocate space used for FP and arguments
        //     file<<"jr $ra"<<std::endl;                         // end of function, return to caller 
        //     file<<"nop"<<std::endl;
        //     context->FuncRetnPoint = initFuncEnd;
        //     context->stack.slider = initSL;
        //     context->stack.size = initSP;
        // }
};

#endif