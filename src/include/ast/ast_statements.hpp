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

        virtual long spaceRequired() const override {
            if(getAction()!=nullptr)    {
                return getAction()->spaceRequired();
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
            if(context->LoopEndPoint!="")  {
                file<<"addiu $sp, $sp, "<<(context->stack.size - context->LoopInitSP)<<std::endl;
                file<<"b "<<context->LoopEndPoint<<std::endl;
                file<<"nop"<<std::endl;
                context->stack.size = context->LoopInitSP;
                context->stack.slider = context->LoopInitSL;
                if(context->LoopScopeCount>0)   {
                    for(int i=0;i<context->LoopScopeCount;i++)  {
                        context->stack.lut.pop_back();
                    }
                }
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
                file<<"b "<<context->LoopStartPoint<<std::endl;
                file<<"nop"<<std::endl;
            }
        }
};

#endif