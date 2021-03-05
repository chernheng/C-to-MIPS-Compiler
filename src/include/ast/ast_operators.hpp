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

        virtual long spaceRequired() const override {
            long tmp=4;
            if(left!=nullptr)   {
                tmp+=left->spaceRequired();
            }
            if(right!=nullptr)  {
                tmp+=right->spaceRequired();
            }
            return tmp;
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

        virtual long spaceRequired() const override  {   // assignmenr operator does not need any temporary stack space
            return 0;
        }

        virtual void generate(std::ofstream &file, const char* destReg, Context *context) const override    {
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

        virtual void generate(std::ofstream &file, const char* destReg, Context *context) const override    {
            getLeft()->generate(file, "$t1", context);
            long ofs = context->stack.slider;
            file<<"sw $t1, "<<(context->stack.size - ofs)<<"($sp)"<<std::endl;
            context->stack.slider+=4;
            getRight()->generate(file, "$t2", context);
            file<<"lw $t1, "<<(context->stack.size - ofs)<<"($sp)"<<std::endl;
            context->stack.slider-=4;
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

        virtual void generate(std::ofstream &file, const char* destReg, Context *context) const override    {
            getLeft()->generate(file, "$t1", context);
            long ofs = context->stack.slider;
            file<<"sw $t1, "<<(context->stack.size - ofs)<<"($sp)"<<std::endl;
            context->stack.slider+=4;
            getRight()->generate(file, "$t2", context);
            file<<"lw $t1, "<<(context->stack.size - ofs)<<"($sp)"<<std::endl;
            context->stack.slider-=4;
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

        virtual void generate(std::ofstream &file, const char* destReg, Context *context) const override    {
            getLeft()->generate(file, "$t1", context);
            long ofs = context->stack.slider;
            file<<"sw $t1, "<<(context->stack.size - ofs)<<"($sp)"<<std::endl;
            context->stack.slider+=4;
            getRight()->generate(file, "$t2", context);
            file<<"lw $t1, "<<(context->stack.size - ofs)<<"($sp)"<<std::endl;
            context->stack.slider-=4;
            file<<"mult $t1, $t2"<<std::endl;
            file<<"mflo "<<std::string(destReg)<<std::endl;
        }
};

class DivOperator : public Operator {
    protected:
        virtual const char *getOpcode() const override  {
            return "/";
        }
    public:
        DivOperator(ProgramPtr _left, ProgramPtr _right) : Operator(_left,_right)   {}
        
        virtual void generate(std::ofstream &file, const char* destReg, Context *context) const override    {
            getLeft()->generate(file, "$t1", context);
            long ofs = context->stack.slider;
            file<<"sw $t1, "<<(context->stack.size - ofs)<<"($sp)"<<std::endl;
            context->stack.slider+=4;
            getRight()->generate(file, "$t2", context);
            file<<"lw $t1, "<<(context->stack.size - ofs)<<"($sp)"<<std::endl;
            context->stack.slider-=4;
            file<<"div $t1, $t2"<<std::endl;
            file<<"mflo "<<std::string(destReg)<<std::endl;
        }
};

class ModuloOperator : public Operator {
    protected:
        virtual const char *getOpcode() const override  {
            return "%";
        }
    public:
        ModuloOperator(ProgramPtr _left, ProgramPtr _right) : Operator(_left,_right)    {}

        virtual void generate(std::ofstream &file, const char* destReg, Context *context) const override    {
            getLeft()->generate(file, "$t1", context);
            long ofs = context->stack.slider;
            file<<"sw $t1, "<<(context->stack.size - ofs)<<"($sp)"<<std::endl;
            context->stack.slider+=4;
            getRight()->generate(file, "$t2", context);
            file<<"lw $t1, "<<(context->stack.size - ofs)<<"($sp)"<<std::endl;
            context->stack.slider-=4;
            file<<"div $t1, $t2"<<std::endl;
            file<<"mfhi "<<std::string(destReg)<<std::endl;
        }
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

        virtual void generate(std::ofstream &file, const char* destReg, Context *context) const override    {
            long offset = getLeft()->getOffset(context);
            file<<"addiu "<<std::string(destReg)<<", $sp, "<<offset<<std::endl;
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

        virtual void generate(std::ofstream &file, const char* destReg, Context *context) const override    {
            getLeft()->generate(file, "$t1", context);
            file<<"lw "<<std::string(destReg)<<", 0($t1)"<<std::endl;
        }
};

class BitANDOperator : public Operator {
    protected:
        virtual const char *getOpcode() const override  {
            return "&";
        }
    public:
        BitANDOperator(ProgramPtr _left, ProgramPtr _right) : Operator(_left,_right)    {}

        virtual void generate(std::ofstream &file, const char* destReg, Context *context) const override    {
            getLeft()->generate(file, "$t1", context);
            long ofs = context->stack.slider;
            file<<"sw $t1, "<<(context->stack.size - ofs)<<"($sp)"<<std::endl;
            context->stack.slider+=4;
            getRight()->generate(file, "$t2", context);
            file<<"lw $t1, "<<(context->stack.size - ofs)<<"($sp)"<<std::endl;
            context->stack.slider-=4;
            file<<"and "<<std::string(destReg)<<", $t1, $t2"<<std::endl;
        }
};

class BitOROperator : public Operator {
    protected:
        virtual const char *getOpcode() const override     {
            return "|";
        }
    public:
        BitOROperator(ProgramPtr _left, ProgramPtr _right) : Operator(_left,_right)     {}

        virtual void generate(std::ofstream &file, const char* destReg, Context *context) const override    {
            getLeft()->generate(file, "$t1", context);
            long ofs = context->stack.slider;
            file<<"sw $t1, "<<(context->stack.size - ofs)<<"($sp)"<<std::endl;
            context->stack.slider+=4;
            getRight()->generate(file, "$t2", context);
            file<<"lw $t1, "<<(context->stack.size - ofs)<<"($sp)"<<std::endl;
            context->stack.slider-=4;
            file<<"or "<<std::string(destReg)<<", $t1, $t2"<<std::endl;
        }
};

class BitXOROperator : public Operator {
    protected:
        virtual const char *getOpcode() const override  {
            return "^";
        }
    public:
        BitXOROperator(ProgramPtr _left, ProgramPtr _right) : Operator(_left,_right)    {}

        virtual void generate(std::ofstream &file, const char* destReg, Context *context) const override    {
            getLeft()->generate(file, "$t1", context);
            long ofs = context->stack.slider;
            file<<"sw $t1, "<<(context->stack.size - ofs)<<"($sp)"<<std::endl;
            context->stack.slider+=4;
            getRight()->generate(file, "$t2", context);
            file<<"lw $t1, "<<(context->stack.size - ofs)<<"($sp)"<<std::endl;
            context->stack.slider-=4;
            file<<"xor "<<std::string(destReg)<<", $t1, $t2"<<std::endl;
        }
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

        virtual void generate(std::ofstream &file, const char* destReg, Context *context) const override    {
            getLeft()->generate(file, destReg, context);
            file<<"nor "<<std::string(destReg)<<", "<<std::string(destReg)<<", "<<std::string(destReg)<<std::endl;
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

        virtual void generate(std::ofstream &file, const char* destReg, Context *context) const override    {
            getLeft()->generate(file, "$t1", context);
            file<<"subu "<<std::string(destReg)<<", $zero, $t1"<<std::endl;
        }
};

class LeftShiftOperator : public Operator {
    protected:
        virtual const char *getOpcode() const override  {
            return "<<";
        }
    public:
        LeftShiftOperator(ProgramPtr _left, ProgramPtr _right) : Operator(_left,_right)   {}

        virtual void generate(std::ofstream &file, const char* destReg, Context *context) const override    {
            getLeft()->generate(file, "$t1", context);
            long ofs = context->stack.slider;
            file<<"sw $t1, "<<(context->stack.size - ofs)<<"($sp)"<<std::endl;
            context->stack.slider+=4;
            getRight()->generate(file, "$t2", context);
            file<<"lw $t1, "<<(context->stack.size - ofs)<<"($sp)"<<std::endl;
            context->stack.slider-=4;
            file<<"sllv "<<std::string(destReg)<<", $t1, $t2"<<std::endl;
        }
};

class RightShiftOperator : public Operator {
    protected:
        virtual const char *getOpcode() const override  {
            return ">>";
        }
    public:
        RightShiftOperator(ProgramPtr _left, ProgramPtr _right) : Operator(_left,_right)    {}

        virtual void generate(std::ofstream &file, const char* destReg, Context *context) const override    {
            getLeft()->generate(file, "$t1", context);
            long ofs = context->stack.slider;
            file<<"sw $t1, "<<(context->stack.size - ofs)<<"($sp)"<<std::endl;
            context->stack.slider+=4;
            getRight()->generate(file, "$t2", context);
            file<<"lw $t1, "<<(context->stack.size - ofs)<<"($sp)"<<std::endl;
            context->stack.slider-=4;
            file<<"srav "<<std::string(destReg)<<", $t1, $t2"<<std::endl;
        }
};

class IncOperator : public Operator {   // i++
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

        virtual void generate(std::ofstream &file, const char* destReg, Context *context) const override    {
            getLeft()->generate(file, "$t0", context);
            file<<"addiu $t1, $t0, 1"<<std::endl;
            if(getLeft()->getVarType(context)=="int")  {
                file<<"sw $t1, "<<getLeft()->getOffset(context)<<"($sp)"<<std::endl;
            }
            else if(getLeft()->getVarType(context)=="char")    {
                file<<"sb $t1, "<<getLeft()->getOffset(context)<<"($sp)"<<std::endl;
            }
            file<<"move "<<std::string(destReg)<<", $t0"<<std::endl;
        }
};

class DecOperator : public Operator {   // i--
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

        virtual void generate(std::ofstream &file, const char* destReg, Context *context) const override    {
            getLeft()->generate(file, "$t0", context);
            file<<"move "<<std::string(destReg)<<", $t0"<<std::endl;
            file<<"addiu $t1, $t0, -1"<<std::endl;
            if(getLeft()->getVarType(context)=="int")  {
                file<<"sw $t1, "<<getLeft()->getOffset(context)<<"($sp)"<<std::endl;
            }
            else if(getLeft()->getVarType(context)=="char")    {
                file<<"sb $t1, "<<getLeft()->getOffset(context)<<"($sp)"<<std::endl;
            }
            file<<"move "<<std::string(destReg)<<", $t0"<<std::endl;
        }
};

class IncAfterOperator : public Operator {   // ++i
    protected:
        virtual const char *getOpcode() const override  {
            return "++";
        }  
    public:
        IncAfterOperator(ProgramPtr _left) : Operator(_left,nullptr) {}

        virtual void print(std::ostream &dst) const override    {
            dst<<"++";
            getLeft()->print(dst);
        }

        virtual void generate(std::ofstream &file, const char* destReg, Context *context) const override    {
            getLeft()->generate(file, "$t0", context);
            file<<"addiu $t0, $t0, 1"<<std::endl;
            if(getLeft()->getVarType(context)=="int")  {
                file<<"sw $t0, "<<getLeft()->getOffset(context)<<"($sp)"<<std::endl;
            }
            else if(getLeft()->getVarType(context)=="char")    {
                file<<"sb $t0, "<<getLeft()->getOffset(context)<<"($sp)"<<std::endl;
            }
            file<<"move "<<std::string(destReg)<<", $t0"<<std::endl;
        }
};

class DecAfterOperator : public Operator {   // --i
    protected:
        virtual const char *getOpcode() const override  {
            return "--";
        }  
    public:
        DecAfterOperator(ProgramPtr _left) : Operator(_left,nullptr) {}

        virtual void print(std::ostream &dst) const override    {
            dst<<"--";
            getLeft()->print(dst);
        }

        virtual void generate(std::ofstream &file, const char* destReg, Context *context) const override    {
            getLeft()->generate(file, "$t0", context);
            file<<"addiu $t0, $t0, -1"<<std::endl;
            if(getLeft()->getVarType(context)=="int")  {
                file<<"sw $t0, "<<getLeft()->getOffset(context)<<"($sp)"<<std::endl;
            }
            else if(getLeft()->getVarType(context)=="char")    {
                file<<"sb $t0, "<<getLeft()->getOffset(context)<<"($sp)"<<std::endl;
            }
            file<<"move "<<std::string(destReg)<<", $t0"<<std::endl;
        }
};

#endif