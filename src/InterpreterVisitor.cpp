#include <algorithm>
#include <any>
#include <cctype>
#include <stdexcept>
#include <string>
#include <typeinfo>
#include <utility>

#include "InterpreterVisitor.h"
#include "BlaiseClasses.h"
#include "antlr/BlaiseParser.h"
#include "Util.h"

InterpreterVisitor::InterpreterVisitor() {
    stack_frames.emplace_back();
    gl_block = &stack_frames.back();
}

std::string InterpreterVisitor::StringToUpper(std::string str) {
    std::transform(str.begin(), str.end(), str.begin(), toupper);
    return str;
}

std::pair<BlaiseVariable *, BlaiseBlock *>
InterpreterVisitor::FindVarAndBlock(const std::string& str) {
    BlaiseBlock *block = nullptr;
    BlaiseVariable *id = nullptr;

    for (auto riter = stack_frames.rbegin(); riter != stack_frames.rend(); riter++) {
        for (auto id_riter = riter->variables.rbegin(); id_riter != riter->variables.rend(); id_riter++) {
            if (id_riter->Name() == str) {
                id = &(*id_riter);
                block = &(*riter);

                return std::make_pair(id, block);
            }
        }
    }
    return std::make_pair(nullptr, nullptr);
}

std::pair<BlaiseFunction *, BlaiseBlock *>
InterpreterVisitor::FindFunctionAndBlock(const std::string& str) {
    BlaiseBlock *block = nullptr;
    BlaiseFunction *func = nullptr;

    for (auto riter = stack_frames.rbegin(); riter != stack_frames.rend(); riter++) {
        for (auto id_riter = riter->functions.rbegin(); id_riter != riter->functions.rend(); id_riter++) {
            if (id_riter->Name() == str) {
                func = &(*id_riter);
                block = &(*riter);

                return std::make_pair(func, block);
            }
        }
    }

    throw std::invalid_argument("Function " + str + " has not been defined!");
}

void InterpreterVisitor::DebugPrintStack() const {
    DEBUG_OUT(0) << "==== Variables ====" << std::endl;
    for (auto riter = stack_frames.rbegin(); riter != stack_frames.rend(); riter++) {
        for (auto id_riter = riter->variables.rbegin(); id_riter != riter->variables.rend(); id_riter++) {
                DEBUG_OUT(0) << id_riter->Name() << ", type: " << id_riter->Type().name() << ", value = " << id_riter->ToString() << std::endl;
        }
    }

    DEBUG_OUT(0) << "==== Functions ====" << std::endl;
    for (auto riter = stack_frames.rbegin(); riter != stack_frames.rend(); riter++) {
        for (auto id_riter = riter->functions.rbegin(); id_riter != riter->functions.rend(); id_riter++) {
                DEBUG_OUT(0) << id_riter->Name() << std::endl;
        }
    }
}

BlaiseFunction& InterpreterVisitor::AddFunction(const std::string& name,
                                                BlaiseParser::Param_listContext *paramlist) {
    ArgsList args;

    if (paramlist)
        args = std::move(std::any_cast<ArgsList>(visit(paramlist)));

    BlaiseFunction& func = stack_frames.back().functions.emplace_back(name, args);

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
    } else if (str == "nothing") {
        return typeid(void);
    }

    throw std::invalid_argument("Invalid type identifier: " + str);
}

std::any InterpreterVisitor::visitProgram(BlaiseParser::ProgramContext *context) {
    std::any value;
    try {
        value = visitChildren(context);
    } catch (const BlaiseVariable& ret) {
        throw std::invalid_argument("Return statement is not allowed outside of functions");
    }

    // Stack contents
    if (DEBUG >= 1) {
        for (const auto& var : gl_block->variables) {
            std::cout << var.Type().name() << " " << var.Name() << " = "
                      << var.ToString()
                      << std::endl;
        }

        for (const auto& func : gl_block->functions) {
            std::cout << func.Name() << std::endl;
            for (const auto& arg : func.Args()) {
                std::cout << arg.Name() << ": " << arg.Type().name() << std::endl;
            }
        }
    }

    return value;
}

std::any InterpreterVisitor::visitStmt(BlaiseParser::StmtContext *context) {
    // DebugPrintStack();
    return visitChildren(context);
}

std::any InterpreterVisitor::visitFunctionDefinition(BlaiseParser::FunctionDefinitionContext *context) {
    const std::string& id = context->IDENTIFIER()->toString();

    auto iter = std::find(stack_frames.back().functions.begin(), stack_frames.back().functions.end(), id);

    if (iter == stack_frames.back().functions.end()) {
        AddFunction(id, context->param_list());
    }

    if (!stack_frames.back().functions.back().IsDefined()) {
        stack_frames.back().functions.back().SetBlock(context->stmt());
    } else {
        throw std::invalid_argument("Function redefinition is not allowed. Function " + id + " is already defined.");
    }

    return 0;
}

std::any InterpreterVisitor::visitFunctionCall(BlaiseParser::FunctionCallContext *context) {
    const std::string& id = context->IDENTIFIER()->toString();
    ArgsList args;
    auto [funcptr, _] = FindFunctionAndBlock(id);

    if (context->arg_list())
        args = std::move(std::any_cast<ArgsList>(visit(context->arg_list())));


    if (args.size() != funcptr->Args().size()) {
        throw std::invalid_argument("Wrong amount of aguments for function " + funcptr->Name());
    }

    auto aiter = args.begin();
    auto fiter = funcptr->Args().begin();

    stack_frames.emplace_back();

    for ( ;fiter != funcptr->Args().end(); fiter++, aiter++) {
        auto& var = stack_frames.back().variables.emplace_back(fiter->Name());
        var.Assign(*aiter);
    }

    try {
        visit(funcptr->Block());

    } catch (const BlaiseVariable& var) {

        stack_frames.pop_back();
        return BlaiseVariable(var); // explicit copying to make my LSP shut up
    }

    stack_frames.pop_back();
    return BlaiseVariable();
}

std::any InterpreterVisitor::visitParamListComma(BlaiseParser::ParamListCommaContext *context) {
    const std::string& id = context->IDENTIFIER()->toString();
    auto args = std::any_cast<ArgsList>(visit(context->param_list()));
    auto iter = std::find(args.begin(), args.end(), id);

    if ( iter != args.end()) {
        throw std::invalid_argument("Identifier " + id + " already is in the list.");
    }

    args.emplace_front(id);

    return args;
}

std::any InterpreterVisitor::visitParamListEnd(BlaiseParser::ParamListEndContext *context) {
    const std::string& id = context->IDENTIFIER()->toString();

    ArgsList args;
    args.emplace_front(id);

    return args;
}

std::any InterpreterVisitor::visitArgListComma(BlaiseParser::ArgListCommaContext *context) {
    BlaiseVariable var;

    if (context->IDENTIFIER()) {
        auto [varptr, _] = FindVarAndBlock(context->IDENTIFIER()->toString());
        var = *varptr;
    } else if (context->expr()) {
        var = std::move(std::any_cast<BlaiseVariable>(visit(context->expr())));
    }

    ArgsList args = std::any_cast<ArgsList>(visit(context->arg_list()));

    args.push_front(var);

    return args;
}

std::any InterpreterVisitor::visitArgListEnd(BlaiseParser::ArgListEndContext *context) {
    BlaiseVariable var;

    if (context->IDENTIFIER()) {
        auto [varptr, _] = FindVarAndBlock(context->IDENTIFIER()->toString());
        var = *varptr;
    } else if (context->expr()) {
        var = std::move(std::any_cast<BlaiseVariable>(visit(context->expr())));
    }
    ArgsList args;

    args.push_front(var);

    return args;
}

std::any InterpreterVisitor::visitCodeBlock(BlaiseParser::CodeBlockContext *context) {
    stack_frames.emplace_back();
    try {
        std::any value = visitChildren(context);
        stack_frames.pop_back();
        return value;
    } catch (const BlaiseVariable& ret) {
        stack_frames.pop_back();
        throw;
    }
}

std::any InterpreterVisitor::visitAssignStmt(BlaiseParser::AssignStmtContext *context) {
    const std::string& id = context->IDENTIFIER()->toString();
    auto [varptr, block] = FindVarAndBlock(id);
    BlaiseVariable value = std::any_cast<BlaiseVariable>(visit(context->expr()));

    // If there is no such variable
    if (varptr == nullptr) {
        const BlaiseVariable& var = stack_frames.back().variables.emplace_back(id, value.Value());
        return var;
    }

    varptr->Assign(value);
    return *varptr;
}

std::any InterpreterVisitor::visitWritelnStmt(BlaiseParser::WritelnStmtContext *context) {
    BlaiseVariable var = std::any_cast<BlaiseVariable>(visit(context->expr()));

    std::cout << (DEBUG ? "writeln: " : "") << var.ToString() << std::endl;

    return var;
}

std::any InterpreterVisitor::visitReturnStmt(BlaiseParser::ReturnStmtContext *context) {
    throw std::any_cast<BlaiseVariable>(visit(context->expr()));
}

std::any InterpreterVisitor::visitIfStmtBlock(BlaiseParser::IfStmtBlockContext *context) {
    BlaiseVariable var = std::any_cast<BlaiseVariable>(visit(context->expr()));
    bool condition;

    if (var.Is<bool>())
        condition = var.Value<bool>();
    else
        throw std::invalid_argument("If statement expression must be boolean!");

    if (condition) {
        stack_frames.emplace_back();

        // We have to be ready to the fact that the stmt can
        // be a return statement.
        try {
            std::any value = visit(context->stmt());
            stack_frames.pop_back();
            return value;
        } catch (const BlaiseVariable& ret) {
            // cleanup the stack
            stack_frames.pop_back();
            // propagate upwards
            throw;
        }

    }

    if (context->else_stmt())
        return visit(context->else_stmt());

    return false;
}

std::any InterpreterVisitor::visitElseStmtBlock(BlaiseParser::ElseStmtBlockContext *context) {
    stack_frames.emplace_back();

    // Again, be ready for return statement
    try {
        std::any value = visitChildren(context);
        stack_frames.pop_back();
        return value;
    } catch (const BlaiseVariable& var) {
        stack_frames.pop_back();
        throw;
    }
}

std::any InterpreterVisitor::visitLoopStmt(BlaiseParser::LoopStmtContext *context) {
    bool condition = false;


    while (true) {
        stack_frames.emplace_back();
        BlaiseVariable var = std::move(std::any_cast<BlaiseVariable>(visit(context->expr())));

        if (var.Is<bool>())
            condition = var.Value<bool>();
        else
            throw std::invalid_argument("Loop if statement expression must be boolean!");

        if (!condition) break;

        // Be ready for return statement
        try {
            if (context->stmt())
                visit(context->stmt());

        } catch (const BlaiseVariable& ret) {

            stack_frames.pop_back();
            throw;
        }

        stack_frames.pop_back();
    }


    return condition;
}

std::any InterpreterVisitor::visitExprOperation(BlaiseParser::ExprOperationContext *context) {
    BlaiseVariable operand = std::move(std::any_cast<BlaiseVariable>(visit(context->operand())));
    BLAISE_OP_ID operator_ = std::move(std::any_cast<BLAISE_OP_ID>(visit(context->operator_())));
    BlaiseVariable expr    = std::move(std::any_cast<BlaiseVariable>(visit(context->expr())));

    switch (operator_) {
        case BLAISE_OP_ID::PLUS:
            DEBUG_OUT(BLAISE_DEBUG_INTERNAL_INFO) << "PLUS" << std::endl;

            return operand + expr;
            break;
        case BLAISE_OP_ID::MINUS:
            DEBUG_OUT(BLAISE_DEBUG_INTERNAL_INFO) << "MINUS" << std::endl;

            return operand - expr;
            break;
        case BLAISE_OP_ID::MUL:
            DEBUG_OUT(BLAISE_DEBUG_INTERNAL_INFO) << "MUL" << std::endl;

            return operand * expr;
            break;
        case BLAISE_OP_ID::DIV:
            DEBUG_OUT(BLAISE_DEBUG_INTERNAL_INFO) << "DIV" << std::endl;

            return operand / expr;
            break;
        case BLAISE_OP_ID::EQUAL:
            DEBUG_OUT(BLAISE_DEBUG_INTERNAL_INFO) << "EQUAL" << std::endl;

            return operand == expr;
            break;
        case BLAISE_OP_ID::NEQUAL:
            DEBUG_OUT(BLAISE_DEBUG_INTERNAL_INFO) << "NEQUAL" << std::endl;

            return operand == expr;
            break;
        case BLAISE_OP_ID::LESS:
            DEBUG_OUT(BLAISE_DEBUG_INTERNAL_INFO) << "LESS" << std::endl;

            return operand < expr;
            break;
        case BLAISE_OP_ID::LEQUAL:
            DEBUG_OUT(BLAISE_DEBUG_INTERNAL_INFO) << "LEQUAL" << std::endl;

            return operand <= expr;
            break;
        case BLAISE_OP_ID::GREATER:
            DEBUG_OUT(BLAISE_DEBUG_INTERNAL_INFO) << "GREATER" << std::endl;

            return operand > expr;
            break;
        case BLAISE_OP_ID::GEQUAL:
            DEBUG_OUT(BLAISE_DEBUG_INTERNAL_INFO) << "GEQUAL" << std::endl;

            return operand >= expr;
            break;
    }

    throw std::invalid_argument("Unknown operator encountered");
}

std::any InterpreterVisitor::visitExprUnaryMinusOperation(BlaiseParser::ExprUnaryMinusOperationContext *context) {
    BlaiseVariable operand = std::any_cast<BlaiseVariable>(visit(context->operand()));

    return -operand;
}

std::any InterpreterVisitor::visitExprUnaryPlusOperation(BlaiseParser::ExprUnaryPlusOperationContext *context) {
    BlaiseVariable operand = std::any_cast<BlaiseVariable>(visit(context->operand()));

    return +operand;
}

std::any InterpreterVisitor::visitExprOperand(BlaiseParser::ExprOperandContext *context) {
    return visitChildren(context);
}

std::any InterpreterVisitor::visitOperandId(BlaiseParser::OperandIdContext *context) {
    const std::string& str = context->IDENTIFIER()->toString();

    const auto [varptr, blockptr] = FindVarAndBlock(str);

    return (*varptr);
}

std::any InterpreterVisitor::visitOperandInt(BlaiseParser::OperandIntContext *context) {
    int value = std::stoi(context->INT()->toString());
    return BlaiseVariable(value);
}

std::any InterpreterVisitor::visitOperandDouble(BlaiseParser::OperandDoubleContext *context) {
    double value = std::stod(context->DOUBLE()->toString());
    return BlaiseVariable(value);
}

std::any InterpreterVisitor::visitOperandChar(BlaiseParser::OperandCharContext *context) {
    std::string str = context->CHAR()->toString();
    char value = str.at(1);
    return BlaiseVariable(value);
}

std::any InterpreterVisitor::visitOperandString(BlaiseParser::OperandStringContext *context) {
    std::string full_str = context->STRING()->toString();
    return BlaiseVariable(std::string(), std::string(full_str.begin() + 1, full_str.end() - 1));
}

std::any InterpreterVisitor::visitOperandFunctionCall(BlaiseParser::OperandFunctionCallContext *context) {
    BlaiseVariable var = std::any_cast<BlaiseVariable>(visit(context->function_call()));
    return var;
}

std::any InterpreterVisitor::visitOperandExpr(BlaiseParser::OperandExprContext *context) {
    return visit(context->expr());
}

std::any InterpreterVisitor::visitOperandBoolean(BlaiseParser::OperandBooleanContext *context) {
    std::string str = context->BOOLEAN()->toString();
    if (str == "true") return BlaiseVariable(true);
    else if (str == "false") return BlaiseVariable(false);

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