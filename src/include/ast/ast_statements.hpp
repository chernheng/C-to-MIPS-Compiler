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
};

class BreakStatement : public Program {
    public:
        virtual void print(std::ostream &dst) const override    {
            dst<<"break;"<<std::endl;
        }
};

class ContinueStatement : public Program {
    public:
        virtual void print(std::ostream &dst) const override    {
            dst<<"continue;"<<std::endl;
        }
};

#endif