#ifndef COMPILER_AST_CONDITIONS_HPP
#define COMPILER_AST_CONDITIONS_HPP

#include "variable_table.hpp"

class Condition : public Program {
    private:
        ProgramPtr a;
        ProgramPtr b;
    protected:
        Condition(ProgramPtr _a, ProgramPtr _b) : a(_a), b(_b)  {}
    public:
        virtual ~Condition()    {
            delete a;
            delete b;
        }

        ProgramPtr getA() const {
            return a;
        }

        ProgramPtr getB() const {
            return b;
        }

        virtual long spaceRequired(Context *context) const override {
            long tmp=4;
            if(getA()!=nullptr) {
                tmp+=getA()->spaceRequired(context);
            }
            if(getB()!=nullptr) {
                tmp+=getB()->spaceRequired(context);
            }
            return tmp;
        }
};

class EqualTo : public Condition {      // a == b
    public:
        EqualTo(ProgramPtr _a, ProgramPtr _b) : Condition(_a,_b)   {}

        virtual void print(std::ostream &dst) const override    {
            getA()->print(dst);
            dst<<" == ";
            getB()->print(dst);
        }

        virtual void generate(std::ofstream &file, const char* destReg, Context *context) const override    {   // destReg = 1 if true, destReg = 0 if false
            long tmpOffset = context->stack.size - context->stack.slider;
            std::string type = getA()->getVarType(context);
            if (type == "float"||type == "double"){
                getA()->generate(file, "$f6", context);                             // store value of A into $t1
                if (type == "float"){
                    file<<"s.s $f6, "<<tmpOffset<<"($sp)"<<std::endl;
                    context->stack.slider+=4;
                } else {
                    file<<"s.d $f6, "<<tmpOffset-4<<"($sp)"<<std::endl;
                    context->stack.slider+=8;
                }
                getB()->generate(file, "$f8", context);                             // store value of B into $t2
                if (type == "float"){
                    file<<"l.s $f6, "<<tmpOffset<<"($sp)"<<std::endl;
                    context->stack.slider-=4;
                } else {
                    file<<"l.d $f6, "<<tmpOffset-4<<"($sp)"<<std::endl;
                    context->stack.slider-=8;
                }
                file<<"addiu $t0, $zero, 1"<<std::endl;      // addiu destReg, $zero, 1 (set destReg = 1)
                std::string tmpLabel=makeLabel("cond_EQ");
                file<<"c.eq.s $f6, $f8"<<std::endl;
                file<<"bc1t "<<tmpLabel<<std::endl;                        // beq $t1, $t2, tmpLabel (if A == B, skip zeroing of destReg)
                file<<"nop"<<std::endl;
                file<<"addiu $t0, $zero, 0"<<std::endl;      // addiu destReg, $zero, 0 (zero destReg) 
                file<<tmpLabel<<":"<<std::endl;
                file<<"move "<<std::string(destReg)<<", $t0"<<std::endl;
            } else {
            getA()->generate(file, "$t1", context);                             // store value of A into $t1
            file<<"sw $t1, "<<tmpOffset<<"($sp)"<<std::endl;
            context->stack.slider+=4;
            getB()->generate(file, "$t2", context);                             // store value of B into $t2
            file<<"lw $t1, "<<tmpOffset<<"($sp)"<<std::endl;
            context->stack.slider-=4;
            file<<"addiu $t0, $zero, 1"<<std::endl;      // addiu destReg, $zero, 1 (set destReg = 1)
            std::string tmpLabel=makeLabel("cond_EQ");
            file<<"beq $t1, $t2, "<<tmpLabel<<std::endl;                        // beq $t1, $t2, tmpLabel (if A == B, skip zeroing of destReg)
            file<<"nop"<<std::endl;
            file<<"addiu $t0, $zero, 0"<<std::endl;      // addiu destReg, $zero, 0 (zero destReg) 
            file<<tmpLabel<<":"<<std::endl;
            file<<"move "<<std::string(destReg)<<", $t0"<<std::endl;
            }
        }
};

class NotEqual : public Condition {     // a != b
    public:
        NotEqual(ProgramPtr _a, ProgramPtr _b) : Condition(_a,_b)   {}

        virtual void print(std::ostream &dst) const override    {
            getA()->print(dst);
            dst<<" != ";
            getB()->print(dst);
        }

        virtual void generate(std::ofstream &file, const char* destReg, Context *context) const override    {
            long tmpOffset = context->stack.size - context->stack.slider;
            getA()->generate(file, "$t1", context);                             // store value of A into $t1
            file<<"sw $t1, "<<tmpOffset<<"($sp)"<<std::endl;
            context->stack.slider+=4;
            getB()->generate(file, "$t2", context);                             // store value of B into $t2
            file<<"lw $t1, "<<tmpOffset<<"($sp)"<<std::endl;
            context->stack.slider-=4;
            file<<"addiu $t0, $zero, 1"<<std::endl;       // addiu destReg, $zero, 1 (set destReg = 1)
            std::string tmpLabel=makeLabel("cond_NEQ");
            file<<"bne $t1, $t2, "<<tmpLabel<<std::endl;                        // bne $t1, $t2, tmpLabel (if A != B, skip zeroing of destReg)
            file<<"nop"<<std::endl;
            file<<"addiu $t0, $zero, 0"<<std::endl;      // addiu destReg, $zero, 0 (zero destReg) 
            file<<tmpLabel<<":"<<std::endl;
            file<<"move "<<std::string(destReg)<<", $t0"<<std::endl;
        }
};

class GreaterThan : public Condition {  // a > b
    public:
        GreaterThan(ProgramPtr _a, ProgramPtr _b) : Condition(_a,_b)    {}

        virtual void print(std::ostream &dst) const override    {
            getA()->print(dst);
            dst<<" > ";
            getB()->print(dst);
        }

        virtual void generate(std::ofstream &file, const char* destReg, Context *context) const override {
            long tmpOffset = context->stack.size - context->stack.slider;
            getA()->generate(file, "$t1", context);                             // store value of A into $t1
            file<<"sw $t1, "<<tmpOffset<<"($sp)"<<std::endl;
            context->stack.slider+=4;
            getB()->generate(file, "$t2", context);                             // store value of B into $t2
            file<<"lw $t1, "<<tmpOffset<<"($sp)"<<std::endl;
            context->stack.slider-=4;
            file<<"addiu $t0, $zero, 1"<<std::endl;       // addiu destReg, $zero, 1 (set destReg = 1)
            std::string tmpLabel=makeLabel("cond_GR");
            file<<"bgt $t1, $t2, "<<tmpLabel<<std::endl;                        // bgt $t1, $t2, tmpLabel (if A != B, skip zeroing of destReg)
            file<<"nop"<<std::endl;
            file<<"addiu $t0, $zero, 0"<<std::endl;      // addiu destReg, $zero, 0 (zero destReg) 
            file<<tmpLabel<<":"<<std::endl;
            file<<"move "<<std::string(destReg)<<", $t0"<<std::endl;
        }
};

class GreaterEqual : public Condition { // a >= b
    public:
        GreaterEqual(ProgramPtr _a, ProgramPtr _b) : Condition(_a,_b)    {}

        virtual void print(std::ostream &dst) const override    {
            getA()->print(dst);
            dst<<" >= ";
            getB()->print(dst);
        }

        virtual void generate(std::ofstream &file, const char* destReg, Context *context) const override {
            long tmpOffset = context->stack.size - context->stack.slider;
            getA()->generate(file, "$t1", context);                             // store value of A into $t1
            file<<"sw $t1, "<<tmpOffset<<"($sp)"<<std::endl;
            context->stack.slider+=4;
            getB()->generate(file, "$t2", context);                             // store value of B into $t2
            file<<"lw $t1, "<<tmpOffset<<"($sp)"<<std::endl;
            context->stack.slider-=4;
            file<<"addiu $t0, $zero, 1"<<std::endl;      // addiu destReg, $zero, 1 (set destReg = 1)
            std::string tmpLabel=makeLabel("cond_GE");
            file<<"bge $t1, $t2, "<<tmpLabel<<std::endl;                        // bge $t1, $t2, tmpLabel (if A != B, skip zeroing of destReg)
            file<<"nop"<<std::endl;
            file<<"addiu $t0, $zero, 0"<<std::endl;      // addiu destReg, $zero, 0 (zero destReg) 
            file<<tmpLabel<<":"<<std::endl;
            file<<"move "<<std::string(destReg)<<", $t0"<< std::endl;
        }
};

class LessThan : public Condition {     // a < b
    public:
        LessThan(ProgramPtr _a, ProgramPtr _b) : Condition(_a,_b)   {}

        virtual void print(std::ostream &dst) const override    {
            getA()->print(dst);
            dst<<" < ";
            getB()->print(dst);
        }

        virtual void generate(std::ofstream &file, const char* destReg, Context *context) const override    {
            long tmpOffset = context->stack.size - context->stack.slider;
            getA()->generate(file, "$t1", context);                             // store value of A into $t1
            file<<"sw $t1, "<<tmpOffset<<"($sp)"<<std::endl;
            context->stack.slider+=4;
            getB()->generate(file, "$t2", context);                             // store value of B into $t2
            file<<"lw $t1, "<<tmpOffset<<"($sp)"<<std::endl;
            context->stack.slider-=4;
            file<<"addiu $t0, $zero, 1"<<std::endl;      // addiu destReg, $zero, 1 (set destReg = 1)
            std::string tmpLabel=makeLabel("cond_LT");
            file<<"blt $t1, $t2, "<<tmpLabel<<std::endl;                        // blt $t1, $t2, tmpLabel (if A != B, skip zeroing of destReg)
            file<<"nop"<<std::endl;
            file<<"addiu $t0, $zero, 0"<<std::endl;      // addiu destReg, $zero, 0 (zero destReg) 
            file<<tmpLabel<<":"<<std::endl;
            file<<"move "<<std::string(destReg)<<", $t0"<<std::endl;
        }
};

class LessEqual : public Condition {    // a <= b
    public:
        LessEqual(ProgramPtr _a, ProgramPtr _b) : Condition(_a,_b)   {}

        virtual void print(std::ostream &dst) const override    {
            getA()->print(dst);
            dst<<" <= ";
            getB()->print(dst);
        }

        virtual void generate(std::ofstream &file, const char* destReg, Context *context) const override    {
            long tmpOffset = context->stack.size - context->stack.slider;
            getA()->generate(file, "$t1", context);                             // store value of A into $t1
            file<<"sw $t1, "<<tmpOffset<<"($sp)"<<std::endl;
            context->stack.slider+=4;
            getB()->generate(file, "$t2", context);                             // store value of B into $t2
            file<<"lw $t1, "<<tmpOffset<<"($sp)"<<std::endl;
            context->stack.slider-=4;
            file<<"addiu $t0, $zero, 1"<<std::endl;      // addiu destReg, $zero, 1 (set destReg = 1)
            std::string tmpLabel=makeLabel("cond_LE");
            file<<"ble $t1, $t2, "<<tmpLabel<<std::endl;                        // ble $t1, $t2, tmpLabel (if A != B, skip zeroing of destReg)
            file<<"nop"<<std::endl;
            file<<"addiu $t0, $zero, 0"<<std::endl;      // addiu destReg, $zero, 0 (zero destReg) 
            file<<tmpLabel<<":"<<std::endl;
            file<<"move "<<std::string(destReg)<<", $t0"<<std::endl;
        }
};

class LogicalAND : public Condition {
    public:
        LogicalAND(ProgramPtr _a, ProgramPtr _b) : Condition(_a,_b) {}

        virtual void print(std::ostream &dst) const override    {
            getA()->print(dst);
            dst<<" && ";
            getB()->print(dst);
        }

        virtual void generate(std::ofstream &file, const char* destReg, Context *context) const override    {
            std::string endPoint=makeLabel("cond_end");
            file<<"addiu "<<std::string(destReg)<<", $zero, 0"<<std::endl;      // addiu destReg, $zero, 0 (zero destReg) 
            getA()->generate(file, "$t1", context);                             // evaluate A
            file<<"beq $t1, $zero, "<<endPoint<<std::endl;                      // if A == 0, jump to end (final destReg value is 0)
            file<<"nop"<<std::endl;
            getB()->generate(file, "$t2", context);                             // evaluate B
            file<<"beq $t2, $zero, "<<endPoint<<std::endl;                      // if B == 0, jump to end (final destReg value is 0)
            file<<"nop"<<std::endl;
            file<<"addiu "<<std::string(destReg)<<", $zero, 1"<<std::endl;      // addiu destReg, $zer0, 1 (set destReg to 1 if A and B are non-zero)
            file<<endPoint<<":"<<std::endl;
        }
};

class LogicalOR : public Condition {
    public:
        LogicalOR(ProgramPtr _a, ProgramPtr _b) : Condition(_a,_b)  {}

        virtual void print(std::ostream &dst) const override    {
            getA()->print(dst);
            dst<<" || ";
            getB()->print(dst);
        }

        virtual void generate(std::ofstream &file, const char* destReg, Context *context) const override    {
            std::string endPoint = makeLabel("cond_end");
            file<<"addiu "<<std::string(destReg)<<", $zero, 1"<<std::endl;      // addiu destReg, $zer0, 1 (set destReg to 1) 
            getA()->generate(file, "$t1", context);                             // evaluate A
            file<<"bne $t1, $zero, "<<endPoint<<std::endl;                        // if A != 0, jump to end (final destreg value is 1)
            file<<"nop"<<std::endl;
            getB()->generate(file, "$t2", context);                             // evalute B
            file<<"bne $t2, $zero, "<<endPoint<<std::endl;                        // if B != 0, jump to end (final destreg value is 1)
            file<<"nop"<<std::endl;
            file<<"addiu "<<std::string(destReg)<<", $zero, 0"<<std::endl;      // addiu destReg, $zero, 0 (zero destReg) 
            file<<endPoint<<":"<<std::endl;
        }
};

class LogicalNOT : public Condition {
    public:
        LogicalNOT(ProgramPtr _a) : Condition(_a,nullptr) {}

        virtual void print(std::ostream &dst) const override    {
            dst << "!";
            getA()->print(dst);
        }

        virtual void generate(std::ofstream &file, const char* destReg, Context *context) const override    {
            std::string endPoint = makeLabel("cond_not");
            file<<"addiu "<<std::string(destReg)<<", $zero, 0"<<std::endl;      // addiu destReg, $zer0, 1 (set destReg to 1) 
            getA()->generate(file, "$t1", context); 
            file<<"bne $t1, $zero, "<<endPoint<<std::endl;                        // if A != 0, jump to end (final destreg value is 1)
            file<<"nop"<<std::endl;
            file<<"addiu "<<std::string(destReg)<<", $zero, 1"<<std::endl;      // addiu destReg, $zero, 0 (zero destReg) 
            file<<endPoint<<":"<<std::endl;

        }
};

#endif