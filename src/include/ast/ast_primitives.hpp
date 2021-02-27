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

        virtual long getOffset(Context &context) const override  {
            std::unordered_map<std::string,varInfo>::iterator it;
            int n=context.stack.lut.size()-1;
            for(int i=n;i>=0;i--)   {
                it=context.stack.lut.at(i).find(getID());
                if(it!=context.stack.lut.at(i).end()) {
                    break;
                }
            }
            long offset = context.stack.size - it->second.offset;
            return offset;
        }

        virtual std::string getVarType(Context &context) const override {
            std::unordered_map<std::string,varInfo>::iterator it;
            int n=context.stack.lut.size()-1;
            for(int i=n;i>=0;i--)   {
                it=context.stack.lut.at(i).find(getID());
                if(it!=context.stack.lut.at(i).end()) {
                    break;
                }
            }
            return it->second.type;
        }

        virtual void print(std::ostream &dst) const override    {
            dst<<id;
        }

        virtual void generate(std::ofstream &file, const char* destReg, Context &context) const override {
            std::unordered_map<std::string,varInfo>::iterator it;
            int n=context.stack.lut.size()-1;
            for(int i=n;i>=0;i--)   {
                it=context.stack.lut.at(i).find(getID());
                if(it!=context.stack.lut.at(i).end()) {
                    break;
                }
            }
            long offset = context.stack.size - it->second.offset;
            if(it->second.type=="int")  {                
                file<<"lw "<<std::string(destReg)<<", "<<offset<<"($sp)"<<std::endl;
            }
            else if(it->second.type=="char")    {
                file<<"lb "<<std::string(destReg)<<", "<<offset<<"($sp)"<<std::endl;
            }
        }
};

class Array : public Program {
    private:
        std::string id;
        ProgramPtr index;
    public:
        Array(std::string *_id, ProgramPtr _index) : id(*_id), index(_index) {
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
            dst<<getID()<<"[";
            index->print(dst);
            dst<<"]";
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

        virtual void generate(std::ofstream &file, const char* destReg, Context & context) const override    {
            file<<"li "<<std::string(destReg)<<", "<<getValue()<<std::endl;     // li {destReg}, {value}
        }
};

#endif