#ifndef COMPILER_AST_PROGRAM_HPP
#define COMPILER_AST_PROGRAM_HPP

#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <memory>

class Program;
typedef const Program *ProgramPtr;

class Program {
    public:
        virtual ~Program()  {};

        virtual void print(std::ostream &dst) const =0;
        
        // Implement generate function to generate IR code
        virtual void generate(std::ofstream &dst, std::string destReg, const std::map<std::string,long> &bindings) const   {     // consider changing bindings to a struct containing the var and fn LUTs
            throw std::runtime_error("Not yet implemented"); 
        }
};

class Command : public Program {
    private:
        ProgramPtr action;
        ProgramPtr next=nullptr;
    public:
        Command(ProgramPtr _action, ProgramPtr _next) : action(_action), next(_next)    {}

        ~Command()  {
            delete action;
            delete next;
        }

        virtual void print(std::ostream &dst) const override    {
            action->print(dst);
            dst<<std::endl;
            if(next!=nullptr)   {
                next->print(dst);
            }
        }
};

class Scope : Program {
    private:
        ProgramPtr action=nullptr;
    public:
        Scope(ProgramPtr _action) : action(_action) {}

        ~Scope()    {
            delete action;
        }

        virtual void print(std::ostream &dst) const override    {
            dst<<"{"<<std::endl;
            if(action!=nullptr) {
                action->print(dst);
            }
            dst<<std::endl<<"}"<<std::endl;
        }
};

#endif