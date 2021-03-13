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
            int wtf;
            for(int i=n;i>=0;i--)   {
                wtf=i;
                it=context->stack.lut.at(i).find(getID());
                if(it!=context->stack.lut.at(i).end()) {
                    context->tempVarInfo = it->second;
                    if(i>0)    {
                        long offset = context->stack.size - it->second.offset;
                        if(it->second.type=="int")  {                
                            file<<"lw "<<std::string(destReg)<<", "<<offset<<"($sp)"<<std::endl;
                        }
                        else if(it->second.type=="char")    {
                            file<<"lb "<<std::string(destReg)<<", "<<offset<<"($sp)"<<std::endl;
                        }
                    }
                    else    {   // insert code for global variable reference

                    }
                    break;
                }
            }
            // if(it!=context->stack.lut.at(wtf).end()) {
            //     context->tempVarInfo = it->second;
            //     long offset = context->stack.size - it->second.offset;
            //     if(it->second.type=="int")  {                
            //         file<<"lw "<<std::string(destReg)<<", "<<offset<<"($sp)"<<std::endl;
            //     }
            //     else if(it->second.type=="char")    {
            //         file<<"lb "<<std::string(destReg)<<", "<<offset<<"($sp)"<<std::endl;
            //     }
            // }            
        }
};

class ArrayIndex : public Program {
    private:
        ProgramPtr value;
        ProgramPtr next=nullptr;
    public:
        ArrayIndex(ProgramPtr _value, ProgramPtr _next) : value(_value), next(_next)    {}

        ~ArrayIndex()   {
            delete value;
            delete next;
        }

        virtual long spaceRequired() const override {
            long tmp = value->spaceRequired();
            if(next!=nullptr)   {
                tmp+=next->spaceRequired();
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
            if(next!=nullptr)   {
                context->indexCounter++;
                next->generate(file, "$t9", context);
                context->indexCounter--;
            }
            value->generate(file, "$t5", context);
            file<<"li $t4, "<<context->vfPointer->blockSize.at(context->indexCounter)<<std::endl;
            file<<"mult $t4, $t5"<<std::endl;
            file<<"mflo "<<std::string(destReg)<<std::endl;
            if(next!=nullptr)   {
                file<<"addu "<<destReg<<", "<<destReg<<", $t9"<<std::endl;
            }
        }
};

class Array : public Program {
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
            int wtf;
            for(int i=n;i>=0;i--)   {
                wtf=i;
                it=context->stack.lut.at(i).find(getID());
                if(it!=context->stack.lut.at(i).end()) {
                    context->tempVarInfo = it->second;
                    context->vfPointer = &it->second;
                    context->indexCounter=0;
                    index->generate(file, "$t8", context);  // load element relative offset into $t8
                    if(i>0)    {
                        file<<"lw $t5, "<<(context->stack.size - it->second.offset)<<"($sp)"<<std::endl;    // load array base address into $t5
                        file<<"addu $t9, $t5, $t8"<<std::endl;      // add element offset to base address to get element address
                        if(it->second.type=="int")  {          
                            file<<"lw "<<std::string(destReg)<<", 0($t9)"<<std::endl;      
                            // file<<"lw "<<std::string(destReg)<<", "<<offset<<"($sp)"<<std::endl;
                        }
                        else if(it->second.type=="char")    {
                            file<<"lb "<<std::string(destReg)<<", 0($t9)"<<std::endl;
                            // file<<"lb "<<std::string(destReg)<<", "<<offset<<"($sp)"<<std::endl;
                        }
                    }
                    else    {   // insert code for global variable reference

                    }
                    break;
                }
            }
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