#ifndef COMPILER_AST_FUNCTIONS_HPP
#define COMPILER_AST_FUNCTIONS_HPP

class Function : public Program {   // function call 
    private:
        std::string id;
    public:
        Function(std::string *_id) : id(*_id)   {
            delete _id;
        }
        
        virtual void print(std::ostream &dst) const override    {
            dst<<id<<"();"<<std::endl;
        }
};

class FunctionDef : public Program {    // function definition 
    private:
        std::string type; // return type of function
        std::string id; // name of function
        ProgramPtr action; //the scope of the function
    public:
        FunctionDef(std::string *_type, std::string *_id, ProgramPtr _action) : type(*_type), id(*_id), action(_action)    {
            delete _type;
            delete _id;
        }  

        ~FunctionDef() {
            delete action;
        }

        ProgramPtr getAction() const    {
            return action;
        }

        std::string getID() const    {
            return id;
        }

        std::string getType() const    {
            return type;
        }

        virtual long spaceRequired() const override {   // 5 ints needed: fp, a0, a1, a2 and a3
            return 20;
        }

        void print(std::ostream &dst) const override    {
            dst<<getType()<<" "<<getID()<<"() ";
            getAction()->print(dst);
        }
};

#endif