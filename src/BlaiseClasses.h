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

using ArgsMap = std::map<std::string, const std::type_info *>;

enum class BLAISE_STATUS {
    OK,
    ERROR,
};

class BlaiseVariable {
public:
    std::string name;
    const std::type_info *type;
    std::any value;
    bool is_const = false;
};

class BlaiseFunction {
public:
    std::string name;
    const std::type_info *ret_type;
    ArgsMap args;
    BlaiseParser::BlockContext *block = nullptr;

    std::any Call(const std::vector<std::any>& args) const;
};

class BlaiseBlock {
public:
    std::deque<std::string> ids;
    std::map<std::string_view, const std::type_info *> ids_to_types;
    std::map<std::string_view, std::any> ids_to_values;
    std::deque<BlaiseFunction> functions;
    std::map<std::string_view, BlaiseFunction *> ids_to_functions;
};