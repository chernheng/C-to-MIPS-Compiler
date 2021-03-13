#ifndef COMPILER_AST_DECLARATIONS_HPP
#define COMPILER_AST_DECLARATIONS_HPP

// #include "src/include/ast.hpp"
#include "variable_table.hpp"

class DeclareVariable : public Program {
    private:
        std::string type;
        std::string id;
        ProgramPtr init=nullptr; //int x = 5;
        int ptr=0;
    public:
        DeclareVariable(std::string *_type, std::string *_id, ProgramPtr _init, int _ptr) : type(*_type), id(*_id), init(_init), ptr(_ptr)  {
            delete _type;
            delete _id;
        }

        DeclareVariable(std::string *_type, std::string *_id, int _ptr) : type(*_type),id(*_id), ptr(_ptr)   {
            delete _type;
            delete _id;
        }

        ~DeclareVariable() {
            delete init;
        }

        std::string getID() const   {
            return id;
        }

        std::string getType() const {
            return type;
        }

        int getPtr() const {
            return ptr;
        }

        virtual void print(std::ostream &dst) const override    {
            dst<<type<<" "<<id;
            if(init!=nullptr)    {
                dst<<"=";
                init->print(dst);
            }
            dst<<";";
        }

        virtual long spaceRequired() const override {
            if(type=="int" || type=="char")  {
                return 4;
            }
            else    {       // temporary, replace later with size requirements of other variable types
                return 0;
            }
        }

        virtual void generate(std::ofstream &file, const char* destReg, Context *context) const override {
            long size = context->stack.lut.size();
            long offset = context->stack.slider;
            varInfo vf;
            vf.offset=offset;
            vf.length=1;
            vf.isPtr =getPtr();
            int stackInc=0;
            if(getType()=="int")    {
                vf.type="int";
                vf.numBytes=4;
                stackInc=4;
            }
            else if(getType()=="char")  {
                vf.type="char";
                vf.numBytes=1;
                stackInc=1;
            }
            if (size == 1) {
                vf.isGlobal = 1;
                if (init!= nullptr){
                    init->generate(file, "$t7", context);
                    std::string value = context->numVal;
                    file<<"   .globl  "<<getID()<<std::endl;
                    file<<"   .type   "<<getID()<<", @object"<<std::endl;
                    file<<"   .size   "<<getID()<<", "<<vf.numBytes<<std::endl;
                    file<<getID()<<":"<<std::endl;
                    file<<"   .word   "<<value<<std::endl;
                } else if (init == nullptr){
                    file<<"   .globl  "<<getID()<<std::endl;
                    file<<"   .type   "<<getID()<<", @object"<<std::endl;
                    file<<"   .section        .bss,\"aw\",@nobits"<<std::endl;
                    file<<"   .size   "<<getID()<<", "<<vf.numBytes<<std::endl;
                    file<<getID()<<":"<<std::endl;
                    file<<"   .space   "<<vf.numBytes<<std::endl;
                }
            }
            context->stack.lut.back().insert(std::pair<std::string,varInfo>(getID(),vf));

            if(init!=nullptr)   {
                if (size != 1){
                    init->generate(file, "$t7", context);
                }
                if(stackInc==4) {
                    file<<"sw $t7, "<<(context->stack.size - offset)<<"($sp)"<<std::endl;
                }
                else if(stackInc==1)    {
                    file<<"sb $t7, "<<(context->stack.size - offset)<<"($sp)"<<std::endl;
                }
            }
            context->stack.slider+=4;
            file<<"li "<<std::string(destReg)<<", 1"<<std::endl;
            return;                        
        }
};

class DeclareArrayElement : public Program {
    private:
        long n;
        DeclareArrayElement *next=nullptr;
    public:
        DeclareArrayElement(std::string *_num, DeclareArrayElement *_next) : next(_next)  {
            n=std::stol(*_num);
            delete _num;
        }

        ~DeclareArrayElement() {
            delete next;
        }

        virtual long spaceRequired() const override {   // returns number of elements in current and subsequent nodes
            if(next!=nullptr)   {
                return n * next->spaceRequired();
            }
            else    {
                return n;
            }
        }

        virtual void print(std::ostream &dst) const override    {
            dst<<"["<<n<<"]";
            if(next!=nullptr)   {
                next->print(dst);
            }
        }

        virtual void generate(std::ofstream &file, const char* destReg, Context *context) const override    {
            context->vfPointer->dimension.push_back(n);
            context->vfPointer->blockSize.push_back(0);
            if(next!=nullptr)   {
                next->generate(file, destReg, context);
            }
        }
};

class DeclareArray : public Program {
    private:
        std::string type;
        std::string id;
        DeclareArrayElement *dimensions;
        ProgramPtr init=nullptr; //int x = 5;
        int ptr=0;
    public:
        DeclareArray(std::string *_type, std::string *_id, DeclareArrayElement *_dimens, ProgramPtr _init) : type(*_type), id(*_id), dimensions(_dimens), init(_init) {
            delete _type;
            delete _id;
        }

        ~DeclareArray() {
            delete dimensions;
            delete init;
        }

        std::string getID() const   {
            return id;
        }

        std::string getType() const {
            return type;
        }

        virtual long spaceRequired() const override {
            long tmp=4;     // for array base pointer
            if(type=="int") {
                tmp += 4*dimensions->spaceRequired();
            }
            else if(type=="char")   {
                tmp += dimensions->spaceRequired();
            }
            else if(type=="float")  {
                tmp +=4*dimensions->spaceRequired();
            }
            else    {       // for structs/typedefs
                tmp += 0;   // change later
            }
            return tmp;
        }

        virtual void print(std::ostream &dst) const override    {
            dst<<type<<" "<<id;
            dimensions->print(dst);
            dst<<";"<<std::endl;
        }

        virtual void generate(std::ofstream &file, const char* destReg, Context *context) const override   {
            varInfo vf;
            context->vfPointer=&vf;
            if(type=="int") {
                vf.numBytes=4;
                vf.type="int";
            }
            else if(type=="char")   {
                vf.numBytes=1;
                vf.type="char";
            }
            vf.length = dimensions->spaceRequired()-4;    // number of elements (removing 4 bytes used for base pointer)
            vf.blockSize.back() = vf.numBytes;
            for(long i=vf.blockSize.size()-2;i>=0;i--)  {                          // build block table
                vf.blockSize.at(i) = vf.dimension.at(i+1) * vf.blockSize.at(i+1);
            }
            if(context->stack.lut.size()==1)    {   // global array
                vf.isGlobal = 1;
            }
            else    {                               // local array
                // vf.offset=context->stack.slider;
                long space=vf.numBytes*vf.length;
                if(space % 4)   {
                    space+=4-(space%4);
                }
                context->stack.slider+=space;
                vf.offset=context->stack.slider;
            }
            context->stack.lut.back().insert(std::pair<std::string,varInfo>(getID(),vf));
        }
};

class DeclareFunction : public Program {
    private:
        std::string type;
        std::string id;
    public:
        DeclareFunction(std::string *_type, std::string *_id) : type(*_type), id(*_id)  {
            delete _type;
            delete _id;
        }

        virtual void print(std::ostream &dst) const override    {
            dst<<type<<" "<<id<<"()"; //int f();
            dst<<";";
        }
};

#endif