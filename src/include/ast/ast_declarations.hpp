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
        int isUnsigned=0;
    public:
        DeclareVariable(std::string *_type, std::string *_id, ProgramPtr _init, int _ptr, int _uns) : type(*_type), id(*_id), init(_init), ptr(_ptr), isUnsigned(_uns)  {
            delete _type;
            delete _id;
        }

        DeclareVariable(std::string *_type, std::string *_id, int _ptr, int _uns) : type(*_type),id(*_id), ptr(_ptr), isUnsigned(_uns)   {
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

        virtual long spaceRequired(Context *context) const override {
            long tmp=1;     // store size of 1 element (in bytes)
            std::unordered_map<std::string,typeInfo>::iterator typeIT;
            std::string bind_name = type;
            while(bind_name!="")    {
                typeIT = context->typeTable.find(bind_name);
                bind_name=typeIT->second.type;
                tmp*=typeIT->second.size;
            }
            if(ptr==1)  {
                tmp=4;  // pointer uses 4 bytes
            }
            if(init!=nullptr)   {
                tmp += init->spaceRequired(context);
            }
            return tmp;
        }

        virtual void generate(std::ofstream &file, const char* destReg, Context *context) const override {
            long size = context->stack.lut.size();
            long offset = context->stack.slider;
            varInfo vf;
            vf.offset=offset;
            vf.length=1;
            vf.isPtr =getPtr();
            vf.isUnsigned = isUnsigned;
            vf.type = type;
            vf.numBytes=1;
            int stackInc=1;
            std::unordered_map<std::string,typeInfo>::iterator typeIT;
            std::string bindType = type;
            while(bindType!="") {
                typeIT = context->typeTable.find(bindType);
                bindType = typeIT->second.type;
                vf.numBytes*=typeIT->second.size;
                stackInc*=typeIT->second.size;
                if(typeIT->second.type!="") {
                    vf.type=typeIT->second.type;
                }
                if(typeIT->second.ptr > vf.isPtr)   {
                    vf.isPtr = typeIT->second.ptr;
                }
                if(typeIT->second.isUnsigned > vf.isUnsigned)   {
                    vf.isUnsigned = typeIT->second.isUnsigned;
                }
            }
            if(vf.isPtr==1) {   // set size to be 4 bytes if it is a pointer
                vf.numBytes=4;
            }
            if (size == 1) {
                vf.isGlobal = 1;
                if (init!= nullptr){
                    init->generate(file, "$t7", context);
                    std::string value = context->numVal;
                    file<<"   .data"<<std::endl;
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
            context->stack.slider+=4;
            if((init!=nullptr)&&(size!=1))   {
                if (size != 1){
                    init->generate(file, "$t7", context);
                }
                if (vf.isPtr==1){
                    file<<"sw $t7, "<<(context->stack.size - offset)<<"($sp)"<<std::endl;
                }
                else if(stackInc==4) {
                    file<<"sw $t7, "<<(context->stack.size - offset)<<"($sp)"<<std::endl;
                }
                else if(stackInc==1)    {
                    file<<"sb $t7, "<<(context->stack.size - offset)<<"($sp)"<<std::endl;
                }
            }
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

        virtual long spaceRequired(Context *context) const override {   // returns number of elements in current and subsequent nodes
            if(next!=nullptr)   {
                return n * next->spaceRequired(context);
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
        ProgramPtr init = nullptr; //int x = 5;  (Array_Init class)
        int ptr=0;
        int isUnsigned=0;
    public:
        DeclareArray(std::string *_type, std::string *_id, DeclareArrayElement *_dimens, ProgramPtr _init, int _uns) : type(*_type), id(*_id), dimensions(_dimens), init(_init), isUnsigned(_uns) {
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

        virtual long spaceRequired(Context *context) const override {
            long tmp=4;       // for array base pointer
            long elementSize=1;
            std::unordered_map<std::string,typeInfo>::iterator typeIT;
            std::string bind_name = type;
            while(bind_name!="")    {
                typeIT = context->typeTable.find(bind_name);
                bind_name=typeIT->second.type;
                elementSize*=typeIT->second.size;
            }
            tmp += elementSize*dimensions->spaceRequired(context);
            return tmp;
        }

        virtual void print(std::ostream &dst) const override    {
            dst<<type<<" "<<id;
            dimensions->print(dst);
            if (init!= nullptr){
                dst<<"={";
                init->print(dst);
                dst<<"}";
            }
            dst<<";"<<std::endl;
        }

        virtual void generate(std::ofstream &file, const char* destReg, Context *context) const override   {
            varInfo vf;
            context->vfPointer=&vf;
            vf.isUnsigned = isUnsigned;
            vf.type=type;
            vf.numBytes=1;
            std::unordered_map<std::string,typeInfo>::iterator typeIT;
            std::string bind_name = type;
            while(bind_name!="") {
                typeIT = context->typeTable.find(bind_name);
                bind_name = typeIT->second.type;
                vf.numBytes*=typeIT->second.size;
                if(typeIT->second.type!="") {
                    vf.type=typeIT->second.type;
                }
                if(typeIT->second.isUnsigned > vf.isUnsigned)   {
                    vf.isUnsigned = typeIT->second.isUnsigned;
                }
            }
            vf.isPtr = 1;
            dimensions->generate(file, "t0", context);
            vf.length = dimensions->spaceRequired(context);    // number of elements (removing 4 bytes used for base pointer) (ok maybe not)
            vf.blockSize.back() = vf.numBytes;
            for(long i=vf.blockSize.size()-2;i>=0;i--)  {                          // build block table
                vf.blockSize.at(i) = vf.dimension.at(i+1) * vf.blockSize.at(i+1);
            }
            if(context->stack.lut.size()==1)    {   // global array
                vf.isGlobal = 1;
                if (init!=nullptr){
                    file<<"   .globl  "<<getID()<<std::endl;
                    file<<"   .data"<<std::endl;
                    file<<"   .type   "<<getID()<<", @object"<<std::endl;
                    file<<"   .size     "<<getID()<<",  "<<vf.numBytes*vf.length<<std::endl;
                    file<<getID()<<":"<<std::endl;
                    file<<"   .word    ";
                    init->generate(file,"t1",context);
                    file<<std::endl;

                } else if (init ==nullptr){
                    file<<"   .globl  "<<getID()<<std::endl;
                    file<<"   .type   "<<getID()<<", @object"<<std::endl;
                    file<<"   .section        .bss,\"aw\",@nobits"<<std::endl;
                    file<<"   .size     "<<getID()<<",  "<<vf.numBytes*vf.length<<std::endl;
                    file<<getID()<<":"<<std::endl;
                    file<<"   .space    "<<vf.numBytes*vf.length<<std::endl;
                }
            }
            else    {                               // local array
                long space=(vf.numBytes*vf.length)+4;
                if(space % 4)   {
                    space+=4-(space%4);
                }
                context->stack.slider+=space;
                vf.offset=context->stack.slider-4;    // offset of base pointer
                file<<"addiu $t1, $sp, "<<(context->stack.size - vf.offset+4)<<std::endl;   // store 1st element address into base pointer
                file<<"sw $t1, "<<(context->stack.size - vf.offset)<<"($sp)"<<std::endl;
                if(init!=nullptr)   {   // initilise array values if required ($t1 contains address of first index)
                    long initIC = context->indexCounter;
                    context->indexCounter=0;
                    init->generate(file, "$t1", context);
                    context->indexCounter=initIC;
                }
            }
            context->stack.lut.back().insert(std::pair<std::string,varInfo>(getID(),vf));
        }
};

class Array_Init : public Program {  // read value in array
    private:
        std::string value;
        ProgramPtr next = nullptr;
    public:
        Array_Init(std::string *_value, ProgramPtr _next) : value(*_value), next(_next) {
            delete _value;
        }

        ~Array_Init() {
            delete next;
        }

        std::string getValue() const    {
            return value;
        }

        virtual void print(std::ostream &dst) const override    {
            dst<<getValue();
            if(next!=nullptr){
                dst<<",";
                next->print(dst);
            }
        }

        virtual void generate(std::ofstream &file, const char* destReg, Context *context) const override    {
            if(context->vfPointer->isGlobal==1) {           // initilise values for global array
                file<<getValue();
                if(next!=nullptr){
                    file<<",";
                    next->generate(file,destReg,context);
                }
            }
            else    {                                       // initialise values for local array
                file<<"li $t0, "<<getValue()<<std::endl;
                if(context->vfPointer->numBytes==1) {
                    file<<"sb $t0, "<<context->indexCounter<<"($t1)"<<std::endl;
                }
                else    {
                    file<<"sw $t0, "<<context->indexCounter<<"($t1)"<<std::endl;
                }
                if(next!=nullptr)   {
                    context->indexCounter+=context->vfPointer->numBytes;
                    next->generate(file, "$t1", context);
                }
            }
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
        virtual void generate(std::ofstream &file, const char* destReg, Context *context) const override    {
        }
};

class DeclareTypeDef : public Program {
    private:
        std::string id;
        std::string bind_type;
        int ptr=0;
        int isUnsigned=0;
    public:
        DeclareTypeDef(std::string *_bn, std::string *_id, int _ptr, int _uns) : id(*_id), bind_type(*_bn), ptr(_ptr), isUnsigned(_uns) {
            delete _id;
            delete _bn;
        }

        virtual void print(std::ostream &dst) const override    {
            dst<<"typedef "<<bind_type<<" "<<id<<";"<<std::endl;
        }

        virtual void generate(std::ofstream &file, const char* destReg, Context *context) const override    {
            typeInfo tmp;
            tmp.type = bind_type;
            tmp.size = 1;
            tmp.ptr = ptr;
            tmp.isUnsigned = isUnsigned;
            context->typeTable.insert(std::pair<std::string,typeInfo>(id,tmp));
        }
};

class FunctionSizeof : public Program {
    private:
        std::string id;
        DeclareArrayElement *elements=nullptr;
    public:
        FunctionSizeof(std::string *_id, DeclareArrayElement *_elements) : id(*_id), elements(_elements)  {
            delete _id;
        }

        ~FunctionSizeof()   {
            delete elements;
        }

        virtual void print(std::ostream &dst) const override    {
            dst<<"sizeof("<<id;
            if(elements!=nullptr)   {
                elements->print(dst);
            }
            dst<<")";
        }

        virtual void generate(std::ofstream &file, const char* destReg, Context *context) const override    {
            std::unordered_map<std::string,varInfo>::iterator it;   // sizeof variable
            int n=context->stack.lut.size()-1;
            for(int i=n;i>=0;i--)   {
                it=context->stack.lut.at(i).find(id);
                if(it!=context->stack.lut.at(i).end()) {
                    long byteSize = it->second.numBytes * it->second.length;
                    file<<"li "<<destReg<<", "<<byteSize<<std::endl;
                    return;
                }
            }
            std::unordered_map<std::string,typeInfo>::iterator typeIT;  // sizeof type
            std::string bind_name=id;
            long byteSize=1;
            while(bind_name!="")    {
                typeIT=context->typeTable.find(bind_name);
                bind_name=typeIT->second.type;
                byteSize*=typeIT->second.size;
            }
            if(elements!=nullptr)   {
                byteSize*=elements->spaceRequired(context);    // get number of elements for type arrays ie int[10]
            }            
            file<<"li "<<destReg<<", "<<byteSize<<std::endl;
        }
};

#endif