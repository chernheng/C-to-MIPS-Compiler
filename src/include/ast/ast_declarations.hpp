#ifndef COMPILER_AST_DECLARATIONS_HPP
#define COMPILER_AST_DECLARATIONS_HPP

class DeclareVariable : public Program {
    private:
        std::string type;
        std::string id;
        ProgramPtr init=nullptr; //int x = 5;
    public:
        DeclareVariable(std::string *_type, std::string *_id, ProgramPtr _init) : type(*_type), id(*_id), init(_init)  {
            delete _type;
            delete _id;
        }

        DeclareVariable(std::string *_type, std::string *_id) : type(*_type),id(*_id)   {
            delete _type;
            delete _id;
        }

        ~DeclareVariable() {
            delete init;
        }

        virtual void print(std::ostream &dst) const override    {
            dst<<type<<" "<<id;
            if(init!=nullptr)    {
                dst<<"=";
                init->print(dst);
            }
            dst<<";";
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