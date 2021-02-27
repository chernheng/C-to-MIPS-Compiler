#ifndef COMPILER_AST_OPERATORS_HPP
#define COMPILER_AST_OPERATORS_HPP

#include "variable_table.hpp"

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

        virtual void generate(std::ofstream &file, const char* destReg, Context &context) const override    {
            long offset=getLeft()->getOffset(context);
            std::string t=getLeft()->getVarType(context);
            getRight()->generate(file, "$t0", context);
            if(t=="int")    {
                file<<"sw $t0, "<<offset<<"($sp)"<<std::endl;
            }
            else if(t=="char")  {
                file<<"sb $t0, "<<offset<<"($sp)"<<std::endl;
            }
            file<<"li "<<std::string(destReg)<<", 1"<<std::endl;
        }
};

class AddOperator : public Operator {
    protected:
        virtual const char *getOpcode() const override  {
            return "+";
        }
    public:
        AddOperator(ProgramPtr _left, ProgramPtr _right) : Operator(_left,_right)   {}

        virtual void generate(std::ofstream &file, const char* destReg, Context &context) const override    {
            getLeft()->generate(file, "$t1", context);
            file<<"addiu $sp, $sp, -4"<<std::endl;
            file<<"sw $t1, 4($sp)"<<std::endl;
            getRight()->generate(file, "$t2", context);
            file<<"lw $t1, 4($sp)"<<std::endl;
            file<<"addiu $sp, $sp, 4"<<std::endl;
            file<<"addu "<<std::string(destReg)<<", $t1, $t2"<<std::endl;
        }
};

class SubOperator : public Operator {
    protected:
        virtual const char *getOpcode() const override  {
            return "-";
        }
    public:
        SubOperator(ProgramPtr _left, ProgramPtr _right) : Operator(_left,_right)  {}

        virtual void generate(std::ofstream &file, const char* destReg, Context &context) const override    {
            getLeft()->generate(file, "$t1", context);
            file<<"addiu $sp, $sp, -4"<<std::endl;
            file<<"sw $t1, 4($sp)"<<std::endl;
            getRight()->generate(file, "$t2", context);
            file<<"lw $t1, 4($sp)"<<std::endl;
            file<<"addiu $sp, $sp, 4"<<std::endl;
            file<<"subu "<<std::string(destReg)<<", $t1, $t2"<<std::endl;
        }
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
            dst<<"DEREF";
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

class BitNOTOperator : public Operator {
    protected:
        virtual const char *getOpcode() const override    {
            return "~";
        }  
    public:
        BitNOTOperator(ProgramPtr _left) : Operator(_left,nullptr)  {}

        virtual void print(std::ostream &dst) const override    {
            dst<<"~";
            getLeft()->print(dst);
        }
};

class NegOperator : public Operator {
    protected:
        virtual const char *getOpcode() const override    {
            return "-";
        }  
    public:
        NegOperator(ProgramPtr _left) : Operator(_left,nullptr)  {}

        virtual void print(std::ostream &dst) const override    {
            dst<<"-";
            getLeft()->print(dst);
        }
};

class LeftShiftOperator : public Operator {
    protected:
        virtual const char *getOpcode() const override  {
            return "<<";
        }
    public:
        LeftShiftOperator(ProgramPtr _left, ProgramPtr _right) : Operator(_left,_right)   {}
};

class RightShiftOperator : public Operator {
    protected:
        virtual const char *getOpcode() const override  {
            return ">>";
        }
    public:
        RightShiftOperator(ProgramPtr _left, ProgramPtr _right) : Operator(_left,_right)    {}
};

class IncOperator : public Operator {
    protected:
        virtual const char *getOpcode() const override  {
            return "++";
        }  
    public:
        IncOperator(ProgramPtr _left) : Operator(_left,nullptr) {}

        virtual void print(std::ostream &dst) const override    {
            getLeft()->print(dst);
            dst<<"++";
        }
};

class DecOperator : public Operator {
    protected:
        virtual const char *getOpcode() const override  {
            return "--";
        }  
    public:
        DecOperator(ProgramPtr _left) : Operator(_left,nullptr) {}

        virtual void print(std::ostream &dst) const override    {
            getLeft()->print(dst);
            dst<<"--";
        }
};

#endif