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
            int Switchinit = context->isSwitch;
            context->isSwitch = 0;
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
            context->isSwitch = Switchinit;
        }
};

class ForLoop : public Loop  {      // for(dec; cond; asn) {...}
    private:
        ProgramPtr dec;
        ProgramPtr asn;        
    public:
        ForLoop(ProgramPtr _dec, ProgramPtr _condition, ProgramPtr _asn, ProgramPtr _action) : Loop(_condition, _action), dec(_dec), asn(_asn)  {}

        virtual ~ForLoop()  {
            delete dec;
            delete asn;
        }

        virtual void print(std::ostream &dst) const override    {
            dst<<"for(";
            dec->print(dst);
            dst<<";";
            getCondition()->print(dst);
            dst<<";";
            asn->print(dst);
            dst<<")"<<std::endl;
            getAction()->print(dst);
        }

        virtual void generate(std::ofstream &file, const char* destReg, Context *context) const override    {
            std::unordered_map<std::string,varInfo> tmp;
            std::string initLoopStart = context->LoopStartPoint;
            std::string initLoopEnd = context->LoopEndPoint;
            std::string entryPoint = makeLabel("for_entry_point");
            context->LoopStartPoint = makeLabel("Loop_Start");
            context->LoopEndPoint = makeLabel("Loop_End");
            int initialIsLoop = context->isLoop;            // info for break; to handle stack deallocation
            long initialLoopSP = context->LoopInitSP;
            long initialStackSize = context->stack.size;
            long initialSliderValue = context->stack.slider;
            context->isLoop=1;
            int Switchinit = context->isSwitch;
            context->isSwitch = 0;
            long scopeSize = dec->spaceRequired();          // allocate new scope on stack for loop conditional variable
            context->stack.lut.push_back(tmp);
            context->stack.slider = context->stack.size;
            context->stack.size += scopeSize;
            context->LoopInitSP = context->stack.size;
            if(scopeSize>0) {
                file<<"addiu $sp, $sp, -"<<scopeSize<<std::endl;
            }
            dec->generate(file, "$t4", context);            // declare for loop variable (dec)
            file<<"b "<<entryPoint<<std::endl;              // jump to entry point
            file<<"nop"<<std::endl;
            file<<context->LoopStartPoint<<":"<<std::endl;  // for loop start point
            asn->generate(file, "$t4", context);            // conditional variable assignment (asn)
            file<<entryPoint<<":"<<std::endl;               // for loop entry point
            getCondition()->generate(file, "$t6", context);     // loop condition
            file<<"beq $t6, $zero, "<<context->LoopEndPoint<<std::endl;
            file<<"nop"<<std::endl;
            getAction()->generate(file, destReg, context);
            file<<"b "<<context->LoopStartPoint<<std::endl;     // jump to for loop start point
            file<<"nop"<<std::endl;
            file<<context->LoopEndPoint<<":"<<std::endl;        // for loop end point
            if(scopeSize>0) {                                   // deallocate loop's scope from stack 
                file<<"addiu $sp, $sp, "<<scopeSize<<std::endl;
            }
            context->stack.lut.pop_back();
            context->LoopStartPoint = initLoopStart;        // restore context variables to their original values 
            context->LoopEndPoint = initLoopEnd;
            context->stack.size = initialStackSize;
            context->stack.slider = initialSliderValue;
            context->LoopInitSP = initialLoopSP;
            context->isLoop = initialIsLoop;
            context->isSwitch = Switchinit;
        }
};

#endif