#ifndef LOGIC_H
#define LOGIC_H


#include "../../definitions.h"

/**
 * @class Logic
 * @brief Solves boolean values.
 * * Role: Performs logical stuff such as comparing and merging.
 */
class Logic {
public:
    typedef bool (*CompareOP)(const double& arg1, const double& arg2);
    typedef bool (*LogicOP)(const bool& arg1, const bool& arg2);

    /**
     * @brief Entry point for all LOGIC commands.
     * Routes the instruction to the correct comparison or logical handler.
     * @return A string representation of the result or an [ERROR].
     */
    static QString execute(const Instruction &inst, QMap<QString, Variable> &memory);

private:

    static const QMap<CompareOperations, CompareOP> comp_operations;
    static const QMap<LogicOperations, LogicOP> logic_operations;

    /**
     * @brief Logic Handlers
     * These compares the two num or str variables and provide a boolean value.
     */
    static bool equal(const double& arg1, const double& arg2);
    static bool notEqual(const double& arg1, const double& arg2);
    static bool greaterThan(const double& arg1, const double& arg2);
    static bool lessThan(const double& arg1, const double& arg2);
    static bool greaterThanEqual(const double& arg1, const double& arg2);
    static bool lessThanEqual(const double& arg1, const double& arg2);

    /**
     * @brief Operator Handlers
     * These compares the two boolean variables and provide a boolean value.
     */
    static bool andOperator(const bool& arg1, const bool& arg2);
    static bool notOperator(const bool& arg1, const bool& arg2);
    static bool orOperator(const bool& arg1, const bool& arg2);
};

#endif // LOGIC_H
