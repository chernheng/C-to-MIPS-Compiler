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
        int ptr = 0;
    public:
        FunctionDefArgs(std::string *_type, std::string *_id, FunctionArgs *_next, int _ptr) : FunctionArgs(nullptr, _next), type(*_type), id(*_id), ptr(_ptr) {
            delete _type;
            delete _id;
        }

        virtual long spaceRequired(Context *context) const override {
            long tmp=0;
            if(ptr==1)  {
                tmp=4;
            }
            else if(type=="int" || type=="char" ||type=="float")  {
                tmp=4;
            }
            else if(type=="double"){
                tmp=8;
            }
            if(next!=nullptr)   {
                tmp+=next->spaceRequired(context);
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
            vf.length=1;
            if((type=="float"||type=="double")&&vf.isPtr==0){
                long delta = context->stack.FP - context->ArgOffset;
                vf.offset=delta;
                vf.type=type;
                vf.isPtr = ptr;
                vf.isFP =1;
                if(type=="double")   {
                    vf.numBytes=8;
                }
                else    {
                    vf.numBytes=4;
                }
                context->stack.lut.back().insert(std::pair<std::string,varInfo>(id,vf));
                context->ftEntry->second.argList.push_back(vf);
                if ((context->ArgCount>0 && context->totalArgCount<4)||(context->FPArgCount>1 && context->totalArgCount<4)){
                    if(type=="float") {
                        file<<"sw $a"<<context->totalArgCount<<", "<<(context->stack.size - delta)<<"($sp)"<<std::endl;
                    } else if (type=="double" && context->totalArgCount<3){
                        file<<"sw $a"<<context->totalArgCount+1<<", "<<(context->stack.size - delta+4)<<"($sp)"<<std::endl;
                        file<<"sw $a"<<context->totalArgCount<<", "<<(context->stack.size - delta)<<"($sp)"<<std::endl;
                    }
                }
                else if(context->FPArgCount<2) {
                    if(type == "float")  {
                        file<<"s.s $f"<<(context->FPArgCount*2)+12<<", "<<(context->stack.size - delta)<<"($sp)"<<std::endl;
                        context->ArgOffset+=4;
                    }
                    else    {
                        file<<"s.d $f"<<(context->FPArgCount*2)+12<<", "<<(context->stack.size - delta)<<"($sp)"<<std::endl;
                        context->ArgOffset+=8;
                    }                
                }
                context->FPArgCount++;
                context->totalArgCount++;
            } 
            else {
                long delta = context->stack.FP - context->ArgOffset;
                vf.offset=delta;
                vf.type=type;
                vf.isPtr = ptr;
                if(type=="char")   {
                    vf.numBytes=1;
                }
                else    {
                    vf.numBytes=4;
                }
                context->stack.lut.back().insert(std::pair<std::string,varInfo>(id,vf));
                context->ftEntry->second.argList.push_back(vf);
                if(context->FPArgCount>0 && context->ArgOffset<12){
                    if(vf.numBytes==1 && vf.isPtr==0)  {
                        file<<"sb $a"<<context->totalArgCount<<", "<<(context->stack.size - delta)<<"($sp)"<<std::endl;
                    }
                    else    {
                        file<<"sw $a"<<context->totalArgCount<<", "<<(context->stack.size - delta)<<"($sp)"<<std::endl;
                    }     
                }
                else if(context->ArgCount<4) {
                    if(vf.numBytes==1 && vf.isPtr==0)  {
                        file<<"sb $a"<<context->ArgCount<<", "<<(context->stack.size - delta)<<"($sp)"<<std::endl;
                    }
                    else    {
                        file<<"sw $a"<<context->ArgCount<<", "<<(context->stack.size - delta)<<"($sp)"<<std::endl;
                    }                
                }
                context->ArgCount++;
                context->ArgOffset+=4;
                context->totalArgCount++;
            }
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
                dst<<",";
                next->print(dst);
            }
        }

        virtual long spaceRequired(Context *context) const override {
            long tmp = action->spaceRequired(context);
            if(next!=nullptr)   {
                tmp+=next->spaceRequired(context);
            }
            return tmp;
        }

        virtual void generate(std::ofstream &file, const char* destReg, Context *context) const override    {
            std::string type = action->getVarType(context);
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
        
        virtual long spaceRequired(Context *context) const override {
            if(args!=nullptr)   {
                long count = args->getCount();
                long space = (4*count)+8;
                if(space<32)    {               // set minimum space required to 20 bytes to accomodate $a0 - $a3 and some padding
                    space=32;
                }
                space+=args->spaceRequired(context);
                return space;
            }
            else    {
                return 32;
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
            // long RAoffset = context->stack.slider;
            long RAoffset = 24;
            if(args!=nullptr)   {
                if(args->getCount()>4)  {
                    RAoffset = (4*args->getCount())+4;
                }
            }            
            // file<<"sw $ra, "<<(context->stack.size - RAoffset)<<"($sp)"<<std::endl;    // store $ra
            file<<"sw $ra, "<<RAoffset<<"($sp)"<<std::endl;

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

            // file<<"lw $ra, "<<(context->stack.size - RAoffset)<<"($sp)"<<std::endl;     // retore value of $ra
            file<<"lw $ra, "<<RAoffset<<"($sp)"<<std::endl;
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

            file << "   .text"<<std::endl;
            file << "   .align 2"<<std::endl;
            file << "   .globl	"<<getID()<<std::endl;
            file << "   .ent	"<<getID()<<std::endl;
            file << "   .type	"<<getID()<<", @function"<<std::endl;

            context->FuncRetnPoint = makeLabel("func_end");
            file<<getID()<<":"<<std::endl;                      // function start
            file<<".set noreorder"<<std::endl;
         
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
                context->FPArgCount=0;
                context->ArgOffset=0;
                context->totalArgCount =0;
                it->second.argCount = args->getCount();
                args->generate(file, "$t0", context);
                context->ArgCount=0;
                context->FPArgCount=0;
                context->ArgOffset=0;
                context->totalArgCount =0;
            }

            if (type == "float" || type == "double" ){
                action->generate(file, "$f0", context);
            } else {
                action->generate(file, destReg, context);           // run function code
            }

            file<<"move $sp, $fp"<<std::endl;                   // deallocate stack space for $fp
            file<<"lw $fp, 4($sp)"<<std::endl;
            file<<"addiu $sp, $sp, 8"<<std::endl;
            file<<"jr $ra"<<std::endl;                          // end of function, return to caller 
            file<<"nop"<<std::endl;

            file<<".set reorder"<<std::endl;
            file << "    .end     "<<getID()<<std::endl;
            file<<".size    "<<getID()<<", .-"<<getID()<<std::endl<<std::endl;
            context->stack.lut.pop_back();                      // clear function argument scope
            context->isFunc=0;                                  // reload iniital context
            context->FuncRetnPoint = initFuncEnd;
            context->stack.slider = initSL;
            context->stack.size = initSP;
            context->stack.FP = initFP;
            if (context->FP.size()!=0){
                file << "     .data" << std::endl;
            }
            while (context->FP.size() != 0) {
                varInfo temp = context->FP.back();
                if (temp.numBytes== 8){
                    file << temp.FP_label << ":   .double " <<temp.FP_value<<std::endl;
                }else if(temp.numBytes ==4){
                    file << temp.FP_label << ":   .float " <<temp.FP_value<<std::endl;
                }
                context->FP.pop_back();
            }
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