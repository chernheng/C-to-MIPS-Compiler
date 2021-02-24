#ifndef COMPILER_AST_PRIMITIVES_HPP
#define COMPILER_AST_PRIMITIVES_HPP

#include "src/include/ast.hpp"

class Variable : public Program {
    private:
        std::string id;
    public:
        Variable(std::string _id) : id(_id) {}

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
        std::string index;
    public:
        Array(std::string _id, std::string _index) : id(_id), index(_index) {}

        std::string getID() const {
            return id;
        }

        std::string getIndex() const    {
            return index;
        }

        virtual void print(std::ostream &dst) const override    {
            dst<<id<<"["<<index<<"]";
        }
};

class Number : public Program {
    private:
        std::string value;
    public:
        Number(std::string _value) : value(_value)  {}

        std::string getValue() const    {
            return value;
        }

        virtual void print(std::ostream &dst) const override    {
            dst<<value;
        }
};

#endif