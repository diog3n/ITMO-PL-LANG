#pragma once

#include <list>
#include <typeinfo>

#include "antlr4-runtime.h"
#include "antlr/BlaiseParser.h"

// enum class BLAISE_TYPE_ID {
//     DOUBLE,
//     INTEGER,
//     CHAR,
//     STRING,
//     BOOLEAN
// };

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
    std::string name;
    const std::type_info *type;
    std::any value;

    BlaiseVariable(): type(nullptr) {};
    BlaiseVariable(std::any value, const std::type_info& type): value(value), type(&type) {}
    BlaiseVariable& operator=(const BlaiseVariable& var);

    bool operator==(const std::string& str) const;
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
};

using ArgsList = std::list<BlaiseVariable>;

class BlaiseFunction {
public:
    std::string name;
    const std::type_info *ret_type;
    ArgsList args;
    BlaiseParser::StmtContext *block = nullptr;

    std::any Call(const ArgsList& args) const;

    bool operator==(const std::string& str) const;
};

class BlaiseBlock {
public:
    std::deque<std::string> ids;
    std::map<std::string_view, const std::type_info *> ids_to_types;
    std::map<std::string_view, std::any> ids_to_values;
    std::deque<BlaiseFunction> functions;
    std::map<std::string_view, BlaiseFunction *> ids_to_functions;
};