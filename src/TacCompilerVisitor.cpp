#include "TacCompilerVisitor.h"
#include "BlaiseClasses.h"
#include "Util.h"
#include "antlr/BlaiseParser.h"
#include <any>
#include <atomic>
#include <clocale>
#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <string>
#include <utility>

#define NEWLINE_IF(__condition) \
    ((__condition) ? ";\n" : "")

using TranslationData = std::pair<std::string, std::string>;

std::string TacCompilerVisitor::GetTempVariableName() const {
    return tmp_name_ + std::to_string(tmp_counter_++);
}

std::string TacCompilerVisitor::InsertTemporaryVariables(const std::string& last_tmp) {
    std::string ret;
    std::string dependency = last_tmp;

    auto dep_iter = std::find(block_.variables.rbegin(), block_.variables.rend(), dependency);
    if (dep_iter == block_.variables.rend()) {
        return "";
    }

    TemporaryVariableInfo info = dep_iter->Value<TemporaryVariableInfo>();

    for (const auto decl : info.dependencies) {
        ret += InsertTemporaryVariables(decl->Name());
        ret += NEWLINE_IF(!ret.empty());
    }

    return ret + dep_iter->Name() + dep_iter->Value<TemporaryVariableInfo>().expr;
}

std::any TacCompilerVisitor::visitProgram(BlaiseParser::ProgramContext *context) {
    DEBUG_BEGIN(BLAISE_BEGIN_COUT);

    std::string ret;
    for (auto stmt : context->stmt()) {
        TranslationData data = std::any_cast<TranslationData>(visit(stmt));
        ret += data.first + NEWLINE_IF(!data.first.empty() && !data.second.empty()) + data.second + NEWLINE_IF(!data.second.empty());
    }

    return ret;
}

std::any TacCompilerVisitor::visitStmt(BlaiseParser::StmtContext *context) {
    DEBUG_BEGIN(BLAISE_BEGIN_COUT);
    std::string ret;
    TranslationData data;

    for (auto child : context->children) {
        if (child->getText() == ";") continue;

        data = std::any_cast<TranslationData>(visit(child));
        break;
    }

    return TranslationData(data.first, data.second);
}

std::any TacCompilerVisitor::visitFunctionDeclaration(BlaiseParser::FunctionDeclarationContext *context) {
    DEBUG_BEGIN(BLAISE_BEGIN_COUT);
    return TranslationData("", context->FUNCTION()->toString() + ' '
                            + context->IDENTIFIER()->toString()
                            + '('
                            + std::any_cast<std::string>(visit(context->param_list()))
                            + ')' );
}

std::any TacCompilerVisitor::visitFunctionDefinition(BlaiseParser::FunctionDefinitionContext *context) {
    DEBUG_BEGIN(BLAISE_BEGIN_COUT);
    TranslationData stmt_data = std::any_cast<TranslationData>(visit(context->stmt()));

    return TranslationData("", context->FUNCTION()->toString() + '('
           + (context->param_list() ? std::any_cast<std::string>(visit(context->param_list())) : "")
           + ')'
           + stmt_data.first + stmt_data.second);
}

std::any TacCompilerVisitor::visitFunctionCall(BlaiseParser::FunctionCallContext *context) {
    DEBUG_BEGIN(BLAISE_BEGIN_COUT);
    std::string expr_str;
    std::string dependencies;

    if (context->arg_list()) {
        auto ret = std::any_cast<std::pair<std::string, std::string>>(visit(context->arg_list()));
        dependencies = ret.first;
        expr_str = ret.second;
    }

    return TranslationData(std::move(dependencies),
                           std::move(context->IDENTIFIER()->toString()
                                    + '('
                                    + (expr_str)
                                    + ')'));
}

std::any TacCompilerVisitor::visitParamListComma(BlaiseParser::ParamListCommaContext *context) {
    DEBUG_BEGIN(BLAISE_BEGIN_COUT);
    return context->IDENTIFIER()->toString() + ", " + std::any_cast<std::string>(visit(context->param_list()));
}

std::any TacCompilerVisitor::visitParamListEnd(BlaiseParser::ParamListEndContext *context) {
    DEBUG_BEGIN(BLAISE_BEGIN_COUT);
    return context->IDENTIFIER()->toString();
}

std::any TacCompilerVisitor::visitArgListComma(BlaiseParser::ArgListCommaContext *context) {
    DEBUG_BEGIN(BLAISE_BEGIN_COUT);
    std::string ret;
    TranslationData expr_data;
    std::string dependencies;

    auto [dependencies_next, expr_str_next] = std::any_cast<TranslationData>(visit(context->arg_list()));

    dependencies += (dependencies_next.empty() ? "" : dependencies_next + ";\n");

    if (context->IDENTIFIER())
        ret += context->IDENTIFIER()->toString() + ", " + expr_str_next;
    else {
        expr_data = std::any_cast<TranslationData>(visit(context->expr()));
        dependencies += InsertTemporaryVariables(expr_data.second);
        ret += expr_data.second + ", " + expr_str_next;
    }

    return TranslationData(std::move(dependencies), std::move(ret));
}

std::any TacCompilerVisitor::visitArgListEnd(BlaiseParser::ArgListEndContext *context) {
    DEBUG_BEGIN(BLAISE_BEGIN_COUT);
    if (context->IDENTIFIER())
        return TranslationData("", context->IDENTIFIER()->toString());

    TranslationData expr_data = std::any_cast<TranslationData>(visit(context->expr()));
    return expr_data;
}

std::any TacCompilerVisitor::visitCodeBlock(BlaiseParser::CodeBlockContext *context) {
    DEBUG_BEGIN(BLAISE_BEGIN_COUT);
    std::string ret = "begin\n";

    for (auto stmt : context->stmt()) {
        TranslationData data = std::any_cast<TranslationData>(visit(stmt));
        ret += data.first  + NEWLINE_IF(!data.first.empty())
            +  data.second + NEWLINE_IF(!data.second.empty());
    }

    return TranslationData("", ret + "end");
}

std::any TacCompilerVisitor::visitReturnStmt(BlaiseParser::ReturnStmtContext *context) {
    DEBUG_BEGIN(BLAISE_BEGIN_COUT);

    TranslationData expr_data = std::any_cast<TranslationData>(visit(context->expr()));
    std::string insertions = InsertTemporaryVariables(expr_data.second);
    std::string dependencies = expr_data.first + NEWLINE_IF(!insertions.empty() && !expr_data.first.empty())
                             + insertions;

    return TranslationData(dependencies, "return " + (context->IDENTIFIER()
                ? context->IDENTIFIER()->toString()
                : expr_data.second));
}

std::any TacCompilerVisitor::visitWritelnStmt(BlaiseParser::WritelnStmtContext *context) {
    DEBUG_BEGIN(BLAISE_BEGIN_COUT);

    TranslationData expr_data = std::any_cast<TranslationData>(visit(context->expr()));

    std::string insertions = InsertTemporaryVariables(expr_data.second);

    return TranslationData(expr_data.first
                           + NEWLINE_IF(!insertions.empty() && !expr_data.first.empty())
                           + insertions,
                             "writeln(" + expr_data.second + ')');
}

std::any TacCompilerVisitor::visitIfStmtBlock(BlaiseParser::IfStmtBlockContext *context) {
    DEBUG_BEGIN(BLAISE_BEGIN_COUT);
    std::string ret;
    TranslationData expr_data = std::any_cast<TranslationData>(visit(context->expr()));
    TranslationData stmt_data = std::any_cast<TranslationData>(visit(context->stmt()));
    TranslationData else_data;

    if (context->else_stmt())
        else_data = std::any_cast<TranslationData>(visit(context->else_stmt()));

    std::string dependencies = InsertTemporaryVariables(expr_data.second);

    dependencies += NEWLINE_IF(!expr_data.first.empty() && !dependencies.empty());
    dependencies += expr_data.first + NEWLINE_IF(!stmt_data.first.empty());
    dependencies += stmt_data.first + NEWLINE_IF(!else_data.first.empty());
    dependencies += else_data.first;

    ret += "if (" + expr_data.second + ") then "
        +  stmt_data.second + NEWLINE_IF(!stmt_data.second.empty()) + else_data.second;

    return TranslationData(dependencies, ret);
}

std::any TacCompilerVisitor::visitElseStmtBlock(BlaiseParser::ElseStmtBlockContext *context) {
    DEBUG_BEGIN(BLAISE_BEGIN_COUT);
    TranslationData data = std::any_cast<TranslationData>(visit(context->stmt()));

    return TranslationData(data.first, "else " + data.second);
}

std::any TacCompilerVisitor::visitElseIfStmt(BlaiseParser::ElseIfStmtContext *context) {
    DEBUG_BEGIN(BLAISE_BEGIN_COUT);
    return TranslationData();
}

std::any TacCompilerVisitor::visitLoopStmt(BlaiseParser::LoopStmtContext *context) {
    DEBUG_BEGIN(BLAISE_BEGIN_COUT);
    std::string expr_str = context->expr()->getText();
    TranslationData data = (context->stmt() ? std::any_cast<TranslationData>(visit(context->stmt()))
                                            : TranslationData());


    return TranslationData(data.first, "loop if (" + expr_str
                             + ") "
                             + (!data.second.empty() ? data.second
                             : ";"));
}

std::any TacCompilerVisitor::visitAssignStmt(BlaiseParser::AssignStmtContext *context) {
    DEBUG_BEGIN(BLAISE_BEGIN_COUT);
    std::any expr = visit(context->expr());
    TranslationData expr_data = std::any_cast<TranslationData>(expr);
    std::string insertions = InsertTemporaryVariables(expr_data.second);

    return TranslationData(expr_data.first
                           + NEWLINE_IF(!insertions.empty() && !expr_data.first.empty())
                           + insertions,
                             std::string(context->IDENTIFIER()->toString() + " = " + expr_data.second));
}

std::any TacCompilerVisitor::visitExprOperation(BlaiseParser::ExprOperationContext *context) {
    DEBUG_BEGIN(BLAISE_BEGIN_COUT);
    const TranslationData operand_data = std::any_cast<TranslationData>(visit(context->operand()));
    const std::string& operator_str = std::any_cast<std::string>(visit(context->operator_()));
    const TranslationData expr_data = std::any_cast<TranslationData>(visit(context->expr()));

    std::list<BlaiseVariable *> dependencies;

    auto operand_iter = std::find(block_.variables.rbegin(), block_.variables.rend(), operand_data.second);
    auto expr_iter = std::find(block_.variables.rbegin(), block_.variables.rend(), expr_data.second);

    if (operand_iter != block_.variables.rend()) {
        dependencies.emplace_back(&(*operand_iter));
    }

    if (expr_iter != block_.variables.rend()) {
        dependencies.emplace_back(&(*expr_iter));
    }

    block_.variables.emplace_back(GetTempVariableName(),
            TemporaryVariableInfo{" = " + operand_data.second + operator_str + expr_data.second, std::move(dependencies) });

    return TranslationData(operand_data.first + NEWLINE_IF(!expr_data.first.empty() && !operand_data.first.empty())
                            + expr_data.first,
                            block_.variables.back().Name());
}

std::any TacCompilerVisitor::visitExprUnaryMinusOperation(BlaiseParser::ExprUnaryMinusOperationContext *context) {
    DEBUG_BEGIN(BLAISE_BEGIN_COUT);
    TranslationData op_data = std::any_cast<TranslationData>(visit(context->operand()));
    const std::string& operator_str = context->MINUS()->toString();

    block_.variables.emplace_back(GetTempVariableName(),
            TemporaryVariableInfo{ " = " + operator_str + op_data.second, {} }
        );

    return TranslationData(op_data.first, block_.variables.back().Name());
}

std::any TacCompilerVisitor::visitExprUnaryPlusOperation(BlaiseParser::ExprUnaryPlusOperationContext *context) {
    DEBUG_BEGIN(BLAISE_BEGIN_COUT);
    TranslationData op_data = std::any_cast<TranslationData>(visit(context->operand()));
    const std::string& operator_str = context->PLUS()->toString();

    block_.variables.emplace_back(GetTempVariableName(),
            TemporaryVariableInfo{ " = " + operator_str + op_data.second, {} }
        );

    return TranslationData(op_data.first, block_.variables.back().Name());
}

std::any TacCompilerVisitor::visitExprOperand(BlaiseParser::ExprOperandContext *context) {
    DEBUG_BEGIN(BLAISE_BEGIN_COUT);
    return visit(context->operand());
}

std::any TacCompilerVisitor::visitOperandBoolean(BlaiseParser::OperandBooleanContext *context) {
    DEBUG_BEGIN(BLAISE_BEGIN_COUT);
    return TranslationData("", context->BOOLEAN()->toString());
}

std::any TacCompilerVisitor::visitOperandInt(BlaiseParser::OperandIntContext *context) {
    DEBUG_BEGIN(BLAISE_BEGIN_COUT);
    return TranslationData("", context->INT()->toString());
}

std::any TacCompilerVisitor::visitOperandDouble(BlaiseParser::OperandDoubleContext *context) {
    DEBUG_BEGIN(BLAISE_BEGIN_COUT);
    return TranslationData("", context->DOUBLE()->toString());
}

std::any TacCompilerVisitor::visitOperandChar(BlaiseParser::OperandCharContext *context) {
    DEBUG_BEGIN(BLAISE_BEGIN_COUT);
    return TranslationData("", context->CHAR()->toString());
}

std::any TacCompilerVisitor::visitOperandString(BlaiseParser::OperandStringContext *context) {
    DEBUG_BEGIN(BLAISE_BEGIN_COUT);
    return TranslationData("", context->STRING()->toString());
}

std::any TacCompilerVisitor::visitOperandId(BlaiseParser::OperandIdContext *context) {
    DEBUG_BEGIN(BLAISE_BEGIN_COUT);
    return TranslationData("", context->IDENTIFIER()->toString());
}

std::any TacCompilerVisitor::visitOperandFunctionCall(BlaiseParser::OperandFunctionCallContext *context) {
    DEBUG_BEGIN(BLAISE_BEGIN_COUT);
    return visit(context->function_call());
}

std::any TacCompilerVisitor::visitOperandExpr(BlaiseParser::OperandExprContext *context) {
    DEBUG_BEGIN(BLAISE_BEGIN_COUT);
    return visit(context->expr());
}

std::any TacCompilerVisitor::visitOperatorPlus(BlaiseParser::OperatorPlusContext *context) {
    DEBUG_BEGIN(BLAISE_BEGIN_COUT);
    return std::string(" " + context->PLUS()->toString() + " ");
}

std::any TacCompilerVisitor::visitOperatorMinus(BlaiseParser::OperatorMinusContext *context) {
    DEBUG_BEGIN(BLAISE_BEGIN_COUT);
    return std::string(" " + context->MINUS()->toString() + " ");
}

std::any TacCompilerVisitor::visitOperatorAster(BlaiseParser::OperatorAsterContext *context) {
    DEBUG_BEGIN(BLAISE_BEGIN_COUT);
    return std::string(" " + context->ASTERISK()->toString() + " ");
}

std::any TacCompilerVisitor::visitOperatorSlash(BlaiseParser::OperatorSlashContext *context) {
    DEBUG_BEGIN(BLAISE_BEGIN_COUT);
    return std::string(" " + context->SLASH()->toString() + " ");
}

std::any TacCompilerVisitor::visitOperatorEqual(BlaiseParser::OperatorEqualContext *context) {
    DEBUG_BEGIN(BLAISE_BEGIN_COUT);
    return std::string(" " + context->EQUAL()->toString() + " ");
}

std::any TacCompilerVisitor::visitOperatorNEqual(BlaiseParser::OperatorNEqualContext *context) {
    DEBUG_BEGIN(BLAISE_BEGIN_COUT);
    return std::string(" " + context->NOT_EQUAL()->toString() + " ");
}

std::any TacCompilerVisitor::visitOperatorLess(BlaiseParser::OperatorLessContext *context) {
    DEBUG_BEGIN(BLAISE_BEGIN_COUT);
    return std::string(" " + context->LESS()->toString() + " ");
}

std::any TacCompilerVisitor::visitOperatorLEqual(BlaiseParser::OperatorLEqualContext *context) {
    DEBUG_BEGIN(BLAISE_BEGIN_COUT);
    return std::string(" " + context->LESS_OR_EQUAL()->toString() + " ");
}

std::any TacCompilerVisitor::visitOperatorGreater(BlaiseParser::OperatorGreaterContext *context) {
    DEBUG_BEGIN(BLAISE_BEGIN_COUT);
    return std::string(" " + context->GREATER()->toString() + " ");
}

std::any TacCompilerVisitor::visitOperatorGEqual(BlaiseParser::OperatorGEqualContext *context) {
    DEBUG_BEGIN(BLAISE_BEGIN_COUT);
    return std::string(" " + context->GREATER_OR_EQUAL()->toString() + " ");
}