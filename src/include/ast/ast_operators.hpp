#ifndef COMPILER_AST_OPERATORS_HPP
#define COMPILER_AST_OPERATORS_HPP

#include "src/include/ast.hpp"

class Operator : public Program {
    private:
        ProgramPtr left;
        ProgramPtr right;
    protected:
        Operator(ProgramPtr _left, ProgramPtr _right) : left(_left), right(_right)  {}    
    public:
        virtual ~Operator() {
            delete left;
            delete right;
        }

        ProgramPtr getLeft() const  {
            return left;
        }

        ProgramPtr getRight()const  {
            return right;
        }

        virtual const char *getOpcode() const =0;

        virtual void print(std::ostream &dst) const override    {
            getLeft()->print(dst);
            dst<<" "<<getOpcode()<<" ";
            getRight()->print(dst);            
        }
};

class AssignmentOperator : public Operator {
    protected:
        virtual const char *getOpcode() const override  {
            return "=";
        }
    public:
        AssignmentOperator(ProgramPtr _left, ProgramPtr _right) : Operator(_left,_right)    {}
};

class AddOperator : public Operator {
    protected:
        virtual const char *getOpcode() const override  {
            return "+";
        }
    public:
        AddOperator(ProgramPtr _left, ProgramPtr _right) : Operator(_left,_right)   {}
};

class SubOperator : public Operator {
    protected:
        virtual const char *getOpcode() const override  {
            return "-";
        }
    public:
        SubOperator(ProgramPtr _left, ProgramPtr _right) : Operator(_left,_right)  {}
};

class MulOperator : public Operator {
    protected:
        virtual const char *getOpcode() const override  {
            return "*";
        }
    public:
        MulOperator(ProgramPtr _left, ProgramPtr _right) : Operator(_left,_right)   {}
};

class DivOperator : public Operator {
    protected:
        virtual const char *getOpcode() const override  {
            return "/";
        }
    public:
        DivOperator(ProgramPtr _left, ProgramPtr _right) : Operator(_left,_right)   {}
};

class ModuloOperator : public Operator {
    protected:
        virtual const char *getOpcode() const override  {
            return "%";
        }
    public:
        ModuloOperator(ProgramPtr _left, ProgramPtr _right) : Operator(_left,_right)    {}
};

class RefOperator : public Operator {
    protected:
        virtual const char *getOpcode() const override  {
            return "&";
        }
    public:
        RefOperator(ProgramPtr _left) : Operator(_left,nullptr) {}

        virtual void print(std::ostream &dst) const override    {
            dst<<"&";
            getLeft()->print(dst);
        }
};

class DerefOperator : public Operator {
    protected:
        virtual const char *getOpcode() const override  {
            return "*";
        }
    public:
        DerefOperator(ProgramPtr _val) : Operator(_val,nullptr) {}

        virtual void print(std::ostream &dst) const override    {
            dst<<"*";
            getLeft()->print(dst);
        }
};

class BitANDOperator : public Operator {
    protected:
        virtual const char *getOpcode() const override  {
            return "&";
        }
    public:
        BitANDOperator(ProgramPtr _left, ProgramPtr _right) : Operator(_left,_right)    {}
};

class BitOROperator : public Operator {
    protected:
        virtual const char *getOpcode() const override     {
            return "|";
        }
    public:
        BitOROperator(ProgramPtr _left, ProgramPtr _right) : Operator(_left,_right)     {}
};

class BitXOROperator : public Operator {
    protected:
        virtual const char *getOpcode() const override  {
            return "^";
        }
    public:
        BitXOROperator(ProgramPtr _left, ProgramPtr _right) : Operator(_left,_right)    {}
};

class BitNOTOperator : Operator {
    protected:
        virtual const char *getOpcode() const override    {
            return "!";
        }  
    public:
        BitNOTOperator(ProgramPtr _left) : Operator(_left,nullptr)  {}

        virtual void print(std::ostream &dst) const override    {
            dst<<"!";
            getLeft()->print(dst);
        }
};

#endif