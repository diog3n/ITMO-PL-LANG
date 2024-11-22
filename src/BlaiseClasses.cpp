#include "BlaiseClasses.h"
#include "antlr/BlaiseParser.h"
#include <any>
#include <stdexcept>
#include <typeinfo>
#include <utility>

#define ANY_RECAST(__type_from, __type_to, __val) \
    static_cast<__type_to>(std::any_cast<__type_from>(__val))

#define CAST(__type, __val) std::any_cast<__type>(__val)

#define OPERATION_BLOCK(__operand1, __operand2, __operation, __type)                \
    if (*__operand1.type == typeid(__type)) {                                       \
        __type tmp$$var = CAST(__type, __operand1.value)                            \
                        __operation CAST(__type, __operand2.value);                 \
        return BlaiseVariable(tmp$$var, typeid(__type)); }

#define OPERATION_BLOCK_ALL_TYPES(__operand1, __operand2, __operation)               \
    OPERATION_BLOCK(__operand1, __operand2, __operation, double);                    \
    OPERATION_BLOCK(__operand1, __operand2, __operation, int);                       \
    OPERATION_BLOCK(__operand1, __operand2, __operation, bool);                      \
    OPERATION_BLOCK(__operand1, __operand2, __operation, char);                      \
    OPERATION_BLOCK(__operand1, __operand2, __operation, std::string)

#define BOOLEAN_LOGIC_BLOCK(__operand1, __operand2, __operation, __type) \
    if (*__operand1.type == typeid(__type)) { \
        bool tmp$$var = (CAST(__type, __operand1.value) == CAST(__type, __operand2.value)); \
        return BlaiseVariable(tmp$$var, typeid(bool)); \
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

std::pair<BlaiseVariable, BlaiseVariable>
BlaiseVariable::CastToOneType(const BlaiseVariable& lhs, const BlaiseVariable& rhs) {
    if (!lhs.value.has_value())
        throw std::invalid_argument(NO_VALUE_MESSAGE(lhs.name));
    if (!rhs.value.has_value())
        throw std::invalid_argument(NO_VALUE_MESSAGE(rhs.name));

    if (*lhs.type == *rhs.type)
        return std::make_pair(lhs, rhs);

    // Conversion rules

    if (*lhs.type == typeid(double)) {
        if (*rhs.type == typeid(int)) {
            BlaiseVariable var1;
            BlaiseVariable var2;

            var1.value = ANY_RECAST(int, double, lhs.value);
            var2.value = rhs.value;

            var1.type = &typeid(double);
            var2.type = &typeid(double);

            return { var1, var2 };
        }
    } else if (*lhs.type == typeid(int)) {
        if (*rhs.type == typeid(double)) {
            BlaiseVariable var1;
            BlaiseVariable var2;

            var1.value = ANY_RECAST(int, double, lhs.value);
            var2.value = rhs.value;

            var1.type = &typeid(double);
            var2.type = &typeid(double);

            return { var1, var2 };
        }
    } else if (*lhs.type == typeid(char)) {
        // Can be added if needed
    } else if (*lhs.type == typeid(bool)) {
        // can be added if needed
    } else if (*lhs.type == typeid(std::string) || *rhs.type == typeid(std::string)) {
        BlaiseVariable var1;
        BlaiseVariable var2;

        var1.value = AnyValueToString(lhs.value);
        var2.value = AnyValueToString(rhs.value);

        var1.type = &typeid(std::string);
        var2.type = &typeid(std::string);

        return { var1, var2 };
    }

    throw std::invalid_argument(NO_VIABLE_CONVERSION(lhs.type->name(), rhs.type->name()));
}

bool BlaiseVariable::operator==(const std::string& str) const {
    return str == name;
}

BlaiseVariable BlaiseVariable::operator+(const BlaiseVariable& var) const {
    auto [var1, var2] = CastToOneType(*this, var);

    OPERATION_BLOCK(var1, var2, +, int);
    OPERATION_BLOCK(var1, var2, +, bool);
    OPERATION_BLOCK(var1, var2, +, double);
    OPERATION_BLOCK(var1, var2, +, std::string);

    throw std::invalid_argument("Invalid operation for given types.");
}

BlaiseVariable BlaiseVariable::operator-(const BlaiseVariable& var) const {
    auto [var1, var2] = CastToOneType(*this, var);

    OPERATION_BLOCK(var1, var1, -, int);
    OPERATION_BLOCK(var1, var2, -, double);

    throw std::invalid_argument("Invalid operation for given types.");
}

BlaiseVariable BlaiseVariable::operator*(const BlaiseVariable& var) const {
    auto [var1, var2] = CastToOneType(*this, var);

    OPERATION_BLOCK(var1, var1, *, int);
    OPERATION_BLOCK(var1, var2, *, double);

    throw std::invalid_argument("Invalid operation for given types.");
}

BlaiseVariable BlaiseVariable::operator/(const BlaiseVariable& var) const {
    auto [var1, var2] = CastToOneType(*this, var);

    OPERATION_BLOCK(var1, var1, /, int);
    OPERATION_BLOCK(var1, var2, /, double);

    throw std::invalid_argument("Invalid operation for given types.");
}

BlaiseVariable BlaiseVariable::operator==(const BlaiseVariable& var) const {
    if (*type != *var.type) {
        throw std::invalid_argument("Invalid operation for given types.");
    }

    BOOLEAN_LOGIC_BLOCK((*this), var, ==, int);
    BOOLEAN_LOGIC_BLOCK((*this), var, ==, double);
    BOOLEAN_LOGIC_BLOCK((*this), var, ==, char);
    BOOLEAN_LOGIC_BLOCK((*this), var, ==, std::string);
    BOOLEAN_LOGIC_BLOCK((*this), var, ==, bool);

    throw std::invalid_argument("Invalid operation for given types.");
}

BlaiseVariable BlaiseVariable::operator!=(const BlaiseVariable& var) const {
    bool result = !std::any_cast<bool>((*this == var).value);
    return BlaiseVariable(result, typeid(bool));
}

BlaiseVariable BlaiseVariable::operator<(const BlaiseVariable& var) const {
    if (*type != *var.type) {
        throw std::invalid_argument("Invalid operation for given types.");
    }

    BOOLEAN_LOGIC_BLOCK((*this), var, <, int);
    BOOLEAN_LOGIC_BLOCK((*this), var, <, double);
    BOOLEAN_LOGIC_BLOCK((*this), var, <, char);

    throw std::invalid_argument("Invalid operation for given types.");
}

BlaiseVariable BlaiseVariable::operator<=(const BlaiseVariable& var) const {
    if (*type != *var.type) {
        throw std::invalid_argument("Invalid operation for given types.");
    }

    BOOLEAN_LOGIC_BLOCK((*this), var, <=, int);
    BOOLEAN_LOGIC_BLOCK((*this), var, <=, double);
    BOOLEAN_LOGIC_BLOCK((*this), var, <=, char);

    throw std::invalid_argument("Invalid operation for given types.");
}

BlaiseVariable BlaiseVariable::operator>(const BlaiseVariable& var) const {
    if (*type != *var.type) {
        throw std::invalid_argument("Invalid operation for given types.");
    }

    BOOLEAN_LOGIC_BLOCK((*this), var, >, int);
    BOOLEAN_LOGIC_BLOCK((*this), var, >, double);
    BOOLEAN_LOGIC_BLOCK((*this), var, >, char);

    throw std::invalid_argument("Invalid operation for given types.");
}

BlaiseVariable BlaiseVariable::operator>=(const BlaiseVariable& var) const {
    if (*type != *var.type) {
        throw std::invalid_argument("Invalid operation for given types.");
    }

    BOOLEAN_LOGIC_BLOCK((*this), var, >=, int);
    BOOLEAN_LOGIC_BLOCK((*this), var, >=, double);
    BOOLEAN_LOGIC_BLOCK((*this), var, >=, char);

    throw std::invalid_argument("Invalid operation for given types.");
}

BlaiseVariable BlaiseVariable::operator+() const {
    if (*type == typeid(double)) {
        double result = std::any_cast<double>(value);
        return BlaiseVariable(+result, typeid(double));
    } else if (*type == typeid(int)) {
        double result = std::any_cast<int>(value);
        return BlaiseVariable(+result, typeid(int));
    }

    throw std::invalid_argument("Invalid operation for given types.");
}

BlaiseVariable BlaiseVariable::operator-() const {
    if (*type == typeid(double)) {
        double result = std::any_cast<double>(value);
        return BlaiseVariable(-result, typeid(double));
    } else if (*type == typeid(int)) {
        double result = std::any_cast<int>(value);
        return BlaiseVariable(-result, typeid(int));
    }

    throw std::invalid_argument("Invalid operation for given types.");
}

bool BlaiseFunction::operator==(const std::string& str) const {
    return str == name;
}
