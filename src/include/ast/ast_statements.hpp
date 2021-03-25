#ifndef COMPILER_AST_STATEMENTS_HPP
#define COMPILER_AST_STATEMENTS_HPP

class ReturnStatement : public Program {
    private:
        ProgramPtr action=nullptr;
    public:
        ReturnStatement(ProgramPtr _action) : action(_action)    {}

        ReturnStatement()    {}

        ~ReturnStatement()   {
            delete action;
        }

        ProgramPtr getAction() const    {
            return action;
        }

        virtual void print(std::ostream &dst) const override    {
            dst<<"return ";
            if(action!=nullptr) {
                getAction()->print(dst);
            }
            else    {
                dst<<";"<<std::endl;
            }
        }

        virtual long spaceRequired(Context *context) const override {
            if(getAction()!=nullptr)    {
                return getAction()->spaceRequired(context);
            }
            else    {
                return 0;
            }
        }

        virtual void generate(std::ofstream &file, const char* destReg, Context *context) const override    {
            if(context->FuncRetnPoint!="")  {                
                if(getAction()!=nullptr)    {
                    getAction()->generate(file, destReg, context);
                }
                file<<"b "<<context->FuncRetnPoint<<std::endl;
                file<<"nop"<<std::endl;
            }
        }
};

class BreakStatement : public Program {
    public:
        virtual void print(std::ostream &dst) const override    {
            dst<<"break;"<<std::endl;
        }

        virtual void generate(std::ofstream &file, const char* destReg, Context *context) const override    {
            if(context->BranchEndPoint!="" && context->isSwitch == 1) {
                file<<"b "<<context->BranchEndPoint<<std::endl;
                file<<"nop"<<std::endl;
            }
            if(context->LoopEndPoint!="" && context->isLoop ==1)  {
                file<<"addiu $sp, $sp, "<<(context->stack.size - context->LoopInitSP)<<std::endl;
                file<<"b "<<context->LoopEndPoint<<std::endl;
                file<<"nop"<<std::endl;
            }
        }
};

class ContinueStatement : public Program {
    public:
        virtual void print(std::ostream &dst) const override    {
            dst<<"continue;"<<std::endl;
        }

        virtual void generate(std::ofstream &file, const char* destReg, Context *context) const override    {
            if(context->LoopStartPoint!="") {
                file<<"addiu $sp, $sp, "<<(context->stack.size - context->LoopInitSP)<<std::endl;
                file<<"b "<<context->LoopStartPoint<<std::endl;
                file<<"nop"<<std::endl;
            }
        }
};

#endif