#ifndef COMPILER_AST_LOOP_HPP
#define COMPILER_AST_LOOP_HPP

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

        virtual long spaceRequired() const override {
            long tmp = getCondition()->spaceRequired();
            if(getAction()!=nullptr)    {
                tmp+=getAction()->spaceRequired();
            }
            return tmp;
        }

        virtual void print(std::ostream &dst) const override    {
            dst<<"while(";
            getCondition()->print(dst);
            dst<<")"<<std::endl;
            getAction()->print(dst);
        }

        virtual void generate(std::ofstream &file, const char* destReg, Context *context) const override    {
            std::string initLoopStart = context->LoopStartPoint;
            std::string initLoopEnd = context->LoopEndPoint;
            int initialIsLoop = context->isLoop;            // info for break; to handle stack deallocation
            long initialLoopSP = context->LoopInitSP;
            context->LoopInitSP = context->stack.size;
            context->isLoop=1;
            context->LoopStartPoint = makeLabel("Loop_Start");
            context->LoopEndPoint = makeLabel("Loop_End");
            file<<context->LoopStartPoint<<":"<<std::endl;  // loop start
            getCondition()->generate(file, "$t6", context);
            file<<"beq $t6, $zero, "<<context->LoopEndPoint<<std::endl; // jump to loop end if condition == 0
            file<<"nop"<<std::endl;
            getAction()->generate(file, destReg, context);  // loop action
            file<<"b "<<context->LoopStartPoint<<std::endl; // jump to start of loop
            file<<"nop"<<std::endl;
            file<<context->LoopEndPoint<<":"<<std::endl;    // loop end
            context->LoopStartPoint = initLoopStart;        // restore context variables to their original values 
            context->LoopEndPoint = initLoopEnd;
            context->LoopInitSP = initialLoopSP;
            context->isLoop = initialIsLoop;
        }
};

class ForLoop : public Loop  {
    public:
        ForLoop(ProgramPtr _condition, ProgramPtr _action) : Loop(_condition, _action)  {}

        virtual void print(std::ostream &dst) const override    {
            dst<<"for(";
            getCondition()->print(dst);
            dst<<")"<<std::endl;
            getAction()->print(dst);
        }
};

#endif