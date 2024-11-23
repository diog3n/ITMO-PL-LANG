#include <any>
#include <stdexcept>
#include <typeinfo>
#include <utility>

#include "BlaiseClasses.h"
#include "Util.h"

#define ANY_RECAST(__type_from, __type_to, __val) \
    static_cast<__type_to>(std::any_cast<__type_from>(__val))

#define CAST(__type, __val) std::any_cast<__type>(__val)

#define OPERATION_BLOCK(__operand1, __operand2, __operation, __type)                \
    if (__operand1.value_.type() == typeid(__type)) {                                       \
        __type tmp$$var = CAST(__type, __operand1.value_)                            \
                        __operation CAST(__type, __operand2.value_);                 \
        return BlaiseVariable(std::string(), tmp$$var); }

#define OPERATION_BLOCK_ALL_TYPES(__operand1, __operand2, __operation)               \
    OPERATION_BLOCK(__operand1, __operand2, __operation, double);                    \
    OPERATION_BLOCK(__operand1, __operand2, __operation, int);                       \
    OPERATION_BLOCK(__operand1, __operand2, __operation, bool);                      \
    OPERATION_BLOCK(__operand1, __operand2, __operation, char);                      \
    OPERATION_BLOCK(__operand1, __operand2, __operation, std::string)

#define BOOLEAN_LOGIC_BLOCK(__operand1, __operand2, __operation, __type) \
    if (__operand1.value_.type() == typeid(__type)) { \
        bool tmp$$var = (CAST(__type, __operand1.value_) __operation CAST(__type, __operand2.value_)); \
        return BlaiseVariable(tmp$$var); \
    }

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

#define NO_VALUE_MESSAGE(__id) "Variable " + __id + " has no value."

#define NO_VIABLE_CONVERSION(__type_from, __type_to) \
    "No viable conversion from " + std::string(__type_from) + " to " + std::string(__type_to)

inline const std::string InvalidOperationForTypesMsg(const std::type_info& t1,
                                                     const std::type_info& t2) {
    return "Invalid operation for " + std::string(t1.name()) + " and " + std::string(t2.name());
}

inline const std::string InternalTypeAndValueTypeDifferMsg(const BlaiseVariable& var) {
    return "Variable " + var.Name() + " has " + var.Type().name() + " as internal type and "
                       + var.Value().type().name() + " as a value type.";
}

std::string BlaiseVariable::AnyValueToString(const std::any& value) {
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

std::string BlaiseVariable::ToString() const {
    return AnyValueToString(value_);
}


const std::string& BlaiseVariable::Name() const {
    return name_;
}

const std::type_info& BlaiseVariable::Type() const {
    return value_.type();
}

const std::any& BlaiseVariable::Value() const {
    return value_;
}

BlaiseVariable& BlaiseVariable::SetName(const std::string& name) {
    name_ = name;
    return *this;
}

BlaiseVariable& BlaiseVariable::SetValue(const std::any& value) {
    Assign({value});
    return *this;
}

std::pair<BlaiseVariable, BlaiseVariable>
BlaiseVariable::CastToOneType(const BlaiseVariable& lhs, const BlaiseVariable& rhs) {
    if (!lhs.value_.has_value())
        throw std::invalid_argument(NO_VALUE_MESSAGE(lhs.name_));
    if (!rhs.value_.has_value())
        throw std::invalid_argument(NO_VALUE_MESSAGE(rhs.name_));

    if (lhs.value_.type() == rhs.value_.type())
        return std::make_pair(lhs, rhs);

    // Conversion rules

    if (lhs.value_.type() == typeid(double)) {
        if (rhs.value_.type() == typeid(int)) {
            BlaiseVariable var1;
            BlaiseVariable var2;

            var1.value_ = lhs.value_;
            var2.value_ = ANY_RECAST(int, double, rhs.value_);

            return { std::move(var1), std::move(var2) };
        }
    } else if (lhs.value_.type() == typeid(int)) {
        if (rhs.value_.type() == typeid(double)) {
            BlaiseVariable var1;
            BlaiseVariable var2;

            var1.value_ = ANY_RECAST(int, double, lhs.value_);
            var2.value_ = rhs.value_;

            return { std::move(var1), std::move(var2) };
        }
    } else if (lhs.value_.type() == typeid(char)) {
        // Can be added if needed
    } else if (lhs.value_.type() == typeid(bool)) {
        // can be added if needed
    } else if (lhs.value_.type() == typeid(std::string) || rhs.value_.type() == typeid(std::string)) {
        BlaiseVariable var1;
        BlaiseVariable var2;

        var1.value_ = AnyValueToString(lhs.value_);
        var2.value_ = AnyValueToString(rhs.value_);

        return { std::move(var1), std::move(var2) };
    }

    throw std::invalid_argument(NO_VIABLE_CONVERSION(lhs.value_.type().name(),
                                                     rhs.value_.type().name()));
}

bool BlaiseVariable::operator==(const std::string& str) const {
    return str == name_;
}

BlaiseVariable& BlaiseVariable::Assign(const BlaiseVariable& var) {
    if (this == &var)
        return *this;

    value_ = var.value_;
    return *this;

    throw std::invalid_argument("Invalid assignment for variable " + name_);
}

BlaiseVariable BlaiseVariable::operator+(const BlaiseVariable& var) const {
    auto [var1, var2] = CastToOneType(*this, var);

    OPERATION_BLOCK(var1, var2, +, int);
    OPERATION_BLOCK(var1, var2, +, bool);
    OPERATION_BLOCK(var1, var2, +, double);
    OPERATION_BLOCK(var1, var2, +, std::string);

    throw std::invalid_argument(InvalidOperationForTypesMsg(var1.Type(), var2.Type()));
}

BlaiseVariable BlaiseVariable::operator-(const BlaiseVariable& var) const {
    auto [var1, var2] = CastToOneType(*this, var);

    OPERATION_BLOCK(var1, var1, -, int);
    OPERATION_BLOCK(var1, var2, -, double);

    throw std::invalid_argument(InvalidOperationForTypesMsg(var1.Type(), var2.Type()));
}

BlaiseVariable BlaiseVariable::operator*(const BlaiseVariable& var) const {
    auto [var1, var2] = CastToOneType(*this, var);

    OPERATION_BLOCK(var1, var1, *, int);
    OPERATION_BLOCK(var1, var2, *, double);

    throw std::invalid_argument(InvalidOperationForTypesMsg(var1.Type(), var2.Type()));
}

BlaiseVariable BlaiseVariable::operator/(const BlaiseVariable& var) const {
    auto [var1, var2] = CastToOneType(*this, var);

    OPERATION_BLOCK(var1, var1, /, int);
    OPERATION_BLOCK(var1, var2, /, double);

    throw std::invalid_argument(InvalidOperationForTypesMsg(var1.Type(), var2.Type()));
}

BlaiseVariable BlaiseVariable::operator==(const BlaiseVariable& var) const {
    auto [var1, var2] = CastToOneType(*this, var);

    BOOLEAN_LOGIC_BLOCK(var1, var2, ==, int);
    BOOLEAN_LOGIC_BLOCK(var1, var2, ==, double);
    BOOLEAN_LOGIC_BLOCK(var1, var2, ==, char);
    BOOLEAN_LOGIC_BLOCK(var1, var2, ==, std::string);
    BOOLEAN_LOGIC_BLOCK(var1, var2, ==, bool);

    throw std::invalid_argument(InvalidOperationForTypesMsg(this->Type(), var.Type()));
}

BlaiseVariable BlaiseVariable::operator!=(const BlaiseVariable& var) const {
    bool result = !std::any_cast<bool>((*this == var).value_);
    return BlaiseVariable(result);
}

BlaiseVariable BlaiseVariable::operator<(const BlaiseVariable& var) const {
    auto [var1, var2] = CastToOneType(*this, var);

    BOOLEAN_LOGIC_BLOCK(var1, var2, <, int);
    BOOLEAN_LOGIC_BLOCK(var1, var2, <, double);
    BOOLEAN_LOGIC_BLOCK(var1, var2, <, char);

    throw std::invalid_argument(InvalidOperationForTypesMsg(this->Type(), var.Type()));
}

BlaiseVariable BlaiseVariable::operator<=(const BlaiseVariable& var) const {
    auto [var1, var2] = CastToOneType(*this, var);


    BOOLEAN_LOGIC_BLOCK(var1, var2, <=, int);
    BOOLEAN_LOGIC_BLOCK(var1, var2, <=, double);
    BOOLEAN_LOGIC_BLOCK(var1, var2, <=, char);

    throw std::invalid_argument(InvalidOperationForTypesMsg(this->Type(), var.Type()));
}

BlaiseVariable BlaiseVariable::operator>(const BlaiseVariable& var) const {
    auto [var1, var2] = CastToOneType(*this, var);

    BOOLEAN_LOGIC_BLOCK(var1, var2, >, int);
    BOOLEAN_LOGIC_BLOCK(var1, var2, >, double);
    BOOLEAN_LOGIC_BLOCK(var1, var2, >, char);

    throw std::invalid_argument(InvalidOperationForTypesMsg(this->Type(), var.Type()));
}

BlaiseVariable BlaiseVariable::operator>=(const BlaiseVariable& var) const {
    auto [var1, var2] = CastToOneType(*this, var);

    BOOLEAN_LOGIC_BLOCK(var1, var2, >=, int);
    BOOLEAN_LOGIC_BLOCK(var1, var2, >=, double);
    BOOLEAN_LOGIC_BLOCK(var1, var2, >=, char);

    throw std::invalid_argument(InvalidOperationForTypesMsg(this->Type(), var.Type()));
}

BlaiseVariable BlaiseVariable::operator+() const {
    if (value_.type() == typeid(double)) {
        double result = std::any_cast<double>(value_);
        return BlaiseVariable(+result);
    } else if (value_.type() == typeid(int)) {
        int result = std::any_cast<int>(value_);
        return BlaiseVariable(+result);
    }

    throw std::invalid_argument("Invalid operation for type " + std::string(value_.type().name()));
}

BlaiseVariable BlaiseVariable::operator-() const {
    if (value_.type() == typeid(double)) {
        double result = std::any_cast<double>(value_);
        return BlaiseVariable(-result);
    } else if (value_.type() == typeid(int)) {
        int result = std::any_cast<int>(value_);
        return BlaiseVariable(-result);
    }

    throw std::invalid_argument("Invalid operation for type " + std::string(value_.type().name()));
}

bool BlaiseFunction::operator==(const std::string& str) const {
    return str == name_;
}

const std::string& BlaiseFunction::Name() const {
    return name_;
}

const ArgsList& BlaiseFunction::Args() const {
    return args_;
}

void BlaiseFunction::SetBlock(BlaiseParser::StmtContext *block) {
    block_ = block;
}

bool BlaiseFunction::IsDefined() const {
    return block_ != nullptr;
}

BlaiseParser::StmtContext *BlaiseFunction::Block() const {
    return block_;
}
