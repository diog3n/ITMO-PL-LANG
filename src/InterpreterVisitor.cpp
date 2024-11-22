#include <algorithm>
#include <any>
#include <cctype>
#include <sstream>
#include <stdexcept>
#include <string>
#include <typeinfo>
#include <utility>

#include "InterpreterVisitor.h"
#include "BlaiseClasses.h"
#include "antlr/BlaiseParser.h"

#define DEBUG   1

#define DEBUG__BEGIN \
    if (DEBUG >= 2) std::cout << "In function " << __func__ << std::endl

#define DEBUG__PRINT_VAL(__level, __val) \
    if (DEBUG >= __level) std::cout << #__val << " = " << __val << std::endl

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
#define any_case_block_begin$$(__any_val) \
    { const std::any& __$$any_val_internal$$ = __any_val

#define any_case_start$$(__type) \
    if (ANY_IS(__$$any_val_internal$$, __type)) {

#define any_case$$(__type) } else any_case_start$$(__type)

#define any_case_default$$ \
    } else {

#define any_case_block_end$$ }}

#define SHOULD_NOT_BE_HERE std::string(__func__) + ": You should not be here"

void recast_to_double(std::any& lhs, std::any& rhs) {
    if (ANY_IS(lhs, int)) lhs = ANY_RECAST(int, double, lhs);
    if (ANY_IS(rhs, int)) rhs = ANY_RECAST(int, double, rhs);
}

InterpreterVisitor::InterpreterVisitor() {
    block_stack.emplace_back();
    gl_block = &block_stack.back();
}

std::string InterpreterVisitor::StringToUpper(std::string str) {
    std::transform(str.begin(), str.end(), str.begin(), toupper);
    return str;
}

std::pair<std::string *, BlaiseBlock *>
InterpreterVisitor::FindIdAndBlock(const std::string& str) {
    BlaiseBlock *block = nullptr;
    std::string *id = nullptr;


    for (auto riter = block_stack.rbegin(); riter != block_stack.rend(); riter++) {
        for (auto id_riter = riter->ids.rbegin(); id_riter != riter->ids.rend(); id_riter++) {
            if (*id_riter == str) {
                id = &(*id_riter);
                block = &(*riter);

                return std::make_pair<std::string *, BlaiseBlock *>(std::move(id), std::move(block));
            }
        }
    }

    throw std::invalid_argument("Variable " + str + " has not been defined!");
}

std::string InterpreterVisitor::AnyValueToString(const std::any& value) {
    std::ostringstream out;

    any_case_block_begin$$(value);

        any_case_start$$(double)
            out << std::any_cast<double>(value);

        any_case$$(std::string)
            out << std::any_cast<std::string>(value);

        any_case$$(int)
            out << std::any_cast<int>(value);

        any_case$$(bool)
            out << (std::any_cast<bool>(value) ? "true" : "false");

        any_case$$(char)
            out << std::any_cast<char>(value);

        any_case_default$$
            out << "Something else";

    any_case_block_end$$;

    return out.str();
}

BlaiseFunction& InterpreterVisitor::AddFunction(const std::string& name, const std::type_info& type,
                                                BlaiseParser::Param_listContext *paramlist) {
    BlaiseFunction& func = block_stack.back().functions.emplace_back();

    func.name = name;
    func.ret_type = &type;

    if (paramlist)
        func.args = std::move(std::any_cast<ArgsList>(visit(paramlist)));

    block_stack.back().ids_to_functions[func.name] = &func;

    return func;
}

const std::type_info& InterpreterVisitor::StringToTypeId(const std::string& str) {
    if (str == "double") {
        return typeid(double);
    } else if (str == "int") {
        return typeid(int);
    } else if (str == "string") {
        return typeid(std::string);
    } else if (str == "boolean") {
        return typeid(bool);
    } else if (str == "char") {
        return typeid(char);
    }

    throw std::invalid_argument("Invalid type identifier: " + str);
}

bool InterpreterVisitor::IsNumber(const std::any& value) {
    return (ANY_IS(value, double) || ANY_IS(value, int));
}

std::any InterpreterVisitor::visitProgram(BlaiseParser::ProgramContext *context) {
    std::any value = visitChildren(context);

    // Stack contents
    if (DEBUG) {
        for (const auto& pair : gl_block->ids_to_types) {
            const std::type_info& type = *pair.second;
            const std::string_view name = pair.first;
            std::cout << type.name() << " " << name << " = "
                      << AnyValueToString(gl_block->ids_to_values[name])
                      << std::endl;
        }

        for (const auto& func : gl_block->functions) {
            std::cout << func.name << ": " << func.ret_type->name() << std::endl;
        }
    }

    return value;
}

std::any InterpreterVisitor::visitStmt(BlaiseParser::StmtContext *context) {
    return visitChildren(context);
}

std::any InterpreterVisitor::visitFunctionDeclaration(BlaiseParser::FunctionDeclarationContext *context) {
    const std::string& id = context->IDENTIFIER(0)->toString();
    const std::type_info& type = StringToTypeId(context->IDENTIFIER(1)->toString());

    AddFunction(id, type, context->param_list());
    return 0;
}

std::any InterpreterVisitor::visitFunctionDefinition(BlaiseParser::FunctionDefinitionContext *context) {
    const std::string& id = context->IDENTIFIER(0)->toString();
    const std::type_info& type = StringToTypeId(context->IDENTIFIER(1)->toString());

    auto iter = std::find_if(block_stack.back().functions.begin(), block_stack.back().functions.end(), [&id](const BlaiseFunction& func) { return func.name == id; });
    if (iter == block_stack.back().functions.end()) {
        AddFunction(id, type, context->param_list());
    }

    if (!block_stack.back().ids_to_functions[id]->block) {
        block_stack.back().ids_to_functions[id]->block = context->stmt();
    } else {
        throw std::invalid_argument("Function redefinition is not allowed.");
    }

    return 0;
}

std::any InterpreterVisitor::visitFunctionCall(BlaiseParser::FunctionCallContext *context) {
    return 0;
}

std::any InterpreterVisitor::visitParamListComma(BlaiseParser::ParamListCommaContext *context) {
    const std::type_info& type = StringToTypeId(context->IDENTIFIER(0)->toString());
    const std::string& id = context->IDENTIFIER(1)->toString();
    auto args = std::any_cast<ArgsList>(visit(context->param_list()));
    auto iter = std::find(args.begin(), args.end(), id);

    if ( iter != args.end()) {
        throw std::invalid_argument("Identifier " + id + " already is in the list.");
    }


    BlaiseVariable& var = args.emplace_front();

    var.name = id;
    var.type = &type;

    return args;
}

std::any InterpreterVisitor::visitParamListEnd(BlaiseParser::ParamListEndContext *context) {
    const std::string& id = context->IDENTIFIER(1)->toString();
    const std::type_info& type = StringToTypeId(context->IDENTIFIER(0)->toString());

    ArgsList args;
    BlaiseVariable& var = args.emplace_front();

    var.name = id;
    var.type = &type;

    return args;
}

std::any InterpreterVisitor::visitArgListComma(BlaiseParser::ArgListCommaContext *context) {
    return 0;
}

std::any InterpreterVisitor::visitArgListEnd(BlaiseParser::ArgListEndContext *context) {
    return 0;
}

std::any InterpreterVisitor::visitCodeBlock(BlaiseParser::CodeBlockContext *context) {
    block_stack.emplace_back();

    std::any value = visitChildren(context);

    block_stack.pop_back();
    return value;
}

std::any InterpreterVisitor::visitVariableDefinition(BlaiseParser::VariableDefinitionContext *context) {
    DEBUG__BEGIN;

    std::string type_id = context->IDENTIFIER(0)->toString();
    std::string var_id  = context->IDENTIFIER(1)->toString();

    std::any value = visit(context->initialization());

    // If block stack is not empty, then create a variable in the narrowest scope
    BlaiseBlock *blockptr = &block_stack.back();

    blockptr->ids.emplace_back(std::move(var_id));
    blockptr->ids_to_types[blockptr->ids.back()] = &StringToTypeId(type_id);

    const std::type_info& var_type = *blockptr->ids_to_types.at(blockptr->ids.back());

    if (value.has_value() && value.type() != var_type) {

        // Re-casting values from int to double and from double to int if needed
        if (ANY_IS(value, double) && var_type == typeid(int)) {
            value = ANY_RECAST(double, int, value);
        } else if (ANY_IS(value, int) && var_type == typeid(double)) {
            value = ANY_RECAST(int, double, value);
        } else {
            throw std::invalid_argument("Invalid value for variable of type " + type_id);
        }
    }

    blockptr->ids_to_values[blockptr->ids.back()] = value;

    return value;
}

std::any InterpreterVisitor::visitVariableInitialization(BlaiseParser::VariableInitializationContext *context) {
    DEBUG__BEGIN;
    return visit(context->expr());
}

std::any InterpreterVisitor::visitAssignStmt(BlaiseParser::AssignStmtContext *context) {
    DEBUG__BEGIN;
    auto [idptr, blockptr] = FindIdAndBlock(context->IDENTIFIER()->toString());

    std::any value = visit(context->expr());

    if (*blockptr->ids_to_types.at(*idptr) == typeid(double)) {
        if (ANY_IS(value, int))
            value = ANY_RECAST(int, double, value);
    } else if (*blockptr->ids_to_types.at(*idptr) == typeid(int)) {
        if (ANY_IS(value, double))
            value = ANY_RECAST(double, int, value);
    }

    if (value.type() != *blockptr->ids_to_types.at(*idptr))
        throw std::invalid_argument("Invalid value for type " + std::string(blockptr->ids_to_types.at(*idptr)->name()));

    blockptr->ids_to_values[*idptr] = value;

    return BLAISE_STATUS::OK;
}

std::any InterpreterVisitor::visitWritelnStmt(BlaiseParser::WritelnStmtContext *context) {
    DEBUG__BEGIN;
    std::string value = AnyValueToString(visit(context->expr()));

    std::cout << (DEBUG ? "writeln: " : "") << value << std::endl;

    return value;

    throw std::invalid_argument(SHOULD_NOT_BE_HERE);
}

std::any InterpreterVisitor::visitReturnStmt(BlaiseParser::ReturnStmtContext *context) {
    return visitChildren(context);
}

std::any InterpreterVisitor::visitIfStmtBlock(BlaiseParser::IfStmtBlockContext *context) {
    std::any condition_expr = visit(context->expr());
    bool condition = false;

    if (ANY_IS(condition_expr, bool))
        condition = std::any_cast<bool>(condition_expr);
    else
        throw std::invalid_argument("If statement expression must be boolean!");

    if (condition) {
        block_stack.emplace_back();
        std::any value = visit(context->stmt());
        block_stack.pop_back();
        return value;
    }

    if (context->else_stmt())
        return visit(context->else_stmt());

    return false;
}

std::any InterpreterVisitor::visitElseStmtBlock(BlaiseParser::ElseStmtBlockContext *context) {
    block_stack.emplace_back();
    std::any value = visitChildren(context);
    block_stack.pop_back();
    return value;
}

std::any InterpreterVisitor::visitElseIfStmt(BlaiseParser::ElseIfStmtContext *context) {
    return visitChildren(context);
}

std::any InterpreterVisitor::visitLoopStmt(BlaiseParser::LoopStmtContext *context) {
    std::any condition_expr;
    bool condition;

    while (true) {
        condition_expr = visit(context->expr());

        if (ANY_IS(condition_expr, bool))
            condition = std::any_cast<bool>(condition_expr);
        else
            throw std::invalid_argument("Loop if statement expression must be boolean!");

        if (!condition) break;

        if (context->stmt())
            visit(context->stmt());
    }

    return condition;
}

std::any InterpreterVisitor::visitExprOperation(BlaiseParser::ExprOperationContext *context) {
    DEBUG__BEGIN;

    std::any operand = visit(context->operand());
    BLAISE_OP_ID operator_ = std::any_cast<BLAISE_OP_ID>(visit(context->operator_()));
    std::any expr = visit(context->expr());

    if (operand.type() != expr.type()) {

        // If they are both numbers (int and double) - then it's okay
        // If one of them is a string - then it's also okay, we implicitly cast
        // both values to string.
        if (!(IsNumber(operand) && IsNumber(expr)) &&
            !(ANY_IS(operand, std::string) || ANY_IS(expr, std::string))) {

            throw std::invalid_argument("Cannot perform an operation between different types");
        }

        if (ANY_IS(operand, int)) {
            operand = ANY_RECAST(int, double, operand);
        }

        if (ANY_IS(expr, int)) {
            expr = ANY_RECAST(int, double, expr);
        }

        if (ANY_IS(operand, std::string)) {
            expr = AnyValueToString(expr);
        }

        if (ANY_IS(expr, std::string)) {
            operand = AnyValueToString(operand);
        }
    }

    switch (operator_) {
        case BLAISE_OP_ID::PLUS:
            if (DEBUG > 2) std::cout << "PLUS" << std::endl;
            OPERATION_BLOCK(operand, expr, +, int);
            OPERATION_BLOCK(operand, expr, +, bool);
            OPERATION_BLOCK(operand, expr, +, double);
            OPERATION_BLOCK(operand, expr, +, std::string);
            break;
        case BLAISE_OP_ID::MINUS:
            if (DEBUG > 2) std::cout << "MINUS" << std::endl;
            OPERATION_BLOCK(operand, expr, -, int);
            OPERATION_BLOCK(operand, expr, -, double);
            break;
        case BLAISE_OP_ID::MUL:
            if (DEBUG > 2) std::cout << "MUL" << std::endl;
            OPERATION_BLOCK(operand, expr, *, int);
            OPERATION_BLOCK(operand, expr, *, double);
            break;
        case BLAISE_OP_ID::DIV:
            if (DEBUG > 2) std::cout << "DIV" << std::endl;
            OPERATION_BLOCK(operand, expr, /, int);
            OPERATION_BLOCK(operand, expr, /, double);
            break;
        case BLAISE_OP_ID::EQUAL:
            if (DEBUG > 2) std::cout << "EQUAL" << std::endl;
            OPERATION_BLOCK_ALL_TYPES(operand, expr, ==);
            break;
        case BLAISE_OP_ID::NEQUAL:
            if (DEBUG > 2) std::cout << "NEQUAL" << std::endl;
            OPERATION_BLOCK_ALL_TYPES(operand, expr, !=);
            break;
        case BLAISE_OP_ID::LESS:
            if (DEBUG > 2) std::cout << "LESS" << std::endl;
            OPERATION_BLOCK_ALL_TYPES(operand, expr, <);
            break;
        case BLAISE_OP_ID::LEQUAL:
            if (DEBUG > 2) std::cout << "LEQUAL" << std::endl;
            OPERATION_BLOCK_ALL_TYPES(operand, expr, <=);
            break;
        case BLAISE_OP_ID::GREATER:
            if (DEBUG > 2) std::cout << "GREATER" << std::endl;
            OPERATION_BLOCK_ALL_TYPES(operand, expr, >);
            break;
        case BLAISE_OP_ID::GEQUAL:
            if (DEBUG > 2) std::cout << "GEQUAL" << std::endl;
            OPERATION_BLOCK_ALL_TYPES(operand, expr, >=);
            break;
    }

    return BLAISE_STATUS::ERROR;
}

std::any InterpreterVisitor::visitExprUnaryMinusOperation(BlaiseParser::ExprUnaryMinusOperationContext *context) {
    DEBUG__BEGIN;
    std::any operand = visit(context->operand());
    if (!IsNumber(operand))
        throw std::invalid_argument("Invalid operand for an unary minus operation.");

    any_case_block_begin$$(operand);
        any_case_start$$(double)
            return -(std::any_cast<double>(operand));
        any_case$$(int)
            return -(std::any_cast<int>(operand));
    any_case_block_end$$;

    throw std::invalid_argument(SHOULD_NOT_BE_HERE);
}

std::any InterpreterVisitor::visitExprUnaryPlusOperation(BlaiseParser::ExprUnaryPlusOperationContext *context) {
    DEBUG__BEGIN;
    std::any operand = visit(context->operand());
    if (!IsNumber(operand))
        throw std::invalid_argument("Invalid operand for an unary plus operation.");

    any_case_block_begin$$(operand);
        any_case_start$$(double)
            return +(std::any_cast<double>(operand));
        any_case$$(int)
            return +(std::any_cast<int>(operand));
    any_case_block_end$$;

    throw std::invalid_argument(SHOULD_NOT_BE_HERE);
}

std::any InterpreterVisitor::visitExprOperand(BlaiseParser::ExprOperandContext *context) {
    DEBUG__BEGIN;
    return visitChildren(context);
}


std::any InterpreterVisitor::visitOperandId(BlaiseParser::OperandIdContext *context) {
    const std::string& str = context->IDENTIFIER()->toString();
    const auto& [idptr, blockptr] = FindIdAndBlock(str);


    if (!idptr && !blockptr)
        throw std::invalid_argument("Variable " + str + " has not been defined!");

    if (!blockptr->ids_to_values.at(*idptr).has_value())
        throw std::invalid_argument("Variable " + *idptr + " has not been initialized!");

    return blockptr->ids_to_values.at(*idptr);
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