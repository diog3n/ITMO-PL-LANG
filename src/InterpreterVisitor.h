#pragma once

#include <deque>
#include <string>
#include <typeinfo>

#include "antlr/BlaiseBaseVisitor.h"
#include "BlaiseClasses.h"
#include "antlr/BlaiseParser.h"

class InterpreterVisitor : public BlaiseBaseVisitor {
public:
    BlaiseBlock *gl_block;              // global block

    std::deque<BlaiseBlock> stack_frames;

private:
    static const std::type_info& StringToTypeId(const std::string& str);

    static std::string StringToUpper(std::string str);

    std::pair<BlaiseVariable *, BlaiseBlock *> FindVarAndBlock(const std::string& id);

    std::pair<BlaiseFunction *, BlaiseBlock *> FindFunctionAndBlock(const std::string& id);

    BlaiseFunction& AddFunction(const std::string& name,
                                BlaiseParser::Param_listContext *paramlist);

    void DebugPrintStack() const;

public:
    InterpreterVisitor();

    virtual std::any visitProgram(BlaiseParser::ProgramContext *context) override;

    virtual std::any visitStmt(BlaiseParser::StmtContext *context) override;

    virtual std::any visitFunctionDefinition(BlaiseParser::FunctionDefinitionContext *context) override;

    virtual std::any visitFunctionCall(BlaiseParser::FunctionCallContext *context) override;

    virtual std::any visitParamListComma(BlaiseParser::ParamListCommaContext *context) override;

    virtual std::any visitParamListEnd(BlaiseParser::ParamListEndContext *context) override;

    virtual std::any visitArgListComma(BlaiseParser::ArgListCommaContext *context) override;

    virtual std::any visitArgListEnd(BlaiseParser::ArgListEndContext *context) override;

    virtual std::any visitCodeBlock(BlaiseParser::CodeBlockContext *context) override;

    virtual std::any visitAssignStmt(BlaiseParser::AssignStmtContext *ctx) override;

    virtual std::any visitReturnStmt(BlaiseParser::ReturnStmtContext *context) override;

    virtual std::any visitWritelnStmt(BlaiseParser::WritelnStmtContext *context) override;

    virtual std::any visitIfStmtBlock(BlaiseParser::IfStmtBlockContext *context) override;

    virtual std::any visitElseStmtBlock(BlaiseParser::ElseStmtBlockContext *context) override;

    virtual std::any visitLoopStmt(BlaiseParser::LoopStmtContext *context) override;

    virtual std::any visitExprOperation(BlaiseParser::ExprOperationContext *context) override;

    virtual std::any visitExprUnaryMinusOperation(BlaiseParser::ExprUnaryMinusOperationContext *context) override;

    virtual std::any visitExprUnaryPlusOperation(BlaiseParser::ExprUnaryPlusOperationContext *context) override;

    virtual std::any visitExprOperand(BlaiseParser::ExprOperandContext *context) override;

    virtual std::any visitOperandId(BlaiseParser::OperandIdContext *context) override;

    virtual std::any visitOperandInt(BlaiseParser::OperandIntContext *context) override;

    virtual std::any visitOperandDouble(BlaiseParser::OperandDoubleContext *context) override;

    virtual std::any visitOperandChar(BlaiseParser::OperandCharContext *context) override;

    virtual std::any visitOperandString(BlaiseParser::OperandStringContext *context) override;

    virtual std::any visitOperandBoolean(BlaiseParser::OperandBooleanContext *context) override;

    virtual std::any visitOperandFunctionCall(BlaiseParser::OperandFunctionCallContext *context) override;

    virtual std::any visitOperandExpr(BlaiseParser::OperandExprContext *context) override;

    virtual std::any visitOperatorPlus(BlaiseParser::OperatorPlusContext *context) override;

    virtual std::any visitOperatorMinus(BlaiseParser::OperatorMinusContext *context) override;

    virtual std::any visitOperatorAster(BlaiseParser::OperatorAsterContext *context) override;

    virtual std::any visitOperatorSlash(BlaiseParser::OperatorSlashContext *context) override;

    virtual std::any visitOperatorEqual(BlaiseParser::OperatorEqualContext *context) override;

    virtual std::any visitOperatorNEqual(BlaiseParser::OperatorNEqualContext *context) override;

    virtual std::any visitOperatorLess(BlaiseParser::OperatorLessContext *context) override;

    virtual std::any visitOperatorLEqual(BlaiseParser::OperatorLEqualContext *context) override;

    virtual std::any visitOperatorGreater(BlaiseParser::OperatorGreaterContext *context) override;

    virtual std::any visitOperatorGEqual(BlaiseParser::OperatorGEqualContext *context) override;

};