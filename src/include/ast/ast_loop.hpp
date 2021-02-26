#ifndef COMPILER_AST_LOOP_HPP
#define COMPILER_AST_LOOP_HPP

#include "src/include/ast.hpp"

class Loop : public Program {
    private:
        ProgramPtr condition;
        ProgramPtr action; //is the scope of the loop, or just 1 line
    protected:
        Loop(ProgramPtr _condition, ProgramPtr _action) : condition(_condition), action(_action)    {}  
    public:
        virtual ~Loop() {
            delete condition;
            delete action;
        }

        ProgramPtr getCondition() const {
            return condition;
        }

        ProgramPtr getAction() const    {
            return action;
        }
};

class WhileLoop : public Loop {
    public:
        WhileLoop(ProgramPtr _condition, ProgramPtr _action) : Loop(_condition, _action)    {}

        virtual void print(std::ostream &dst) const override    {
            dst<<"while(";
            getCondition()->print(dst);
            dst<<") {"<<std::endl;
            getAction()->print(dst);
            dst<<"}"<<std::endl;
        }
};

class ForLoop : public Loop  {
    public:
        ForLoop(ProgramPtr _condition, ProgramPtr _action) : Loop(_condition, _action)  {}

        virtual void print(std::ostream &dst) const override    {
            dst<<"for(";
            getCondition()->print(dst);
            dst<<") {"<<std::endl;
            getAction()->print(dst);
            dst<<"}"<<std::endl;
        }
};

#endif