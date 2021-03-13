#ifndef COMPILER_AST_DECLARATIONS_HPP
#define COMPILER_AST_DECLARATIONS_HPP

// #include "src/include/ast.hpp"
#include "variable_table.hpp"

class DeclareVariable : public Program {
    private:
        std::string type;
        std::string id;
        std::string number;
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

        DeclareVariable(std::string *_type, std::string *_id, std::string *_number, int _ptr) : type(*_type), id(*_id), number(*_number), ptr(_ptr)  {
            delete _type;
            delete _number;
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

        std::string getNumber() const {
            return number;
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
            if ((size == 1) && (getNumber()=="")){
                file<<"   .globl  "<<getID()<<std::endl;
                file<<"   .type   "<<getID()<<", @object"<<std::endl;
                file<<"   .section        .bss,\"aw\",@nobits"<<std::endl;
                file<<"   .size   "<<getID()<<", "<<vf.numBytes<<std::endl;
                file<<getID()<<":"<<std::endl;
                vf.isGlobal = 1;
            } else if((size == 1) && (getNumber()!="")) {
                file<<"   .globl  "<<getID()<<std::endl;
                file<<"   .type   "<<getID()<<", @object"<<std::endl;
                file<<"   .size   "<<getID()<<", "<<vf.numBytes<<std::endl;
                file<<getID()<<":"<<std::endl;
                vf.isGlobal = 1;
            } 
            context->stack.lut.back().insert(std::pair<std::string,varInfo>(getID(),vf));
            if((getNumber()=="")&& (size==1)){
                file<<"   .space   "<<vf.numBytes<<std::endl;
                return;         // end if global variable
            } else if((getNumber()!="")&& (size==1)) {
                file<<"   .word   "<<getNumber()<<std::endl;
            }
            if (getNumber() != ""){
                file<<"li $t7, "<<getNumber()<<std::endl; 
                file<<"sw $t7, "<<(context->stack.size - offset)<<"($sp)"<<std::endl;
            }
            if(init!=nullptr)   {
                init->generate(file, "$t7", context);
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
            if(type=="int") {
                return 4*dimensions->spaceRequired();
            }
            else if(type=="char")   {
                return dimensions->spaceRequired();
            }
            else if(type=="float")  {
                return 4*dimensions->spaceRequired();
            }
            else    {       // for structs/typedefs
                return 0;   // change later
            }
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
            vf.length = dimensions->spaceRequired();    // number of elements
            vf.blockSize.back() = vf.numBytes;
            for(long i=vf.blockSize.size()-2;i>=0;i--)  {                          // build block table
                vf.blockSize.at(i) = vf.dimension.at(i+1) * vf.blockSize.at(i+1);
            }
            if(context->stack.lut.size()==1)    {   // global array
                vf.isGlobal = 1;
            }
            else    {                               // local array
                vf.offset=context->stack.slider;
                long space=vf.numBytes*vf.length;
                if(space % 4)   {
                    space+=4-(space%4);
                }
                context->stack.slider+=space;
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