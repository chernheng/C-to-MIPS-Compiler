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

        virtual long spaceRequired(Context *context) const override {
            if(getAction()!=nullptr) {
                return getAction()->spaceRequired(context);
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

        virtual long spaceRequired(Context *context) const override {
            long tmp = getAction()->spaceRequired(context);
            tmp+=getCondition()->spaceRequired(context);
            if(getElseIf()!=nullptr)    {
                tmp+=getElseIf()->spaceRequired(context);
            }
            if(getElse()!=nullptr)    {
                tmp+=getElse()->spaceRequired(context);
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

        virtual long spaceRequired(Context *context) const override {
            long tmp = getAction()->spaceRequired(context);
            tmp+=getCondition()->spaceRequired(context);
            if(getNext()!=nullptr)  {
                tmp+=getNext()->spaceRequired(context);
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

class CaseBlock : public Branch {
    private:
        ProgramPtr constant;
        CaseBlock* nextCase=nullptr;
        ProgramPtr defaultAction=nullptr;
    public:
        CaseBlock(ProgramPtr _constant, ProgramPtr _action, CaseBlock* _nextCase, ProgramPtr _defaultAction) : Branch(_action), constant(_constant), nextCase(_nextCase), defaultAction(_defaultAction) {}

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

        virtual long spaceRequired(Context *context) const override {
            long tmp = getAction()->spaceRequired(context);
            tmp+=getConstant()->spaceRequired(context);
            if(getNextCase()!=nullptr)  {
                tmp+=getNextCase()->spaceRequired(context);
            }
            if(getDefaultAction()!=nullptr)  {
                tmp+=getDefaultAction()->spaceRequired(context);
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
            if(getDefaultAction() == nullptr && getNextCase()==nullptr){
                file<<"b "<<context->BranchEndPoint<<std::endl;
                file<<"nop"<<std::endl;
            }
        }

        virtual void generate(std::ofstream &file, const char* destReg, Context *context) const override    {
            std::list<std::string>::iterator it = context->Case_label.begin();
            std::string case_label = *it;
            file<<case_label<<":"<<std::endl;
            if(getAction()!=nullptr)    {
                getAction()->generate(file,destReg,context);
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

class SwitchBlock : public Branch {
    private:
        ProgramPtr expr;
        CaseBlock* casePtr=nullptr;
    public:
        SwitchBlock(ProgramPtr _expr, CaseBlock* _casePtr) : Branch(nullptr), expr(_expr),casePtr(_casePtr) {}

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

        virtual long spaceRequired(Context *context) const override {
            long tmp = getExpr()->spaceRequired(context);
            if(getCasePtr()!=nullptr)    {
                tmp+=getCasePtr()->spaceRequired(context);
            }
            return tmp;
        }

        virtual void print(std::ostream &dst) const override    {
            dst<<"switch(";
            getExpr()->print(dst);
            dst<<")"<<std::endl;
            if (getCasePtr() != nullptr){
                getCasePtr()->print(dst);
            }

            
        }

        virtual void generate(std::ofstream &file, const char* destReg, Context *context) const override    {
            std::string initialEndPoint = context->BranchEndPoint;  // save previous BranchEndPoint (to support nested Ifs)
            context->BranchEndPoint = makeLabel("SWITCH_end");
            int Switchinit = context->isSwitch;
            int Loopinit = context->isLoop;
            context->isLoop = 0;
            context->isSwitch = 1;
            std::unordered_map<std::string,varInfo> tmp;
            long initStackSize = context->stack.size;
            long initSliderVal=context->stack.slider;
            context->stack.slider=context->stack.size;
            long delta=spaceRequired(context);
            context->stack.size+=delta;
            if(delta>0) {
                file<<"addiu $sp, $sp, -"<<delta<<std::endl;
            }
            context->stack.lut.push_back(tmp);
            std::list<std::string> initCase_Label = context->Case_label;
            std::list<std::string> case_label;
            context->Case_label = case_label;
            // std::string nextBranch = makeLabel("next");
            getExpr()->generate(file,"$t7",context); // save expression that we need to compare
            if(getCasePtr() != nullptr) {
                getCasePtr()->comparison(file,"$t7",context);
                getCasePtr()->generate(file,destReg,context);
            }

            file<<context->BranchEndPoint<<":"<<std::endl;
            context->BranchEndPoint = initialEndPoint;          // restore previous BranchEndPoint
            context->Case_label = initCase_Label;
            context->isSwitch = Switchinit;
            context->isLoop = Loopinit;
            if(delta>0)    {
                file<<"addiu $sp, $sp, "<<delta<<std::endl;    // shift down the stack pointer (always move sp by 4 to maintain word alignment)
            }
            context->stack.size=initStackSize;
            context->stack.slider=initSliderVal;
            context->stack.lut.pop_back();
        }
};

class TernaryBlock : public Branch {
    private:
        ProgramPtr cond;
        ProgramPtr falseExpr;
    public:
        TernaryBlock(ProgramPtr _condition, ProgramPtr _action, ProgramPtr _falseExpr) : Branch(_action), cond(_condition), falseExpr(_falseExpr)  {}

        ~TernaryBlock()  {
            delete cond;
            delete falseExpr;
        }

        ProgramPtr getCondition() const {
            return cond;
        }

        ProgramPtr getFalse() const    {
            return falseExpr;
        }

        virtual long spaceRequired(Context *context) const override {
            long tmp = getAction()->spaceRequired(context);
            tmp+=getCondition()->spaceRequired(context);
            tmp+=getFalse()->spaceRequired(context);
            return tmp;
        }

        virtual void print(std::ostream &dst) const override    {
            getCondition()->print(dst);
            dst<<" ? ";
            getAction()->print(dst);
            dst<<" : ";
            getFalse()->print(dst);
            
        }

        virtual void generate(std::ofstream &file, const char* destReg, Context *context) const override    {
            std::string falseLabel = makeLabel("False_Expr");
            std::string ternaryLabel = makeLabel("Ternary_End");
            getCondition()->generate(file,"$t7",context);
            file<<"beq $t7, $zero, "<<falseLabel<<std::endl;
            file<<"nop"<<std::endl;
            getAction()->generate(file,destReg,context);
            file<<"b "<<ternaryLabel<<std::endl;
            file<<"nop"<<std::endl;
            file<<falseLabel<<":"<<std::endl;
            getFalse()->generate(file,destReg,context);
            file<<ternaryLabel<<":"<<std::endl;
        }
};


#endif