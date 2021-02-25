#ifndef COMPILER_AST_BRANCHES_HPP
#define COMPILER_AST_BRANCHES_HPP

#include "src/include/ast.hpp"

class Branch : public Program {
    private:
        ProgramPtr action=nullptr;
    protected:
        Branch(ProgramPtr _action) : action(_action)    {}
    public:
        virtual ~Branch()   {
            delete action;
        }

        ProgramPtr getAction() const    {
            return action;
        }
};

class IfBlock : public Branch {
    private:
        ProgramPtr cond;
        ProgramPtr elsePtr=nullptr;
    public:
        IfBlock(ProgramPtr _condition, ProgramPtr _action, ProgramPtr _elsePtr) : Branch(_action), cond(_condition), elsePtr(_elsePtr)  {}

        ~IfBlock()  {
            delete cond;
            delete elsePtr;
        }

        ProgramPtr getCondition() const {
            return cond;
        }

        ProgramPtr getElse() const  {
            return elsePtr;
        }

        virtual void print(std::ostream &dst) const override    {
            dst<<"if( ";
            getCondition()->print(dst);
            dst<<" )";
            if(getAction()!=nullptr)    {
                getAction()->print(dst);
            }
            else    {
                dst<<"  {}"<<std::endl;
            }
            if(getElse()!=nullptr)  {
                getElse()->print(dst);
            }            
        }
};

class ElseBlock : public Branch {
    public:
        ElseBlock(ProgramPtr _action) : Branch(_action) {}

        virtual void print(std::ostream &dst) const override    {
            dst<<"else ";
            if(getAction()!=nullptr)    {
                getAction()->print(dst);
            }
            else    {
                dst<<"  {}"<<std::endl;
            }
        }
};

#endif