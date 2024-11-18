#pragma once

#include <list>

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

class BlaiseBlock {
public:
    std::deque<std::string> ids;
    std::map<const std::string *, std::type_info> ids_to_types;
    std::map<const std::string *, std::any> ids_to_values;
};

class BlaiseFunction {
public:
    std::string name;
    std::list<std::string> args;
    BlaiseParser::BlockContext *block;
};