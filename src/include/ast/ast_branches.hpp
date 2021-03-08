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
            long tmp = getAction()->spaceRequired();
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
            if(getElseIf()!=nullptr)  {
                getElseIf()->print(dst);
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
            long tmp = getAction()->spaceRequired();
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

class SwitchBlock : public Branch {
    private:
        ProgramPtr expr;
        ProgramPtr casePtr=nullptr;
    public:
        SwitchBlock(ProgramPtr _expr, ProgramPtr _casePtr) : Branch(nullptr), expr(_expr),casePtr(_casePtr) {}

        ~SwitchBlock()  {
            delete expr;
            delete casePtr;
        }

        ProgramPtr getExpr() const {
            return expr;
        }

        ProgramPtr getCasePtr() const    {
            return casePtr;
        }

        virtual long spaceRequired() const override {
            long tmp = getExpr()->spaceRequired();
            if(getCasePtr()!=nullptr)    {
                tmp+=getCasePtr()->spaceRequired();
            }
            return tmp;
        }

        virtual void print(std::ostream &dst) const override    {
            dst<<"switch(";
            getExpr()->print(dst);
            dst<<")"<<std::endl;
            getCasePtr()->print(dst);

            
        }

        virtual void generate(std::ofstream &file, const char* destReg, Context *context) const override    {
            std::string initialEndPoint = context->BranchEndPoint;  // save previous BranchEndPoint (to support nested Ifs)
            context->BranchEndPoint = makeLabel("SWITCH_end");
            int Loopinit = context->isLoop;
            context->isLoop = 0;
            context->isSwitch = 1;
            std::list<std::string> initCase_Label = context->Case_label;
            std::list<std::string> case_label;
            context->Case_label = case_label;
            // std::string nextBranch = makeLabel("next");
            getExpr()->generate(file,"$t7",context); // save expression that we need to compare
            if(getCasePtr() != nullptr) {
                getCasePtr()->comparison(file,"t7",context);
                getCasePtr()->generate(file,destReg,context);
            }

            file<<context->BranchEndPoint<<":"<<std::endl;
            context->BranchEndPoint = initialEndPoint;          // restore previous BranchEndPoint
            context->Case_label = initCase_Label;
            context->isSwitch = 0;
            context->isLoop = Loopinit;
        }
};

class CaseBlock : public Branch {
    private:
        ProgramPtr constant;
        ProgramPtr nextCase=nullptr;
        ProgramPtr defaultAction=nullptr;
    public:
        CaseBlock(ProgramPtr _constant, ProgramPtr _action, ProgramPtr _nextCase, ProgramPtr _defaultAction) : Branch(_action), constant(_constant), nextCase(_nextCase), defaultAction(_defaultAction) {}

        ~CaseBlock()  {
            delete constant;
            delete nextCase;
            delete defaultAction;
        }

        ProgramPtr getConstant() const {
            return constant;
        }

        ProgramPtr getNextCase() const  {
            return nextCase;
        }

        ProgramPtr getDefaultAction() const  {
            return defaultAction;
        }

        virtual long spaceRequired() const override {
            long tmp = getAction()->spaceRequired();
            tmp+=getConstant()->spaceRequired();
            if(getNextCase()!=nullptr)  {
                tmp+=getNextCase()->spaceRequired();
            }
            if(getDefaultAction()!=nullptr)  {
                tmp+=getDefaultAction()->spaceRequired();
            }
            
            return tmp;
        }

        virtual void print(std::ostream &dst) const override    {
            dst<<"case ";
            getConstant()->print(dst);
            dst<<":"<<std::endl;
            if(getAction()!=nullptr)    {
                getAction()->print(dst);
            }
            if(getNextCase()!=nullptr)  {
                getNextCase()->print(dst);
            }
            if(getDefaultAction() != nullptr){
                dst<<"default:"<<std::endl;
                getDefaultAction()->print(dst);
            }
        }

        virtual void comparison(std::ofstream &file, const char* srcReg, Context *context) const override    {
            std::string nextLabel = makeLabel("nextcase");
            context->Case_label.push_back(nextLabel);
            long ofs = context->stack.slider;
            file<<"sw "<<std::string(srcReg)<<", "<<(context->stack.size - ofs)<<"($sp)"<<std::endl;
            context->stack.slider+=4;
            getConstant()->generate(file, "$t0",context);
            file<<"lw $t1, "<<(context->stack.size - ofs)<<"($sp)"<<std::endl;
            context->stack.slider-=4;
            file <<"beq $t1, $t0, "<<nextLabel<<std::endl;
            file<<"nop"<<std::endl;
            if (getNextCase() != nullptr){
                getNextCase()->comparison(file,"$t1",context);
            }
            if(getDefaultAction() != nullptr){
                std::string defaultLabel = makeLabel("default");
                file<<"b "<<defaultLabel<<std::endl;
                file<<"nop"<<std::endl;
                context->Case_label.push_back(defaultLabel);
            }
        }

        virtual void generate(std::ofstream &file, const char* destReg, Context *context) const override    {
            std::list<std::string>::iterator it = context->Case_label.begin();
            std::string case_label = *it;
            file<<case_label<<":"<<std::endl;
            if(getAction()!=nullptr)    {
                getAction()->generate(file,destReg,context);
                file<<"b "<<context->BranchEndPoint<<std::endl;
                file<<"nop"<<std::endl;
            }
            if (context->Case_label.size() != 0){
                context->Case_label.pop_front();
            }
            if(getNextCase() != nullptr){
                getNextCase()->generate(file,destReg,context);
            }
            if(getDefaultAction() != nullptr){
                std::list<std::string>::iterator it_def = context->Case_label.begin();
                std::string default_label = *it_def;
                file<<default_label<<":"<<std::endl;
                getDefaultAction()->generate(file,destReg,context);
            }
        }
};

#endif