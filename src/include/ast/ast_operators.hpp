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

        virtual long spaceRequired(Context *context) const override {
            long tmp=4;
            if(left!=nullptr)   {
                tmp+=left->spaceRequired(context);
            }
            if(right!=nullptr)  {
                tmp+=right->spaceRequired(context);
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

        virtual long spaceRequired(Context *context) const override  {   // pass through space requirement of right operator
            return getRight()->spaceRequired(context);
        }

        virtual void generate(std::ofstream &file, const char* destReg, Context *context) const override    {
            // long offset=getLeft()->getOffset(context);
            // std::string t=getLeft()->getVarType(context);
            int ptr_left = getLeft()->getPointer(context);
            std::string type = getLeft()->getVarType(context);
            if (ptr_left == 0 && (type=="float" || type=="double")){
                getRight()->generate(file, "$f4", context);
                getLeft()->generate(file, "$f4",context);
            } else {
                getRight()->generate(file, "$t0", context);
                getLeft()->generate(file, "$t0", context);
            }
            // if(t=="int")    {
            //     file<<"sw $t0, "<<offset<<"($sp)"<<std::endl;
            // }
            // else if(t=="char")  {
            //     file<<"sb $t0, "<<offset<<"($sp)"<<std::endl;
            // }
            if(std::string(destReg)!="$f0"){
                file<<"li "<<std::string(destReg)<<", 1"<<std::endl;
            }
        }
};

class AssignmentSumOperator : public Operator {
    protected:
        virtual const char *getOpcode() const override  {
            return "+=";
        }
    public:
        AssignmentSumOperator(ProgramPtr _left, ProgramPtr _right) : Operator(_left,_right)    {}

        virtual void print(std::ostream &dst) const override    {
            getLeft()->print(dst);
            dst<<"+=";
            getRight()->print(dst);
        }

        virtual void generate(std::ofstream &file, const char* destReg, Context *context) const override    {
            
            getRight()->generate(file,"$t2",context);
            getLeft()->generate(file, "$t0", context);
            file<<"addu $t2, $t0, $t2"<<std::endl;
            
            if(context->tempVarInfo.derefPtr==1){
                file<<"lw $t0, "<<getLeft()->getOffset(context)<<"($sp)"<<std::endl;
                if(getLeft()->getVarType(context)=="int")  {
                    file<<"sw $t2, 0($t0)"<<std::endl;
                }
                else if(getLeft()->getVarType(context)=="char")    {
                    file<<"sb $t2, 0($t0)"<<std::endl;
                }
            } else { 
                if(getLeft()->getVarType(context)=="int")  {
                    file<<"sw $t2, "<<getLeft()->getOffset(context)<<"($sp)"<<std::endl;
                }
                else if(getLeft()->getVarType(context)=="char")    {
                    file<<"sb $t2, "<<getLeft()->getOffset(context)<<"($sp)"<<std::endl;
                }
            }
            file<<"move "<<std::string(destReg)<<", $t2"<<std::endl;
        }
};

class AssignmentDiffOperator : public Operator {
    protected:
        virtual const char *getOpcode() const override  {
            return "-=";
        }
    public:
        AssignmentDiffOperator(ProgramPtr _left, ProgramPtr _right) : Operator(_left,_right)    {}

        virtual void print(std::ostream &dst) const override    {
            getLeft()->print(dst);
            dst<<"-=";
            getRight()->print(dst);
        }

        virtual void generate(std::ofstream &file, const char* destReg, Context *context) const override    {
            getRight()->generate(file,"$t2",context);
            getLeft()->generate(file, "$t0", context);
            file<<"subu $t2, $t0, $t2"<<std::endl;
            
            if(context->tempVarInfo.derefPtr==1){
                file<<"lw $t0, "<<getLeft()->getOffset(context)<<"($sp)"<<std::endl;
                if(getLeft()->getVarType(context)=="int")  {
                    file<<"sw $t2, 0($t0)"<<std::endl;
                }
                else if(getLeft()->getVarType(context)=="char")    {
                    file<<"sb $t2, 0($t0)"<<std::endl;
                }
            } else { 
                if(getLeft()->getVarType(context)=="int")  {
                    file<<"sw $t2, "<<getLeft()->getOffset(context)<<"($sp)"<<std::endl;
                }
                else if(getLeft()->getVarType(context)=="char")    {
                    file<<"sb $t2, "<<getLeft()->getOffset(context)<<"($sp)"<<std::endl;
                }
            }
            file<<"move "<<std::string(destReg)<<", $t2"<<std::endl;
        }
};

class AssignmentProductOperator : public Operator {
    protected:
        virtual const char *getOpcode() const override  {
            return "*=";
        }
    public:
        AssignmentProductOperator(ProgramPtr _left, ProgramPtr _right) : Operator(_left,_right)    {}

        virtual void print(std::ostream &dst) const override    {
            getLeft()->print(dst);
            dst<<"*=";
            getRight()->print(dst);
        }

        virtual void generate(std::ofstream &file, const char* destReg, Context *context) const override    {
            getRight()->generate(file,"$t2",context);
            getLeft()->generate(file, "$t0", context);
            file<<"mult $t2, $t0"<<std::endl;
            file<<"mflo $t2"<<std::endl;
            
            if(context->tempVarInfo.derefPtr==1){
                file<<"lw $t0, "<<getLeft()->getOffset(context)<<"($sp)"<<std::endl;
                if(getLeft()->getVarType(context)=="int")  {
                    file<<"sw $t2, 0($t0)"<<std::endl;
                }
                else if(getLeft()->getVarType(context)=="char")    {
                    file<<"sb $t2, 0($t0)"<<std::endl;
                }
            } else { 
                if(getLeft()->getVarType(context)=="int")  {
                    file<<"sw $t2, "<<getLeft()->getOffset(context)<<"($sp)"<<std::endl;
                }
                else if(getLeft()->getVarType(context)=="char")    {
                    file<<"sb $t2, "<<getLeft()->getOffset(context)<<"($sp)"<<std::endl;
                }
            }
            file<<"move "<<std::string(destReg)<<", $t2"<<std::endl;
        }
};

class AssignmentDivideOperator : public Operator {
    protected:
        virtual const char *getOpcode() const override  {
            return "/=";
        }
    public:
        AssignmentDivideOperator(ProgramPtr _left, ProgramPtr _right) : Operator(_left,_right)    {}

        virtual void print(std::ostream &dst) const override    {
            getLeft()->print(dst);
            dst<<"/=";
            getRight()->print(dst);
        }

        virtual void generate(std::ofstream &file, const char* destReg, Context *context) const override    {
            getRight()->generate(file,"$t2",context);
            getLeft()->generate(file, "$t0", context);
            file<<"div $t0, $t2"<<std::endl;
            file<<"mflo $t2"<<std::endl;
            
            if(context->tempVarInfo.derefPtr==1){
                file<<"lw $t0, "<<getLeft()->getOffset(context)<<"($sp)"<<std::endl;
                if(getLeft()->getVarType(context)=="int")  {
                    file<<"sw $t2, 0($t0)"<<std::endl;
                }
                else if(getLeft()->getVarType(context)=="char")    {
                    file<<"sb $t2, 0($t0)"<<std::endl;
                }
            } else { 
                if(getLeft()->getVarType(context)=="int")  {
                    file<<"sw $t2, "<<getLeft()->getOffset(context)<<"($sp)"<<std::endl;
                }
                else if(getLeft()->getVarType(context)=="char")    {
                    file<<"sb $t2, "<<getLeft()->getOffset(context)<<"($sp)"<<std::endl;
                }
            }
            file<<"move "<<std::string(destReg)<<", $t2"<<std::endl;
        }
};

class AssignmentModOperator : public Operator {
    protected:
        virtual const char *getOpcode() const override  {
            return "%=";
        }
    public:
        AssignmentModOperator(ProgramPtr _left, ProgramPtr _right) : Operator(_left,_right)    {}

        virtual void print(std::ostream &dst) const override    {
            getLeft()->print(dst);
            dst<<"%=";
            getRight()->print(dst);
        }

        virtual void generate(std::ofstream &file, const char* destReg, Context *context) const override    {
            getRight()->generate(file,"$t2",context);
            getLeft()->generate(file, "$t0", context);
            file<<"div $t0, $t2"<<std::endl;
            file<<"mfhi $t2"<<std::endl;
            
            if(context->tempVarInfo.derefPtr==1){
                file<<"lw $t0, "<<getLeft()->getOffset(context)<<"($sp)"<<std::endl;
                if(getLeft()->getVarType(context)=="int")  {
                    file<<"sw $t2, 0($t0)"<<std::endl;
                }
                else if(getLeft()->getVarType(context)=="char")    {
                    file<<"sb $t2, 0($t0)"<<std::endl;
                }
            } else { 
                if(getLeft()->getVarType(context)=="int")  {
                    file<<"sw $t2, "<<getLeft()->getOffset(context)<<"($sp)"<<std::endl;
                }
                else if(getLeft()->getVarType(context)=="char")    {
                    file<<"sb $t2, "<<getLeft()->getOffset(context)<<"($sp)"<<std::endl;
                }
            }
            file<<"move "<<std::string(destReg)<<", $t2"<<std::endl;
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
            std::string type = getLeft()->getVarType(context);
            int ptr_left = getLeft()->getPointer(context);
            int ptr_right = getRight()->getPointer(context);
            if (ptr_left == 1 || ptr_right == 1){
                getLeft()->generate(file, "$t1", context);
                long ofs = context->stack.slider;
                file<<"sw $t1, "<<(context->stack.size - ofs)<<"($sp)"<<std::endl;
                context->stack.slider+=4;
                varInfo varLeft = context->tempVarInfo;
                getRight()->generate(file, "$t2", context);
                varInfo varRight = context->tempVarInfo;
                if (varLeft.isPtr==1 && varLeft.numBytes > 1) {
                    if (varLeft.type == "double"){
                        file<<"li $t3, 8"<<std::endl;
                    } else {
                        file<<"li $t3, "<<varLeft.numBytes<<std::endl;
                    }
                    file<<"mult $t2, $t3"<<std::endl;
                    file<<"mflo $t2"<<std::endl;
                    file<<"lw $t1, "<<(context->stack.size - ofs)<<"($sp)"<<std::endl;
                    context->stack.slider-=4;
                    file<<"addu "<<std::string(destReg)<<", $t1, $t2"<<std::endl;
                    context->tempVarInfo = varLeft;

                } else if(varRight.isPtr==1 && varRight.numBytes > 1) {
                    if (varLeft.type == "double"){
                        file<<"li $t3, 8"<<std::endl;
                    } else {
                        file<<"li $t3, "<<varRight.numBytes<<std::endl;
                    }
                    file<<"lw $t1, "<<(context->stack.size - ofs)<<"($sp)"<<std::endl;
                    file<<"mult $t1, $t3"<<std::endl;
                    file<<"mflo $t1"<<std::endl;
                    context->stack.slider-=4;
                    file<<"addu "<<std::string(destReg)<<", $t1, $t2"<<std::endl;
                }
            } else if (type == "double" || type == "float"){
                getLeft()->generate(file, "$f6", context);
                long ofs = context->stack.slider;
                if (type == "float"){
                    file<<"s.s $f6, "<<(context->stack.size - ofs)<<"($sp)"<<std::endl;
                    context->stack.slider+=4;
                } else if(type == "double"){
                    file<<"s.d $f6, "<<(context->stack.size - ofs-4)<<"($sp)"<<std::endl;
                    context->stack.slider+=8;
                }
                getRight()->generate(file, "$f8", context);
                if (type == "float"){
                    file<<"l.s $f6, "<<(context->stack.size - ofs)<<"($sp)"<<std::endl;
                    context->stack.slider-=4;
                    file<<"add.s "<<std::string(destReg)<<", $f6, $f8"<<std::endl;
                } else if(type == "double"){
                    file<<"l.d $f6, "<<(context->stack.size - ofs-4)<<"($sp)"<<std::endl;
                    context->stack.slider-=8;
                    file<<"add.d "<<std::string(destReg)<<", $f6, $f8"<<std::endl;
                }
            }else {
                getLeft()->generate(file, "$t1", context);
                long ofs = context->stack.slider;
                file<<"sw $t1, "<<(context->stack.size - ofs)<<"($sp)"<<std::endl;
                context->stack.slider+=4;
                varInfo varLeft = context->tempVarInfo;
                getRight()->generate(file, "$t2", context);
                varInfo varRight = context->tempVarInfo;
                file<<"lw $t1, "<<(context->stack.size - ofs)<<"($sp)"<<std::endl;
                context->stack.slider-=4;
                file<<"addu "<<std::string(destReg)<<", $t1, $t2"<<std::endl;
            }


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
                       std::string type = getLeft()->getVarType(context);
            int ptr_left = getLeft()->getPointer(context);
            int ptr_right = getRight()->getPointer(context);
            if (ptr_left == 1 || ptr_right == 1){
                getLeft()->generate(file, "$t1", context);
                long ofs = context->stack.slider;
                file<<"sw $t1, "<<(context->stack.size - ofs)<<"($sp)"<<std::endl;
                context->stack.slider+=4;
                varInfo varLeft = context->tempVarInfo;
                getRight()->generate(file, "$t2", context);
                varInfo varRight = context->tempVarInfo;
                if (varLeft.isPtr==1 && varLeft.numBytes > 1) {
                    if (varLeft.type == "double"){
                        file<<"li $t3, 8"<<std::endl;
                    } else {
                        file<<"li $t3, "<<varLeft.numBytes<<std::endl;
                    }
                    file<<"mult $t2, $t3"<<std::endl;
                    file<<"mflo $t2"<<std::endl;
                    file<<"lw $t1, "<<(context->stack.size - ofs)<<"($sp)"<<std::endl;
                    context->stack.slider-=4;
                    file<<"subu "<<std::string(destReg)<<", $t1, $t2"<<std::endl;
                    context->tempVarInfo = varLeft;

                } else if(varRight.isPtr==1 && varRight.numBytes > 1) {
                    if (varLeft.type == "double"){
                        file<<"li $t3, 8"<<std::endl;
                    } else {
                        file<<"li $t3, "<<varRight.numBytes<<std::endl;
                    }
                    file<<"lw $t1, "<<(context->stack.size - ofs)<<"($sp)"<<std::endl;
                    file<<"mult $t1, $t3"<<std::endl;
                    file<<"mflo $t1"<<std::endl;
                    context->stack.slider-=4;
                    file<<"subu "<<std::string(destReg)<<", $t1, $t2"<<std::endl;
                }
            } else if (type == "double" || type == "float"){
                getLeft()->generate(file, "$f6", context);
                long ofs = context->stack.slider;
                if (type == "float"){
                    file<<"s.s $f6, "<<(context->stack.size - ofs)<<"($sp)"<<std::endl;
                    context->stack.slider+=4;
                } else if(type == "double"){
                    file<<"s.d $f6, "<<(context->stack.size - ofs-4)<<"($sp)"<<std::endl;
                    context->stack.slider+=8;
                }
                getRight()->generate(file, "$f8", context);
                if (type == "float"){
                    file<<"l.s $f6, "<<(context->stack.size - ofs)<<"($sp)"<<std::endl;
                    context->stack.slider-=4;
                    file<<"sub.s "<<std::string(destReg)<<", $f6, $f8"<<std::endl;
                } else if(type == "double"){
                    file<<"l.d $f6, "<<(context->stack.size - ofs-4)<<"($sp)"<<std::endl;
                    context->stack.slider-=8;
                    file<<"sub.d "<<std::string(destReg)<<", $f6, $f8"<<std::endl;
                }
            }else {
                getLeft()->generate(file, "$t1", context);
                long ofs = context->stack.slider;
                file<<"sw $t1, "<<(context->stack.size - ofs)<<"($sp)"<<std::endl;
                context->stack.slider+=4;
                varInfo varLeft = context->tempVarInfo;
                getRight()->generate(file, "$t2", context);
                varInfo varRight = context->tempVarInfo;
                file<<"lw $t1, "<<(context->stack.size - ofs)<<"($sp)"<<std::endl;
                context->stack.slider-=4;
                file<<"subu "<<std::string(destReg)<<", $t1, $t2"<<std::endl;
            }
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
            std::string type = getLeft()->getVarType(context);
            if (type == "double" || type == "float"){
                getLeft()->generate(file, "$f6", context);
                long ofs = context->stack.slider;
                if (type == "float"){
                    file<<"s.s $f6, "<<(context->stack.size - ofs)<<"($sp)"<<std::endl;
                    context->stack.slider+=4;
                } else if(type == "double"){
                    file<<"s.d $f6, "<<(context->stack.size - ofs-4)<<"($sp)"<<std::endl;
                    context->stack.slider+=8;
                }
                varInfo varLeft = context->tempVarInfo;
                getRight()->generate(file, "$f8", context);
                varInfo varRight = context->tempVarInfo;
                if (context->tempVarInfo.numBytes == 4){
                    file<<"l.s $f6, "<<(context->stack.size - ofs)<<"($sp)"<<std::endl;
                    context->stack.slider-=4;
                    file<<"mul.s "<<std::string(destReg)<<", $f6, $f8"<<std::endl;
                } else if(context->tempVarInfo.numBytes == 8){
                    file<<"l.d $f6, "<<(context->stack.size - ofs-4)<<"($sp)"<<std::endl;
                    context->stack.slider-=8;
                    file<<"mul.d "<<std::string(destReg)<<", $f6, $f8"<<std::endl;
                }
            }else {
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
            std::string type = getLeft()->getVarType(context);
            if (type == "double" || type == "float"){
                getLeft()->generate(file, "$f6", context);
                long ofs = context->stack.slider;
                if (type == "float"){
                    file<<"s.s $f6, "<<(context->stack.size - ofs)<<"($sp)"<<std::endl;
                    context->stack.slider+=4;
                } else if(type == "double"){
                    file<<"s.d $f6, "<<(context->stack.size - ofs-4)<<"($sp)"<<std::endl;
                    context->stack.slider+=8;
                }
                varInfo varLeft = context->tempVarInfo;
                getRight()->generate(file, "$f8", context);
                varInfo varRight = context->tempVarInfo;
                if (context->tempVarInfo.numBytes == 4){
                    file<<"l.s $f6, "<<(context->stack.size - ofs)<<"($sp)"<<std::endl;
                    context->stack.slider-=4;
                    file<<"div.s "<<std::string(destReg)<<", $f6, $f8"<<std::endl;
                } else if(context->tempVarInfo.numBytes == 8){
                    file<<"l.d $f6, "<<(context->stack.size - ofs-4)<<"($sp)"<<std::endl;
                    context->stack.slider-=8;
                    file<<"div.d "<<std::string(destReg)<<", $f6, $f8"<<std::endl;
                }
            }else {
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
            dst<<"REF";
        }

        virtual std::string getVarType(Context *context) const override {
            return getLeft()->getVarType(context);
        }

        virtual int getPointer(Context *context) const override  {  // returns offset from current $sp
            return 1;
        }

        virtual void generate(std::ofstream &file, const char* destReg, Context *context) const override    {
            long offset = getLeft()->getOffset(context);
            std::unordered_map<std::string,structInfo>::iterator it;
            it=context->structTable.find(getLeft()->getVarType(context));
            if(it!=context->structTable.end())    {                                 // if Left is a struct
                file<<"lw "<<std::string(destReg)<<", "<<offset<<"($sp)"<<std::endl;
                context->tempVarInfo.numBytes = it->second.size;
                return;
            }
            else    {
                if (getLeft()->getVarType(context) == "int" ){
                    context->tempVarInfo.numBytes =4;
                } else if(getLeft()->getVarType(context) == "char" ){
                    context->tempVarInfo.numBytes =1;
                }
                context->tempVarInfo.isPtr = 1;
                file<<"addiu "<<std::string(destReg)<<", $sp, "<<offset<<std::endl;
            }            
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

        virtual std::string getVarType(Context *context) const override {
            return getLeft()->getVarType(context);
        }

        virtual long getOffset(Context *context) const override  { 
            return getLeft()->getOffset(context);
        }

        virtual int getPointer(Context *context) const override  {  // returns offset from current $sp
            return 0;
        }


        virtual void generate(std::ofstream &file, const char* destReg, Context *context) const override    {
            getLeft()->generate(file, "$t1", context);
            context->tempVarInfo.isPtr = 0;
            context->tempVarInfo.derefPtr = 1;
            if (context->tempVarInfo.type=="int"){
                file<<"lw "<<std::string(destReg)<<", 0($t1)"<<std::endl;
            } else if(context->tempVarInfo.type=="char"){
                file<<"lb "<<std::string(destReg)<<", 0($t1)"<<std::endl;
            } else if(context->tempVarInfo.isFP == 1) {
                if(context->tempVarInfo.type == "float"){
                    file<<"l.s "<<std::string(destReg)<<", 0($t1)"<<std::endl;
                } else if (context->tempVarInfo.type == "double"){
                    file<<"l.d "<<std::string(destReg)<<", 0($t1)"<<std::endl;
                }
            }
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
        virtual std::string getVarType(Context *context) const override {
            return getLeft()->getVarType(context);
        }

        virtual void generate(std::ofstream &file, const char* destReg, Context *context) const override    {
            std::string type = getLeft()->getVarType(context);
            if (type == "double" || type == "float"){
                getLeft()->generate(file, "$f6", context);
                if (type == "float"){
                    file<<"neg.s "<<std::string(destReg)<<", $f6"<<std::endl;
                } else if(type == "double"){
                    file<<"neg.d "<<std::string(destReg)<<", $f6"<<std::endl;
                }
            }else {
            getLeft()->generate(file, "$t1", context);
            file<<"subu "<<std::string(destReg)<<", $zero, $t1"<<std::endl;
            }
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
            std::string type = getLeft()->getVarType(context);
            int ptr = getLeft()->getPointer(context);
            if ((type == "double" || type == "float")&& ptr == 0){
                getLeft()->generate(file, "$f6", context);
                if (type == "float"){
                    file<<"l.s $f8, ONE_Float"<<std::endl;
                    file<<"add.s $f8, $f6, $f8"<<std::endl;
                    if (context->tempVarInfo.derefPtr==1){
                        file<<"lw $t0, "<<getLeft()->getOffset(context)<<"($sp)"<<std::endl;
                        file<<"s.s $f8, 0($t0)"<<std::endl;
                    } else {
                        file<<"s.s $f8, "<<getLeft()->getOffset(context)<<"($sp)"<<std::endl;
                    }
                    file<<"mov.s "<<std::string(destReg)<<", $f6"<<std::endl;
                } else if(type == "double"){
                    file<<"l.d $f8, ONE_Double"<<std::endl;
                    file<<"add.d $f8, $f6, $f8"<<std::endl;
                    if (context->tempVarInfo.derefPtr==1){
                        file<<"lw $t0, "<<getLeft()->getOffset(context)<<"($sp)"<<std::endl;
                        file<<"s.d $f8, 0($t0)"<<std::endl;
                    } else {
                        file<<"s.d $f8, "<<getLeft()->getOffset(context)<<"($sp)"<<std::endl;
                    }
                    file<<"mov.d "<<std::string(destReg)<<", $f6"<<std::endl;
                }
            } else{
                getLeft()->generate(file, "$t0", context);
                if(context->tempVarInfo.isPtr==1){
                    if (context->tempVarInfo.type == "double"){
                        file<<"addiu $t1, $t0, 8"<<std::endl;
                    } else {
                        file<<"addiu $t1, $t0, "<<context->tempVarInfo.numBytes<<std::endl;
                    }
                }else {
                    file<<"addiu $t1, $t0, 1"<<std::endl;
                }
                if(context->tempVarInfo.derefPtr==1){
                    file<<"lw $t2, "<<getLeft()->getOffset(context)<<"($sp)"<<std::endl;
                    if(getLeft()->getVarType(context)=="int")  {
                        file<<"sw $t1, 0($t2)"<<std::endl;
                    }
                    else if(getLeft()->getVarType(context)=="char")    {
                        file<<"sb $t1, 0($t2)"<<std::endl;
                    }
                } else { 
                    if(getLeft()->getVarType(context)=="int"||getLeft()->getVarType(context)=="float"||getLeft()->getVarType(context)=="double")  {
                        file<<"sw $t1, "<<getLeft()->getOffset(context)<<"($sp)"<<std::endl;
                    }
                    else if(getLeft()->getVarType(context)=="char")    {
                        file<<"sb $t1, "<<getLeft()->getOffset(context)<<"($sp)"<<std::endl;
                    }
                }
                if (std::string(destReg)!="$f0"){
                    file<<"move "<<std::string(destReg)<<", $t0"<<std::endl;
                }
            }
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
            std::string type = getLeft()->getVarType(context);
            int ptr = getLeft()->getPointer(context);
            if ((type == "double" || type == "float")&& ptr == 0){
                getLeft()->generate(file, "$f6", context);
                if (type == "float"){
                    file<<"l.s $f8, ONE_Float"<<std::endl;
                    file<<"sub.s $f8, $f6, $f8"<<std::endl;
                    if (context->tempVarInfo.derefPtr==1){
                        file<<"lw $t0, "<<getLeft()->getOffset(context)<<"($sp)"<<std::endl;
                        file<<"s.s $f8, 0($t0)"<<std::endl;
                    } else {
                        file<<"s.s $f8, "<<getLeft()->getOffset(context)<<"($sp)"<<std::endl;
                    }
                    file<<"mov.s "<<std::string(destReg)<<", $f6"<<std::endl;
                } else if(type == "double"){
                    file<<"l.d $f8, ONE_Double"<<std::endl;
                    file<<"sub.d $f8, $f6, $f8"<<std::endl;
                    if (context->tempVarInfo.derefPtr==1){
                        file<<"lw $t0, "<<getLeft()->getOffset(context)<<"($sp)"<<std::endl;
                        file<<"s.d $f8, 0($t0)"<<std::endl;
                    } else {
                        file<<"s.d $f8, "<<getLeft()->getOffset(context)<<"($sp)"<<std::endl;
                    }
                    file<<"mov.d "<<std::string(destReg)<<", $f6"<<std::endl;
                }
            } else{
                getLeft()->generate(file, "$t0", context);
                if(context->tempVarInfo.isPtr==1){
                    if (context->tempVarInfo.type == "double"){
                        file<<"addiu $t1, $t0, -8"<<std::endl;
                    } else {
                        file<<"addiu $t1, $t0, -"<<context->tempVarInfo.numBytes<<std::endl;
                    }
                }else {
                    file<<"addiu $t1, $t0, -1"<<std::endl;
                }
                if(context->tempVarInfo.derefPtr==1){
                    file<<"lw $t2, "<<getLeft()->getOffset(context)<<"($sp)"<<std::endl;
                    if(getLeft()->getVarType(context)=="int")  {
                        file<<"sw $t1, 0($t2)"<<std::endl;
                    }
                    else if(getLeft()->getVarType(context)=="char")    {
                        file<<"sb $t1, 0($t2)"<<std::endl;
                    }
                } else { 
                    if(getLeft()->getVarType(context)=="int"||getLeft()->getVarType(context)=="float"||getLeft()->getVarType(context)=="double")  {
                        file<<"sw $t1, "<<getLeft()->getOffset(context)<<"($sp)"<<std::endl;
                    }
                    else if(getLeft()->getVarType(context)=="char")    {
                        file<<"sb $t1, "<<getLeft()->getOffset(context)<<"($sp)"<<std::endl;
                    }
                }
                if (std::string(destReg)!="$f0"){
                    file<<"move "<<std::string(destReg)<<", $t0"<<std::endl;
                }
            }

        //     getLeft()->generate(file, "$t0", context);
        //     if(context->tempVarInfo.isPtr==1){
        //         file<<"addiu $t1, $t0, -"<<context->tempVarInfo.numBytes<<std::endl;
        //     }else {
        //         file<<"addiu $t1, $t0, -1"<<std::endl;
        //     }
        //     if(context->tempVarInfo.derefPtr==1){
        //         file<<"lw $t0, "<<getLeft()->getOffset(context)<<"($sp)"<<std::endl;
        //         if(getLeft()->getVarType(context)=="int")  {
        //             file<<"sw $t1, 0($t0)"<<std::endl;
        //         }
        //         else if(getLeft()->getVarType(context)=="char")    {
        //             file<<"sb $t1, 0($t0)"<<std::endl;
        //         }
        //     } else { 
        //         if(getLeft()->getVarType(context)=="int")  {
        //             file<<"sw $t1, "<<getLeft()->getOffset(context)<<"($sp)"<<std::endl;
        //         }
        //         else if(getLeft()->getVarType(context)=="char")    {
        //             file<<"sb $t1, "<<getLeft()->getOffset(context)<<"($sp)"<<std::endl;
        //         }
        //     }
        //     getLeft()->generate(file, "$t0", context);
        //     file<<"move "<<std::string(destReg)<<", $t0"<<std::endl;
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
            std::string type = getLeft()->getVarType(context);
            int ptr = getLeft()->getPointer(context);
            if ((type == "double" || type == "float")&& ptr == 0){
                getLeft()->generate(file, "$f6", context);
                if (type == "float"){
                    file<<"l.s $f8, ONE_Float"<<std::endl;
                    file<<"add.s $f8, $f6, $f8"<<std::endl;
                    if (context->tempVarInfo.derefPtr==1){
                        file<<"lw $t0, "<<getLeft()->getOffset(context)<<"($sp)"<<std::endl;
                        file<<"s.s $f8, 0($t0)"<<std::endl;
                    } else {
                        file<<"s.s $f8, "<<getLeft()->getOffset(context)<<"($sp)"<<std::endl;
                    }
                    file<<"mov.s "<<std::string(destReg)<<", $f6"<<std::endl;
                } else if(type == "double"){
                    file<<"l.d $f8, ONE_Double"<<std::endl;
                    file<<"add.d $f8, $f6, $f8"<<std::endl;
                    if (context->tempVarInfo.derefPtr==1){
                        file<<"lw $t0, "<<getLeft()->getOffset(context)<<"($sp)"<<std::endl;
                        file<<"s.d $f8, 0($t0)"<<std::endl;
                    } else {
                        file<<"s.d $f8, "<<getLeft()->getOffset(context)<<"($sp)"<<std::endl;
                    }
                    file<<"mov.d "<<std::string(destReg)<<", $f8"<<std::endl;
                }
            } else{
                getLeft()->generate(file, "$t0", context);
                if(context->tempVarInfo.isPtr==1){
                    if (context->tempVarInfo.type == "double"){
                        file<<"addiu $t0, $t0, 8"<<std::endl;
                    } else {
                        file<<"addiu $t0, $t0, "<<context->tempVarInfo.numBytes<<std::endl;
                    }
                }else {
                    file<<"addiu $t0, $t0, 1"<<std::endl;
                }
                if(context->tempVarInfo.derefPtr==1){
                    file<<"lw $t1, "<<getLeft()->getOffset(context)<<"($sp)"<<std::endl;
                    if(getLeft()->getVarType(context)=="int")  {
                        file<<"sw $t0, 0($t1)"<<std::endl;
                    }
                    else if(getLeft()->getVarType(context)=="char")    {
                        file<<"sb $t0, 0($t1)"<<std::endl;
                    }
                } else { 
                    if(getLeft()->getVarType(context)=="int"||getLeft()->getVarType(context)=="float"||getLeft()->getVarType(context)=="double")  {
                        file<<"sw $t0, "<<getLeft()->getOffset(context)<<"($sp)"<<std::endl;
                    }
                    else if(getLeft()->getVarType(context)=="char")    {
                        file<<"sb $t0, "<<getLeft()->getOffset(context)<<"($sp)"<<std::endl;
                    }
                }
                if (std::string(destReg)!="$f0"){
                    file<<"move "<<std::string(destReg)<<", $t0"<<std::endl;
                }
            }
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
            std::string type = getLeft()->getVarType(context);
            int ptr = getLeft()->getPointer(context);
            if ((type == "double" || type == "float")&& ptr == 0){
                getLeft()->generate(file, "$f6", context);
                if (type == "float"){
                    file<<"l.s $f8, ONE_Float"<<std::endl;
                    file<<"sub.s $f8, $f6, $f8"<<std::endl;
                    if (context->tempVarInfo.derefPtr==1){
                        file<<"lw $t0, "<<getLeft()->getOffset(context)<<"($sp)"<<std::endl;
                        file<<"s.s $f8, 0($t0)"<<std::endl;
                    } else {
                        file<<"s.s $f8, "<<getLeft()->getOffset(context)<<"($sp)"<<std::endl;
                    }
                    file<<"mov.s "<<std::string(destReg)<<", $f6"<<std::endl;
                } else if(type == "double"){
                    file<<"l.d $f8, ONE_Double"<<std::endl;
                    file<<"sub.d $f8, $f6, $f8"<<std::endl;
                    if (context->tempVarInfo.derefPtr==1){
                        file<<"lw $t0, "<<getLeft()->getOffset(context)<<"($sp)"<<std::endl;
                        file<<"s.d $f8, 0($t0)"<<std::endl;
                    } else {
                        file<<"s.d $f8, "<<getLeft()->getOffset(context)<<"($sp)"<<std::endl;
                    }
                    file<<"mov.d "<<std::string(destReg)<<", $f8"<<std::endl;
                }
            } else{
                getLeft()->generate(file, "$t0", context);
                if(context->tempVarInfo.isPtr==1){
                    if (context->tempVarInfo.type == "double"){
                        file<<"addiu $t0, $t0, -8"<<std::endl;
                    } else {
                        file<<"addiu $t0, $t0, -"<<context->tempVarInfo.numBytes<<std::endl;
                    }
                }else {
                    file<<"addiu $t0, $t0, -1"<<std::endl;
                }
                if(context->tempVarInfo.derefPtr==1){
                    file<<"lw $t1, "<<getLeft()->getOffset(context)<<"($sp)"<<std::endl;
                    if(getLeft()->getVarType(context)=="int")  {
                        file<<"sw $t0, 0($t1)"<<std::endl;
                    }
                    else if(getLeft()->getVarType(context)=="char")    {
                        file<<"sb $t0, 0($t1)"<<std::endl;
                    }
                } else { 
                    if(getLeft()->getVarType(context)=="int"||getLeft()->getVarType(context)=="float"||getLeft()->getVarType(context)=="double")  {
                        file<<"sw $t0, "<<getLeft()->getOffset(context)<<"($sp)"<<std::endl;
                    }
                    else if(getLeft()->getVarType(context)=="char")    {
                        file<<"sb $t0, "<<getLeft()->getOffset(context)<<"($sp)"<<std::endl;
                    }
                }
                if (std::string(destReg)!="$f0"){
                    file<<"move "<<std::string(destReg)<<", $t0"<<std::endl;
                }
            }
        }
};

#endif