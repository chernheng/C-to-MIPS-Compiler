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
                        else if(it->second.numBytes==1)    {
                            file<<"lb "<<std::string(destReg)<<", "<<offset<<"($sp)"<<std::endl;
                        }
                        else    {
                            file<<"lw "<<std::string(destReg)<<", "<<offset<<"($sp)"<<std::endl;
                        }
                    }
                    else    {   // insert code for global variable reference
                        std::string id = it->first;
                        file<<"lui "<<std::string(destReg)<<", \%hi("<<id<<")"<<std::endl;
                        file<<"lw "<<std::string(destReg)<<", \%lo("<<id<<")("<<std::string(destReg)<<")"<<std::endl;
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
                            if(it->second.numBytes==1)    {
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
                        if(it->second.numBytes==1)    {
                            file<<"lui $t1, %hi("<<id<<")"<<std::endl;
                            file<<"sb "<<std::string(destReg)<<", %lo("<<id<<")($t1)"<<std::endl;
                        }
                        else    {
                            file<<"lui $t1, %hi("<<id<<")"<<std::endl;
                            file<<"sw "<<std::string(destReg)<<", %lo("<<id<<")($t1)"<<std::endl;
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
                            file<<"lb "<<std::string(destReg)<<", 0($t9)"<<std::endl;
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

#endif