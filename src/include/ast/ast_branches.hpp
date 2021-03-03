#ifndef COMPILER_AST_BRANCHES_HPP
#define COMPILER_AST_BRANCHES_HPP

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

        virtual long spaceRequired() const override {
            if(getAction()!=nullptr) {
                return getAction()->spaceRequired();
            }
            else    {
                return 0;
            }
        }
};

class IfBlock : public Branch {
    private:
        ProgramPtr cond;
        ProgramPtr elseIfPtr=nullptr;
        ProgramPtr elsePtr=nullptr;
    public:
        IfBlock(ProgramPtr _condition, ProgramPtr _action, ProgramPtr _elseIfPtr, ProgramPtr _elsePtr) : Branch(_action), cond(_condition), elseIfPtr(_elseIfPtr), elsePtr(_elsePtr)  {}

        ~IfBlock()  {
            delete cond;
            delete elseIfPtr;
            delete elsePtr;
        }

        ProgramPtr getCondition() const {
            return cond;
        }

        ProgramPtr getElseIf() const    {
            return elseIfPtr;
        }

        ProgramPtr getElse() const  {
            return elsePtr;
        }

        virtual long spaceRequired() const override {
            long tmp = spaceRequired();
            tmp+=getCondition()->spaceRequired();
            if(getElseIf()!=nullptr)    {
                tmp+=getElseIf()->spaceRequired();
            }
            if(getElse()!=nullptr)    {
                tmp+=getElse()->spaceRequired();
            }
            return tmp;
        }

        virtual void print(std::ostream &dst) const override    {
            dst<<"if( ";
            getCondition()->print(dst);
            dst<<" )  ";
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

        virtual void generate(std::ofstream &file, const char* destReg, Context *context) const override    {
            std::string initialEndPoint = context->BranchEndPoint;  // save previous BranchEndPoint (to support nested Ifs)
            context->BranchEndPoint = makeLabel("IF_end");
            std::string nextBranch = makeLabel("next");
            getCondition()->generate(file, "$t6", context);     // process condition
            file<<"beq $t6, $zero, "<<nextBranch<<std::endl;      // skip If action if condition evalutes to 0
            file<<"nop"<<std::endl;
            getAction()->generate(file, destReg, context);      // process action
            file<<"b "<<context->BranchEndPoint<<std::endl;     // goto end of IfBlock
            file<<"nop"<<std::endl;
            file<<nextBranch<<":"<<std::endl;
            if(getElseIf()!=nullptr)    {
                getElseIf()->generate(file, destReg, context);
            }
            if(getElse()!=nullptr)  {
                getElse()->generate(file, destReg, context);        // process Else action
            }                   
            file<<context->BranchEndPoint<<":"<<std::endl;
            context->BranchEndPoint = initialEndPoint;          // restore previous BranchEndPoint
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

        virtual void generate(std::ofstream &file, const char* destReg, Context *context) const override    {
            if(getAction()!=nullptr)    {
                getAction()->generate(file, destReg, context);
            }
        }
};

class ElseIfBlock : public Branch {
    private:
        ProgramPtr cond;
        ProgramPtr next=nullptr;
    public:
        ElseIfBlock(ProgramPtr _cond, ProgramPtr _action, ProgramPtr _next) : Branch(_action), cond(_cond), next(_next) {}

        ~ElseIfBlock()  {
            delete cond;
            delete next;
        }

        ProgramPtr getCondition() const {
            return cond;
        }

        ProgramPtr getNext() const  {
            return next;
        }

        virtual long spaceRequired() const override {
            long tmp = spaceRequired();
            tmp+=getCondition()->spaceRequired();
            if(getNext()!=nullptr)  {
                tmp+=getNext()->spaceRequired();
            }
            return tmp;
        }

        virtual void print(std::ostream &dst) const override    {
            dst<<"else if( ";
            getCondition()->print(dst);
            dst<<" )  ";
            if(getAction()!=nullptr)    {
                getAction()->print(dst);
            }
            else    {
                dst<<"{}"<<std::endl;
            }
            if(getNext()!=nullptr)  {
                getNext()->print(dst);
            }
        }

        virtual void generate(std::ofstream &file, const char* destReg, Context *context) const override    {
            if(getAction()!=nullptr)    {
                std::string nextLabel = makeLabel("next");
                getCondition()->generate(file, "$t6", context);
                file<<"beq $t6, $zero, "<<nextLabel<<std::endl;
                file<<"nop"<<std::endl;
                getAction()->generate(file, destReg, context);
                file<<"b "<<context->BranchEndPoint<<std::endl;
                file<<"nop"<<std::endl;
                file<<nextLabel<<":"<<std::endl;
            }
            if(getNext()!=nullptr)  {
                getNext()->generate(file, destReg, context);
            }
        }
};

#endif