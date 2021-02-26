#ifndef COMPILER_AST_FUNCTIONS_HPP
#define COMPILER_AST_FUNCTIONS_HPP

class Function : public Program {
    private:
        std::string type; // return type of function
        std::string id; // name of function
        ProgramPtr action; //the scope of the function
    public:
        Function(std::string *_type, std::string *_id, ProgramPtr _action) : type(*_type), id(*_id), action(_action)    {}  
    public:
        ~Function() {
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

        void print(std::ostream &dst) const override    {
            dst<<getType()<<" "<<getID()<<"() ";
            dst<<"{"<<std::endl;
            getAction()->print(dst);
            dst<<"}"<<std::endl;
        }
};

#endif