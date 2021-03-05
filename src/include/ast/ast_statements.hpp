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
                    getAction()->generate(file, "$v0", context);
                    file<<"move "<<std::string(destReg)<<", $v0"<<std::endl;
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