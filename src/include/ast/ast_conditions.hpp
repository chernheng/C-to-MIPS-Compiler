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

        virtual long spaceRequired() const override {
            long tmp=4;
            if(getA()!=nullptr) {
                tmp+=getA()->spaceRequired();
            }
            if(getB()!=nullptr) {
                tmp+=getB()->spaceRequired();
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
};

class NotEqual : public Condition {     // a != b
    public:
        NotEqual(ProgramPtr _a, ProgramPtr _b) : Condition(_a,_b)   {}

        virtual void print(std::ostream &dst) const override    {
            getA()->print(dst);
            dst<<" != ";
            getB()->print(dst);
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
};

class GreaterEqual : public Condition { // a >= b
    public:
        GreaterEqual(ProgramPtr _a, ProgramPtr _b) : Condition(_a,_b)    {}

        virtual void print(std::ostream &dst) const override    {
            getA()->print(dst);
            dst<<" >= ";
            getB()->print(dst);
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
};

class LessEqual : public Condition {    // a <= b
    public:
        LessEqual(ProgramPtr _a, ProgramPtr _b) : Condition(_a,_b)   {}

        virtual void print(std::ostream &dst) const override    {
            getA()->print(dst);
            dst<<" <= ";
            getB()->print(dst);
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
};

class LogicalOR : public Condition {
    public:
        LogicalOR(ProgramPtr _a, ProgramPtr _b) : Condition(_a,_b)  {}

        virtual void print(std::ostream &dst) const override    {
            getA()->print(dst);
            dst<<" || ";
            getB()->print(dst);
        }
};

class LogicalNOT : public Condition {
    public:
        LogicalNOT(ProgramPtr _a) : Condition(_a,nullptr) {}

        virtual void print(std::ostream &dst) const override    {
            dst << "!";
            getA()->print(dst);
        }
};

#endif