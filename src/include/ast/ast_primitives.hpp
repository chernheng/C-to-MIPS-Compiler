#ifndef COMPILER_AST_PRIMITIVES_HPP
#define COMPILER_AST_PRIMITIVES_HPP

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

        virtual void print(std::ostream &dst) const override    {
            dst<<id;
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
};

#endif