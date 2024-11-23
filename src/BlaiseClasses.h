#pragma once

#include <list>
#include <typeinfo>

#include "antlr/BlaiseParser.h"

enum class BLAISE_OP_ID {
    PLUS,
    MINUS,
    MUL,
    DIV,
    EQUAL,
    NEQUAL,
    LESS,
    LEQUAL,
    GREATER,
    GEQUAL,
};


enum class BLAISE_STATUS {
    OK,
    ERROR,
};

class BlaiseVariable {
public:

    BlaiseVariable(): type_(nullptr) {};

    BlaiseVariable(const std::string& name,
                   const std::type_info& type)
                : name_(name), type_(&type) {}

    template<typename T>
    BlaiseVariable(const std::type_info& type,
                   const T& value)
                : value_(value), type_(&type) {}

    template<typename T>
    BlaiseVariable(const std::string& name,
                   const std::type_info& type,
                   const T& value)
                : name_(name), type_(&type), value_(value) {}

    const std::string& Name() const;
    const std::type_info& Type() const;

    template<typename T>
    T Value() const;

    template<typename T>
    bool Is() const;

    const std::any& Value() const;

    void SetName(const std::string& name);
    void SetType(const std::type_info& type);
    void SetValue(const std::any& value);

    std::string ToString() const;

    BlaiseVariable& operator=(const BlaiseVariable& var) = default;

    bool operator==(const std::string& str) const;
    BlaiseVariable& Assign(const BlaiseVariable& var);

    BlaiseVariable operator+(const BlaiseVariable& var) const;
    BlaiseVariable operator-(const BlaiseVariable& var) const;
    BlaiseVariable operator*(const BlaiseVariable& var) const;
    BlaiseVariable operator/(const BlaiseVariable& var) const;

    BlaiseVariable operator==(const BlaiseVariable& var) const;
    BlaiseVariable operator!=(const BlaiseVariable& var) const;
    BlaiseVariable operator<(const BlaiseVariable& var) const;
    BlaiseVariable operator<=(const BlaiseVariable& var) const;
    BlaiseVariable operator>(const BlaiseVariable& var) const;
    BlaiseVariable operator>=(const BlaiseVariable& var) const;

    BlaiseVariable operator+() const;
    BlaiseVariable operator-() const;
private:
    static std::string AnyValueToString(const std::any& any);
    static std::pair<BlaiseVariable, BlaiseVariable> CastToOneType(const BlaiseVariable& lhs,
                                                                   const BlaiseVariable& rhs);
    std::string name_;
    const std::type_info *type_;
    std::any value_;
};

template<typename T>
bool BlaiseVariable::Is() const {
    return typeid(T) == *type_;
}


template<typename T>
T BlaiseVariable::Value() const {
    return std::any_cast<T>(value_);
}

using ArgsList = std::list<BlaiseVariable>;

class BlaiseFunction {
public:
    BlaiseFunction(const std::string& name,
                   const std::type_info& ret_type,
                   const ArgsList& args)
                : name_(name), ret_type_(&ret_type), args_(args) {}

    BlaiseFunction(const std::string& name,
                   const std::type_info& ret_type,
                   const ArgsList& args,
                   BlaiseParser::StmtContext *block)
                : name_(name), ret_type_(&ret_type), args_(args), block_(block) {}

    const std::string& Name() const;

    const std::type_info& RetType() const;

    const ArgsList& Args() const;

    BlaiseParser::StmtContext *Block() const;

    bool IsDefined() const;

    void SetBlock(BlaiseParser::StmtContext *block);

    bool operator==(const std::string& str) const;
private:
    std::string name_;

    const std::type_info *ret_type_;

    ArgsList args_;

    BlaiseParser::StmtContext *block_ = nullptr;
};

class BlaiseBlock {
public:
    std::deque<BlaiseVariable> variables;
    std::deque<BlaiseFunction> functions;
};