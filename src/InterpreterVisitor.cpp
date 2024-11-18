#include <algorithm>
#include <any>
#include <cctype>
#include <sstream>
#include <stdexcept>

#include "InterpreterVisitor.h"
#include "BlaiseClasses.h"
#include "antlr/BlaiseParser.h"
#include "tree/TerminalNode.h"

#define DEBUG   1

#define DEBUG__BEGIN \
    if (DEBUG) std::cout << "In function " << __func__ << std::endl

#define DEBUG__PRINT_VAL(__val) \
    if (DEBUG) std::cout << #__val << " = " << __val << std::endl

#define ANY_RECAST(__type_from, __type_to, __val) \
    static_cast<__type_to>(std::any_cast<__type_from>(__val))

#define CAST(__type, __val) std::any_cast<__type>(__val)

#define OPERATION_BLOCK(__operand1, __operand2, __operation, __type)                 \
    if (__operand1.type() == typeid(__type))                                         \
        return CAST(__type, __operand1) __operation CAST(__type, __operand2)

#define OPERATION_BLOCK_ALL_TYPES(__operand1, __operand2, __operation)               \
    OPERATION_BLOCK(__operand1, __operand2, __operation, double);                    \
    OPERATION_BLOCK(__operand1, __operand2, __operation, int);                       \
    OPERATION_BLOCK(__operand1, __operand2, __operation, bool);                      \
    OPERATION_BLOCK(__operand1, __operand2, __operation, char);                      \
    OPERATION_BLOCK(__operand1, __operand2, __operation, std::string)

#define ANY_IS(__val, __type) \
    (__val.type() == typeid(__type))

// MACRO MAGIC FOR std::any
#define $$any_case_block_begin(__any_val) \
    { const std::any& __$$any_val_internal$$ = __any_val

#define $$any_case_start(__type) \
    if (ANY_IS(__$$any_val_internal$$, __type)) {

#define $$any_case(__type) } else $$any_case_start(__type)

#define $$any_case_default \
    } else {

#define $$any_case_block_end }}

std::string InterpreterVisitor::StringToUpper(std::string str) {
    std::transform(str.begin(), str.end(), str.begin(), toupper);
    return str;
}

std::string InterpreterVisitor::AnyValueToString(const std::any& value) {
    std::ostringstream out;

    $$any_case_block_begin(value);

        $$any_case_start(double)
            out << std::any_cast<double>(value);

        $$any_case(std::string)
            out << std::any_cast<std::string>(value);

        $$any_case(int)
            out << std::any_cast<int>(value);

        $$any_case(bool)
            out << (std::any_cast<bool>(value) ? "true" : "false");

        $$any_case(char)
            out << std::any_cast<char>(value);

        $$any_case_default
            out << "Something else";

    $$any_case_block_end;

    return out.str();
}


// std::string InterpreterVisitor::TypeIdToString(BLAISE_TYPE_ID type) {
//     switch(type) {
//         case BLAISE_TYPE_ID::DOUBLE:
//             return "double";
//             break;
//         case BLAISE_TYPE_ID::INTEGER:
//             return "int";
//             break;
//         case BLAISE_TYPE_ID::CHAR:
//             return "char";
//             break;
//         case BLAISE_TYPE_ID::STRING:
//             return "string";
//             break;
//         case BLAISE_TYPE_ID::BOOLEAN:
//             return "boolean";
//             break;
//         }
// }

BLAISE_TYPE_ID InterpreterVisitor::StringToTypeId(const std::string& str) {
    if (str == "double")
        return BLAISE_TYPE_ID::DOUBLE;
    else if (str == "int")
        return BLAISE_TYPE_ID::INTEGER;
    else if (str == "char")
        return BLAISE_TYPE_ID::CHAR;
    else if (str == "string")
        return BLAISE_TYPE_ID::STRING;
    else if (str == "boolean")
        return BLAISE_TYPE_ID::BOOLEAN;

    throw std::invalid_argument("Unknown type: " + str);
}

BLAISE_TYPE_ID InterpreterVisitor::GetType(const std::any& value) {
    if (value.type() == typeid(double)) {
        return BLAISE_TYPE_ID::DOUBLE;
    } else if (value.type() == typeid(int)) {
        return BLAISE_TYPE_ID::INTEGER;
    } else if (value.type() == typeid(char)) {
        return BLAISE_TYPE_ID::CHAR;
    } else if (value.type() == typeid(std::string)) {
        return BLAISE_TYPE_ID::STRING;
    } else if (value.type() == typeid(bool)) {
        return BLAISE_TYPE_ID::BOOLEAN;
    }

    throw std::invalid_argument("std::any has no value.");
}

bool InterpreterVisitor::HasType(const std::any& value, BLAISE_TYPE_ID type) {
    try {
        switch(type) {
            case BLAISE_TYPE_ID::DOUBLE:
                std::any_cast<double>(value);
                break;
            case BLAISE_TYPE_ID::INTEGER:
                std::any_cast<int>(value);
                break;
            case BLAISE_TYPE_ID::CHAR:
                std::any_cast<char>(value);
                break;
            case BLAISE_TYPE_ID::STRING:
                std::any_cast<std::string>(value);
                break;
            case BLAISE_TYPE_ID::BOOLEAN:
                std::any_cast<bool>(value);
                break;
        }
    } catch (const std::bad_any_cast&) {
        return false;
    }

    return true;
}

bool InterpreterVisitor::IsNumber(const std::any& value) {
    return (GetType(value) == BLAISE_TYPE_ID::DOUBLE) ||
           (GetType(value) == BLAISE_TYPE_ID::INTEGER);
}

std::any InterpreterVisitor::visitProgram(BlaiseParser::ProgramContext *context) {
    std::any value = visitChildren(context);

    // Stack contents
    if (DEBUG) {
        for (const auto& pair : gl_block.ids_to_types) {
            BLAISE_TYPE_ID type = pair.second;
            const std::string *name = pair.first;
            std::cout << TypeIdToString(type) << " " << *name << " = "  << AnyValueToString(gl_block.ids_to_values[name]) << std::endl;
        }
    }

    return value;
}

std::any InterpreterVisitor::visitStmt(BlaiseParser::StmtContext *context) {
    return visitChildren(context);
}

std::any InterpreterVisitor::visitFunctionDeclaration(BlaiseParser::FunctionDeclarationContext *context) {
    return 0;
}

std::any InterpreterVisitor::visitFunctionDefinition(BlaiseParser::FunctionDefinitionContext *context) {
    return 0;
}

std::any InterpreterVisitor::visitFunctionCall(BlaiseParser::FunctionCallContext *context) {
    return 0;
}

std::any InterpreterVisitor::visitParamListComma(BlaiseParser::ParamListCommaContext *context) {
    return 0;
}

std::any InterpreterVisitor::visitParamListEnd(BlaiseParser::ParamListEndContext *context) {
    return 0;
}

std::any InterpreterVisitor::visitArgListComma(BlaiseParser::ArgListCommaContext *context) {
    return 0;
}

std::any InterpreterVisitor::visitArgListEnd(BlaiseParser::ArgListEndContext *context) {
    return 0;
}

std::any InterpreterVisitor::visitCodeBlock(BlaiseParser::CodeBlockContext *context) {
    return visitChildren(context);
}

std::any InterpreterVisitor::visitVariableDefinition(BlaiseParser::VariableDefinitionContext *context) {
    DEBUG__BEGIN;

    std::string type_id = context->IDENTIFIER(0)->toString();
    std::string var_id  = context->IDENTIFIER(1)->toString();
    std::cout << "encountered " << type_id << " " << var_id << std::endl;

    std::any value = visit(context->initialization());

    gl_block.ids.emplace_back(std::move(var_id));
    gl_block.ids_to_types[&gl_block.ids.back()] = StringToTypeId(type_id);

    BLAISE_TYPE_ID var_type = gl_block.ids_to_types.at(&gl_block.ids.back());

    if (value.has_value() && GetType(value) != var_type) {

        // Re-casting values from int to double and from double to int if needed
        if (GetType(value) == BLAISE_TYPE_ID::DOUBLE && var_type == BLAISE_TYPE_ID::INTEGER) {
            value = ANY_RECAST(double, int, value);
        } else if (GetType(value) == BLAISE_TYPE_ID::INTEGER && var_type == BLAISE_TYPE_ID::DOUBLE) {
            value = ANY_RECAST(int, double, value);
        } else {
            throw std::invalid_argument("Invalid value for variable of type " + type_id);
        }
    }

    gl_block.ids_to_values[&gl_block.ids.back()] = value;

    return value;
}

std::any InterpreterVisitor::visitVariableInitialization(BlaiseParser::VariableInitializationContext *context) {
    DEBUG__BEGIN;
    return visit(context->expr());
}

std::any InterpreterVisitor::visitAssignStmt(BlaiseParser::AssignStmtContext *context) {
    DEBUG__BEGIN;
    auto id_iter = std::find(gl_block.ids.begin(), gl_block.ids.end(), context->IDENTIFIER()->toString());
    if (id_iter == gl_block.ids.end()) throw std::invalid_argument("Variable has not been initialized");

    const std::string& id = *id_iter;

    std::any value = visit(context->expr());

    if (gl_block.ids_to_types.at(&id) == BLAISE_TYPE_ID::DOUBLE) {
        if (GetType(value) == BLAISE_TYPE_ID::INTEGER) value = ANY_RECAST(int, double, value);
    } else if (gl_block.ids_to_types.at(&id) == BLAISE_TYPE_ID::INTEGER) {
        if (GetType(value) == BLAISE_TYPE_ID::DOUBLE) value = ANY_RECAST(double, int, value);
    }

    if (GetType(value) != gl_block.ids_to_types.at(&id))
        throw std::invalid_argument("Invalid value for type " + TypeIdToString(gl_block.ids_to_types.at(&id)));

    gl_block.ids_to_values[&id] = value;

    return BLAISE_STATUS::OK;
}

std::any InterpreterVisitor::visitWritelnStmt(BlaiseParser::WritelnStmtContext *context) {
    DEBUG__BEGIN;
    std::any value = context->expr();
    std::cout << "1" << std::endl;
    DEBUG__PRINT_VAL(AnyValueToString(value));

    if (typeid(value) == typeid(bool))

    switch (GetType(value)) {
        case BLAISE_TYPE_ID::BOOLEAN:
            std::cout << "writeln: " << (std::any_cast<bool>(value) ? "true" : "false") << std::endl;
            break;
        case BLAISE_TYPE_ID::CHAR:
            std::cout << "writeln: " << std::any_cast<char>(value) << std::endl;
            break;
        case BLAISE_TYPE_ID::DOUBLE:
            std::cout << "writeln: " << std::any_cast<double>(value) << std::endl;
            break;
        case BLAISE_TYPE_ID::INTEGER:
            std::cout << "writeln: " << std::any_cast<int>(value) << std::endl;
            break;
        case BLAISE_TYPE_ID::STRING:
            std::cout << "writeln: " << std::any_cast<std::string>(value) << std::endl;
            break;
    }

    return BLAISE_STATUS::OK;
}

std::any InterpreterVisitor::visitReturnStmt(BlaiseParser::ReturnStmtContext *context) {
    return visitChildren(context);
}

std::any InterpreterVisitor::visitIfStmtBlock(BlaiseParser::IfStmtBlockContext *context) {
    return visitChildren(context);
}

std::any InterpreterVisitor::visitIfStmtExpr(BlaiseParser::IfStmtExprContext *context) {
    return visitChildren(context);
}

std::any InterpreterVisitor::visitElseStmtBlock(BlaiseParser::ElseStmtBlockContext *context) {

    return visitChildren(context);
}

std::any InterpreterVisitor::visitElseStmtExpr(BlaiseParser::ElseStmtExprContext *context) {
    return visitChildren(context);
}

std::any InterpreterVisitor::visitElseIfStmtBlock(BlaiseParser::ElseIfStmtBlockContext *context) {
    return visitChildren(context);
}

std::any InterpreterVisitor::visitElseIfStmtExpr(BlaiseParser::ElseIfStmtExprContext *context) {
    return visitChildren(context);
}

std::any InterpreterVisitor::visitLoopStmt(BlaiseParser::LoopStmtContext *context) {
    return visitChildren(context);
}

std::any InterpreterVisitor::visitExprOperation(BlaiseParser::ExprOperationContext *context) {
    DEBUG__BEGIN;

    std::any operand = visit(context->operand());
    BLAISE_OP_ID operator_ = std::any_cast<BLAISE_OP_ID>(visit(context->operator_()));
    std::any expr = visit(context->expr());

    if (GetType(operand) != GetType(expr)) {
        if (!(IsNumber(operand) && IsNumber(expr)))
            throw std::invalid_argument("Cannot perform an operation between different types");

        if (GetType(operand) != BLAISE_TYPE_ID::DOUBLE) {
            operand = ANY_RECAST(int, double, operand);
        }

        if (GetType(expr) != BLAISE_TYPE_ID::DOUBLE) {
            expr = ANY_RECAST(int, double, expr);
        }
    }

    switch (operator_) {
        case BLAISE_OP_ID::PLUS:
            std::cout << "PLUS" << std::endl;
            OPERATION_BLOCK(operand, expr, +, int);
            OPERATION_BLOCK(operand, expr, +, bool);
            OPERATION_BLOCK(operand, expr, +, double);
            OPERATION_BLOCK(operand, expr, +, std::string);
            break;
        case BLAISE_OP_ID::MINUS:
            std::cout << "MINUS" << std::endl;
            OPERATION_BLOCK(operand, expr, -, int);
            OPERATION_BLOCK(operand, expr, -, double);
            break;
        case BLAISE_OP_ID::MUL:
            std::cout << "MUL" << std::endl;
            OPERATION_BLOCK(operand, expr, *, int);
            OPERATION_BLOCK(operand, expr, *, double);
            break;
        case BLAISE_OP_ID::DIV:
            std::cout << "DIV" << std::endl;
            OPERATION_BLOCK(operand, expr, /, int);
            OPERATION_BLOCK(operand, expr, /, double);
            break;
        case BLAISE_OP_ID::EQUAL:
            std::cout << "EQUAL" << std::endl;
            OPERATION_BLOCK_ALL_TYPES(operand, expr, ==);
            break;
        case BLAISE_OP_ID::NEQUAL:
            std::cout << "NEQUAL" << std::endl;
            OPERATION_BLOCK_ALL_TYPES(operand, expr, !=);
            break;
        case BLAISE_OP_ID::LESS:
            std::cout << "LESS" << std::endl;
            OPERATION_BLOCK_ALL_TYPES(operand, expr, <);
            break;
        case BLAISE_OP_ID::LEQUAL:
            std::cout << "LEQUAL" << std::endl;
            OPERATION_BLOCK_ALL_TYPES(operand, expr, <=);
            break;
        case BLAISE_OP_ID::GREATER:
            std::cout << "GREATER" << std::endl;
            OPERATION_BLOCK_ALL_TYPES(operand, expr, >);
            break;
        case BLAISE_OP_ID::GEQUAL:
            std::cout << "GEQUAL" << std::endl;
            OPERATION_BLOCK_ALL_TYPES(operand, expr, >=);
            break;
    }

    return BLAISE_STATUS::ERROR;
}

std::any InterpreterVisitor::visitExprUnaryMinusOperation(BlaiseParser::ExprUnaryMinusOperationContext *context) {
    DEBUG__BEGIN;
    std::any operand = visit(context->operand());
    return visitChildren(context);
}

std::any InterpreterVisitor::visitExprUnaryPlusOperation(BlaiseParser::ExprUnaryPlusOperationContext *context) {
    DEBUG__BEGIN;
    std::any operand = visit(context->operand());
    return visitChildren(context);
}

std::any InterpreterVisitor::visitExprOperand(BlaiseParser::ExprOperandContext *context) {
    DEBUG__BEGIN;
    return visitChildren(context);
}

std::any InterpreterVisitor::visitOperandId(BlaiseParser::OperandIdContext *context) {
    auto id_iter = std::find(gl_block.ids.begin(), gl_block.ids.end(), context->IDENTIFIER()->toString());

    // if present in the global scope
    if (id_iter == gl_block.ids.end())
        throw std::invalid_argument("Variable " + context->IDENTIFIER()->toString()
                                                + " has not been defined!");

    const std::string& id = *id_iter;

    // If it has not been initialized
    if (!gl_block.ids_to_values.at(&id).has_value())
        throw std::invalid_argument("Variable " + id + " has not been initialized!");

    return gl_block.ids_to_values[&id];
}

std::any InterpreterVisitor::visitOperandInt(BlaiseParser::OperandIntContext *context) {
    return std::stoi(context->INT()->toString());
}

std::any InterpreterVisitor::visitOperandDouble(BlaiseParser::OperandDoubleContext *context) {
    return std::stod(context->DOUBLE()->toString());
}

std::any InterpreterVisitor::visitOperandChar(BlaiseParser::OperandCharContext *context) {
    std::string value = context->CHAR()->toString();
    return value.at(1);
}

std::any InterpreterVisitor::visitOperandString(BlaiseParser::OperandStringContext *context) {
    std::string full_str = context->STRING()->toString();
    return std::string(full_str.begin() + 1, full_str.end() - 1);
}

std::any InterpreterVisitor::visitOperandFunctionCall(BlaiseParser::OperandFunctionCallContext *context) {
    return 0; // visitChildren(context);
}

std::any InterpreterVisitor::visitOperandExpr(BlaiseParser::OperandExprContext *context) {
    return visit(context->expr());
}

std::any InterpreterVisitor::visitOperandBoolean(BlaiseParser::OperandBooleanContext *context) {
    std::string str = context->BOOLEAN()->toString();
    if (str == "true") return true;
    else if (str == "false") return false;

    throw std::invalid_argument(str + " is not a valid boolean value.");
}

std::any InterpreterVisitor::visitOperatorPlus(BlaiseParser::OperatorPlusContext *context) {
    return BLAISE_OP_ID::PLUS;
}

std::any InterpreterVisitor::visitOperatorMinus(BlaiseParser::OperatorMinusContext *context) {
    return BLAISE_OP_ID::MINUS;
}

std::any InterpreterVisitor::visitOperatorAster(BlaiseParser::OperatorAsterContext *context) {
    return BLAISE_OP_ID::MUL;
}

std::any InterpreterVisitor::visitOperatorSlash(BlaiseParser::OperatorSlashContext *context) {
    return BLAISE_OP_ID::DIV;
}

std::any InterpreterVisitor::visitOperatorEqual(BlaiseParser::OperatorEqualContext *context) {
    return BLAISE_OP_ID::EQUAL;
}

std::any InterpreterVisitor::visitOperatorNEqual(BlaiseParser::OperatorNEqualContext *context) {
    return BLAISE_OP_ID::NEQUAL;
}

std::any InterpreterVisitor::visitOperatorLess(BlaiseParser::OperatorLessContext *context) {
    return BLAISE_OP_ID::LESS;
}

std::any InterpreterVisitor::visitOperatorLEqual(BlaiseParser::OperatorLEqualContext *context) {
    return BLAISE_OP_ID::LEQUAL;
}

std::any InterpreterVisitor::visitOperatorGreater(BlaiseParser::OperatorGreaterContext *context) {
    return BLAISE_OP_ID::GREATER;
}

std::any InterpreterVisitor::visitOperatorGEqual(BlaiseParser::OperatorGEqualContext *context) {
    return BLAISE_OP_ID::GEQUAL;
}