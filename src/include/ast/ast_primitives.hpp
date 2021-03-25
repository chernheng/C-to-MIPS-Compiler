#ifndef COMPILER_AST_PRIMITIVES_HPP
#define COMPILER_AST_PRIMITIVES_HPP

// #include "src/include/ast.hpp"
#include "variable_table.hpp"

class Variable : public Program {
    private:
        std::string id;
    public:
        Variable(std::string *_id) : id(*_id) {
            delete _id;
        }

        std::string getID() const   {
            return id;
        }

        virtual long getOffset(Context *context) const override  {  // returns offset from current $sp
            std::unordered_map<std::string,varInfo>::iterator it;
            int n=context->stack.lut.size()-1;
            for(int i=n;i>=0;i--)   {
                it=context->stack.lut.at(i).find(getID());
                if(it!=context->stack.lut.at(i).end()) {
                    long offset = context->stack.size - it->second.offset;
                    return offset;
                }
            }
            return 0;
        }

        virtual std::string getVarType(Context *context) const override {
            std::unordered_map<std::string,varInfo>::iterator it;
            int n=context->stack.lut.size()-1;
            for(int i=n;i>=0;i--)   { //iterating through the vector of maps from the last map as it is the latest scope
                it=context->stack.lut.at(i).find(getID());
                if(it!=context->stack.lut.at(i).end()) { // iterating through each map, and if cant be found, move on ot next map
                    return it->second.type;
                }
            }
            return "";
        }

        virtual void print(std::ostream &dst) const override    {
            dst<<id;
        }

        virtual void generate(std::ofstream &file, const char* destReg, Context *context) const override {
            std::unordered_map<std::string,varInfo>::iterator it;
            int n=context->stack.lut.size()-1;
            for(int i=n;i>=0;i--)   {
                it=context->stack.lut.at(i).find(getID());
                if(it!=context->stack.lut.at(i).end()) {
                    context->tempVarInfo = it->second;
                    if(i>0)    { //not global
                        long offset = context->stack.size - it->second.offset;
                        if(context->tempVarInfo.isPtr==1) {
                            file<<"lw "<<std::string(destReg)<<", "<<offset<<"($sp)"<<std::endl;
                        }
                        else if(it->second.isFP ==1) {
                            if(it->second.numBytes == 4){
                                file<<"l.s "<<std::string(destReg)<<", "<<offset<<"($sp)"<<std::endl;
                            }
                            if(it->second.numBytes == 8){
                                file<<"l.d "<<std::string(destReg)<<", "<<offset-4<<"($sp)"<<std::endl;
                            }
                        }
                        else if(it->second.numBytes==1)    {
                            if(it->second.isUnsigned==1)    {
                                file<<"lbu "<<std::string(destReg)<<", "<<offset<<"($sp)"<<std::endl;
                            }
                            else{
                                file<<"lb "<<std::string(destReg)<<", "<<offset<<"($sp)"<<std::endl;
                            }                            
                        }
                        else    {
                            file<<"lw "<<std::string(destReg)<<", "<<offset<<"($sp)"<<std::endl;
                        }
                    }
                    else    {   // insert code for global variable reference
                        std::string id = it->first;
                        if(it->second.isFP ==1) {
                            if(it->second.numBytes == 4){
                                file<<"l.s "<<std::string(destReg)<<", ("<<id<<")"<<std::endl;
                            }
                            if(it->second.numBytes == 8){
                                file<<"l.d "<<std::string(destReg)<<", ("<<id<<")"<<std::endl;
                            }
                            
                        }else {
                            if(it->second.numBytes==1)    {
                                file<<"lui "<<std::string(destReg)<<", \%hi("<<id<<")"<<std::endl;
                                file<<"lbu "<<std::string(destReg)<<", \%lo("<<id<<")("<<std::string(destReg)<<")"<<std::endl;
                            } else {
                                file<<"lui "<<std::string(destReg)<<", \%hi("<<id<<")"<<std::endl;
                                file<<"lw "<<std::string(destReg)<<", \%lo("<<id<<")("<<std::string(destReg)<<")"<<std::endl;
                            }
                        }
                    }
                    break;
                }
            }           
        }
};

class VariableStore : public Program {  // store vale into variable
    private:
        std::string id;
        int ptr =0;
    public:
        VariableStore(std::string *_id, int _ptr) : id(*_id), ptr(_ptr)  {
            delete _id;
        }

        std::string getID() const   {
            return id;
        }
        
        int getPtr() const{
            return ptr;
        }

        virtual void print(std::ostream &dst) const override    {
            if (getPtr() == 1){
                dst<<"*";
            }
            dst<<id;
        }

        virtual void generate(std::ofstream &file, const char* destReg, Context *context) const override    {   // value to store comes in destReg
            std::unordered_map<std::string,varInfo>::iterator it;
            int n=context->stack.lut.size()-1;
            for(int i=n;i>=0;i--)   {
                it=context->stack.lut.at(i).find(getID());
                if(it!=context->stack.lut.at(i).end()) {
                    context->tempVarInfo = it->second;
                    if(i>0)    { //not global (write to local variable)
                        long offset = context->stack.size - it->second.offset;
                        if (getPtr()==0){
                            if(it->second.isFP == 1){
                                if (it->second.numBytes == 4){
                                    file <<"s.s "<<destReg<<", "<<(context->stack.size - it->second.offset)<<"($sp)"<<std::endl;
                                } else if(it->second.numBytes == 8) {
                                    file <<"s.d "<<destReg<<", "<<(context->stack.size - it->second.offset-4)<<"($sp)"<<std::endl;
                                }
                            }else if(it->second.numBytes==1)    {
                                file<<"sb "<<destReg<<", "<<(context->stack.size - it->second.offset)<<"($sp)"<<std::endl;
                            }
                            else    {
                                file<<"sw "<<destReg<<", "<<(context->stack.size - it->second.offset)<<"($sp)"<<std::endl;
                            }
                        } else if (getPtr()==1){
                            if(it->second.numBytes==1)    {
                                file<<"lw $t1, "<<(context->stack.size - it->second.offset)<<"($sp)"<<std::endl;
                                file<<"sb "<<destReg<<", 0($t1)"<<std::endl;
                            }
                            else    {
                                file<<"lw $t1, "<<(context->stack.size - it->second.offset)<<"($sp)"<<std::endl;
                                file<<"sw "<<destReg<<", 0($t1)"<<std::endl;
                            }
                        }
                    }
                    else    {   // insert code for storing to global variable reference
                        if(it->second.isFP ==1) {
                            if(it->second.numBytes == 4){
                                file<<"s.s "<<std::string(destReg)<<", ("<<id<<")"<<std::endl;
                            }
                            if(it->second.numBytes == 8){
                                file<<"s.d "<<std::string(destReg)<<", ("<<id<<")"<<std::endl;
                            }
                        }else {
                            if(it->second.numBytes==1)    {
                                file<<"lui $t1, %hi("<<id<<")"<<std::endl;
                                file<<"sb "<<std::string(destReg)<<", %lo("<<id<<")($t1)"<<std::endl;
                            }
                            else    {
                                file<<"lui $t1, %hi("<<id<<")"<<std::endl;
                                file<<"sw "<<std::string(destReg)<<", %lo("<<id<<")($t1)"<<std::endl;
                            }
                        }
                    }
                    break;
                }
            }
        }
};

class ArrayIndex : public Program { // handle index for array access
    private:
        ProgramPtr value;
        ProgramPtr next=nullptr;
    public:
        ArrayIndex(ProgramPtr _value, ProgramPtr _next) : value(_value), next(_next)    {}

        ~ArrayIndex()   {
            delete value;
            delete next;
        }

        virtual long spaceRequired(Context *context) const override {
            long tmp = value->spaceRequired(context);
            if(next!=nullptr)   {
                tmp+=next->spaceRequired(context);
            }
            return tmp;
        }

        virtual void print(std::ostream &dst) const override    {
            dst<<"[";
            value->print(dst);
            dst<<"]";
            if(next!=nullptr)   {
                next->print(dst);
            }
        }

        virtual void generate(std::ofstream &file, const char* destReg, Context *context) const override    {
            varInfo arrInfo = context->tempVarInfo;
            if(next!=nullptr)   {
                context->indexCounter++;
                next->generate(file, "$t9", context);
                context->indexCounter--;
            }
            value->generate(file, "$t5", context);
            if (arrInfo.isPtr == 1){
                if(arrInfo.type == "int"){
                    file<<"li $t4, 4"<<std::endl;
                }else if(arrInfo.type == "char"){
                    file<<"li $t4, 1"<<std::endl;
                }
            }else {
                file<<"li $t4, "<<context->vfPointer->blockSize.at(context->indexCounter)<<std::endl;   // load block size (problematic line)
            }
            file<<"mult $t4, $t5"<<std::endl;
            file<<"mflo "<<std::string(destReg)<<std::endl;
            if(next!=nullptr)   {
                file<<"addu "<<destReg<<", "<<destReg<<", $t9"<<std::endl;
            }
        }
};

class Array : public Program {  // read value in array
    private:
        std::string id;
        ArrayIndex *index;
    public:
        Array(std::string *_id, ArrayIndex *_index) : id(*_id), index(_index) {
            delete _id;
        }

        ~Array() {
            delete index;
        }

        std::string getID() const {
            return id;
        }

        ProgramPtr getIndex() const    {
            return index;
        }

        virtual void print(std::ostream &dst) const override    {
            dst<<getID();
            index->print(dst);
        }

        virtual void generate(std::ofstream &file, const char* destReg, Context *context) const override    {
            std::unordered_map<std::string,varInfo>::iterator it;
            int n=context->stack.lut.size()-1;
            for(int i=n;i>=0;i--)   {
                it=context->stack.lut.at(i).find(getID());
                if(it!=context->stack.lut.at(i).end()) {
                    context->tempVarInfo = it->second;
                    context->vfPointer = &it->second;
                    context->indexCounter=0;
                    index->generate(file, "$t8", context);  // load element relative offset into $t8
                    if(i>0)    {
                        file<<"lw $t5, "<<(context->stack.size - it->second.offset)<<"($sp)"<<std::endl;    // load array base address into $t5
                        file<<"addu $t9, $t5, $t8"<<std::endl;      // add element offset to base address to get element address
                        if(it->second.numBytes==1)    {
                            if(it->second.isUnsigned==1)    {
                                file<<"lbu "<<std::string(destReg)<<", 0($t9)"<<std::endl;
                            }
                            else    {
                                file<<"lb "<<std::string(destReg)<<", 0($t9)"<<std::endl;
                            }                            
                        }
                        else    {
                            file<<"lw "<<std::string(destReg)<<", 0($t9)"<<std::endl;   
                        }
                    }
                    else    {   // insert code for global variable reference
                        file<<"lui "<<std::string(destReg)<<", %hi("<<getID()<<")"<<std::endl;
                        file<<"addiu "<<std::string(destReg)<<", "<<std::string(destReg)<<", %lo("<<getID()<<")"<<std::endl;
                        file<<"addu "<<std::string(destReg)<<", "<<std::string(destReg)<<", $t8"<<std::endl;
                        if(it->second.numBytes==1)    {
                            file<<"lb "<<std::string(destReg)<<", 0("<<std::string(destReg)<<")"<<std::endl;
                        }
                        else    {
                            file<<"lw "<<std::string(destReg)<<", 0("<<std::string(destReg)<<")"<<std::endl;
                        }

                    }
                    break;
                }
            }
        }
};

class ArrayStore : public Program {     // store value into array
    private:
        std::string id;
        ArrayIndex *index;
    public:
        ArrayStore(std::string *_id, ArrayIndex *_index) : id(*_id), index(_index) {
            delete _id;
        }

        ~ArrayStore() {
            delete index;
        }

        std::string getID() const {
            return id;
        }

        ProgramPtr getIndex() const    {
            return index;
        }

        virtual long spaceRequired(Context *context) const override {
            return 4;
        }

        virtual void print(std::ostream &dst) const override    {
            dst<<getID();
            index->print(dst);
        }

        virtual void generate(std::ofstream &file, const char* destReg, Context *context) const override    {
            std::unordered_map<std::string,varInfo>::iterator it;
            int offset = context->stack.slider;
            file<<"sw "<<std::string(destReg)<<", "<<(context->stack.size - offset)<<"($sp)"<<std::endl;
            context->stack.slider+=4;
            int n=context->stack.lut.size()-1;
            for(int i=n;i>=0;i--)   {
                it=context->stack.lut.at(i).find(getID());
                if(it!=context->stack.lut.at(i).end()) {
                    context->tempVarInfo = it->second;
                    context->vfPointer = &it->second;
                    context->indexCounter=0;
                    index->generate(file, "$t8", context);  // load element relative offset into $t8
                    file<<"lw $t0, "<<(context->stack.size - offset)<<"($sp)"<<std::endl;
                    if(i>0)    {
                        file<<"lw $t5, "<<(context->stack.size - it->second.offset)<<"($sp)"<<std::endl;    // load array base address into $t5
                        file<<"addu $t9, $t5, $t8"<<std::endl;      // add element offset to base address to get element address
                        if(it->second.numBytes==1)    {
                            file<<"sb $t0, 0($t9)"<<std::endl;
                        }
                        else    {
                            file<<"sw $t0, 0($t9)"<<std::endl;
                        }
                    }
                    else    {   // insert code for global variable reference
                        file<<"lui $t1, %hi("<<getID()<<")"<<std::endl;
                        file<<"addiu $t1, $t1, %lo("<<getID()<<")"<<std::endl;
                        file<<"addu $t1, $t1, $t8"<<std::endl;
                        if(it->second.numBytes==1)    {
                            file<<"sb $t0, 0($t1)"<<std::endl;
                        }
                        else    {
                            file<<"sw $t0, 0($t1)"<<std::endl;
                        }
                    }
                    break;
                }
            }
            context->stack.slider-=4;
        }
};

class Float : public Program {
    private:
        std::string value;
    public:
        Float(std::string *_value) : value(*_value)  {
            delete _value;
        }

        std::string getValue() const    {
            return value;
        }

        virtual void print(std::ostream &dst) const override    {
            dst<<getValue();
        }

        virtual void generate(std::ofstream &file, const char* destReg, Context *context) const override    {
            varInfo tmp;
            std::string val = getValue();
            val.pop_back();
            context->numVal = val;
            tmp.FP_value = val;
            tmp.isFP = 1;
            tmp.numBytes = 4;
            std::string FloatLabel = makeLabel("Float");
            tmp.FP_label = FloatLabel;
            file<<"l.s "<<std::string(destReg)<<", "<<FloatLabel<<std::endl;
            context->tempVarInfo = tmp;
            context->FP.push_back(tmp);
        }
};

class Double : public Program {
    private:
        std::string value;
    public:
        Double(std::string *_value) : value(*_value)  {
            delete _value;
        }

        std::string getValue() const    {
            return value;
        }

        virtual void print(std::ostream &dst) const override    {
            dst<<getValue();
        }

        virtual void generate(std::ofstream &file, const char* destReg, Context *context) const override    {
            varInfo tmp;
            context->numVal = getValue();
            tmp.FP_value = getValue();
            tmp.isFP = 1;
            tmp.numBytes = 8;
            std::string DoubleLabel = makeLabel("Double");
            tmp.FP_label = DoubleLabel;
            file<<"l.d "<<std::string(destReg)<<", "<<DoubleLabel<<std::endl;
            context->tempVarInfo = tmp;
            context->FP.push_back(tmp);
        }
};

class Number : public Program {
    private:
        std::string value;
    public:
        Number(std::string *_value) : value(*_value)  {
            delete _value;
        }

        std::string getValue() const    {
            return value;
        }

        virtual void print(std::ostream &dst) const override    {
            dst<<getValue();
        }

        virtual void generate(std::ofstream &file, const char* destReg, Context *context) const override    {
            varInfo tmp;
            context->tempVarInfo = tmp;
            context->numVal = getValue();
            file<<"li "<<std::string(destReg)<<", "<<getValue()<<std::endl;     // li {destReg}, {value}
        }
};

class AccessStructElement : public Program {
    private:
        std::string id;
        AccessStructElement *next=nullptr;
    public:
        AccessStructElement(std::string *_id, AccessStructElement *_next)  : id(*_id), next(_next)  {
            delete _id;
        }

        ~AccessStructElement()  {
            delete next;
        }

        virtual void print(std::ostream &dst) const override    {
            dst<<id;
            if(next!=nullptr)   {
                dst<<".";
                next->print(dst);
            }
        }

        virtual void generate(std::ofstream &file, const char* destReg, Context *context) const override    {   // gets offset of element relative to struct base
            std::unordered_map<std::string,varInfo>::iterator it1;
            it1 = context->stPointer->structElements.find(id);
            context->tempVarInfo.numBytes *= it1->second.numBytes;
            if(it1->second.isPtr > context->tempVarInfo.isPtr)  {
                context->tempVarInfo.isPtr = it1->second.isPtr;
            }
            if(it1->second.isUnsigned > context->tempVarInfo.isUnsigned)    {
                context->tempVarInfo.isUnsigned= it1->second.isUnsigned;
            }
            file<<"addiu "<<std::string(destReg)<<", "<<it1->second.offset<<std::endl;
            if(next!=nullptr)   {
                next->generate(file, destReg, context);
            }
        }
};

class StructRead : public Program {
    private:
        std::string id;
        AccessStructElement *element;
    public:
        StructRead(std::string *_id, AccessStructElement *_ele) : id(*_id), element(_ele)  {
            delete _id;
        }

        ~StructRead()   {
            delete element;
        }

        virtual void print(std::ostream &dst) const override    {
            dst<<id<<".";
            element->print(dst);
        }

        virtual void generate(std::ofstream &file, const char* destReg, Context *context) const override    {
            structInfo *initSTP = context->stPointer;
            varInfo initVF=context->tempVarInfo;
            context->tempVarInfo.numBytes=1;
            context->tempVarInfo.isPtr=0;
            context->tempVarInfo.isUnsigned=0;
            std::unordered_map<std::string,structInfo>::iterator sIT;            
            std::unordered_map<std::string,varInfo>::iterator it;
            int n=context->stack.lut.size()-1;
            for(int i=n;i>=0;i--)   {                       // get base address of struct instance
                it=context->stack.lut.at(i).find(id);
                if(it!=context->stack.lut.at(i).end())  {
                    if(i>0) {   // local struct instance 
                        sIT = context->structTable.find(it->second.type);    // find struct info
                        context->stPointer = &sIT->second;
                        file<<"move $t4, $zero"<<std::endl;
                        element->generate(file, "$t4", context);  // get element offset
                        file<<"lw $t3, "<<(context->stack.size - it->second.offset)<<"($sp)"<<std::endl; // load base address to $t3
                        file<<"addu $t5, $t3, $t4"<<std::endl;
                        if(context->tempVarInfo.numBytes==1 && context->tempVarInfo.isPtr==0) {
                            if(context->tempVarInfo.isUnsigned==1)  {
                                file<<"lbu "<<std::string(destReg)<<", 0($t5)"<<std::endl;
                            }
                            else    {
                                file<<"lb "<<std::string(destReg)<<", 0($t5)"<<std::endl;
                            }
                        }
                        else    {
                            file<<"lw "<<std::string(destReg)<<", 0($t5)"<<std::endl;
                        }                        
                    }
                    else    {   // global struct instance 

                    }
                    break;
                }                
            }
            context->tempVarInfo=initVF;
            context->stPointer=initSTP;
        }
};

class StructStore : public Program {
    private:
        std::string id;
        AccessStructElement *element;
    public:
        StructStore(std::string *_id, AccessStructElement *_ele) : id(*_id), element(_ele)  {
            delete _id;
        }

        ~StructStore()  {
            delete element;
        }

        virtual void print(std::ostream &dst) const override    {
            dst<<id<<".";
            element->print(dst);
        }

        virtual void generate(std::ofstream &file, const char* destReg, Context *context) const override    {       // value comes in destReg       
            file<<"move $t0, "<<std::string(destReg)<<std::endl;    // temporarily sotre value into $t0
            structInfo *initSTP = context->stPointer;
            varInfo initVF=context->tempVarInfo;
            context->tempVarInfo.numBytes=1;
            context->tempVarInfo.isPtr=0;
            context->tempVarInfo.isUnsigned=0;
            std::unordered_map<std::string,structInfo>::iterator sIT;            
            std::unordered_map<std::string,varInfo>::iterator it;
            int n=context->stack.lut.size()-1;
            for(int i=n;i>=0;i--)   {                       // get base address of struct instance
                it=context->stack.lut.at(i).find(id);
                if(it!=context->stack.lut.at(i).end())  {
                    if(i>0) {                                       // local struct instance 
                        sIT = context->structTable.find(it->second.type);    // find struct info
                        context->stPointer = &sIT->second;
                        file<<"move $t4, $zero"<<std::endl;
                        element->generate(file, "$t4", context);  // get element offset
                        file<<"lw $t3, "<<(context->stack.size - it->second.offset)<<"($sp)"<<std::endl; // load base address to $t3
                        file<<"addu $t5, $t3, $t4"<<std::endl;
                        if(context->tempVarInfo.numBytes==1 && context->tempVarInfo.isPtr==0) {
                            file<<"sb "<<std::string(destReg)<<", 0($t5)"<<std::endl;
                        }
                        else    {
                            file<<"sw "<<std::string(destReg)<<", 0($t5)"<<std::endl;
                        }                        
                    }
                    else    {   // global struct instance 

                    }
                    break;
                }                
            }
            context->tempVarInfo=initVF;
            context->stPointer=initSTP;
        }
};

#endif