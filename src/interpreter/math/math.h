#ifndef MATH_H
#define MATH_H

#include <cmath>
#include "../../definitions.h"

/**
 * @class Math
 * @brief Solves stuff.
 * * Role: Performs arithmetic transformations.
 * Security: This ring is "Stateless" and "Read-Only" regarding memory.
 * It calculates results and returns them to the Interpreter,
 * which then coordinates with the Core to save the data.
 */
class Math {
public:
    /**
     * @brief Entry point for all MATH commands.
     * Routes the instruction to the correct arithmetic handler.
     * @return A string representation of the result or an [ERROR].
     */
    static QString execute(const Instruction &inst, QMap<QString, Variable> &memory);

private:
    /**
     * @brief Arithmetic Handlers
     * These take a list of doubles and return the calculated result.
     * Using QList<double> ensures the logic is separated from the Token/Instruction objects.
     */
    static double add(const QList<double>& args);
    static double sub(const QList<double>& args);
    static double mul(const QList<double>& args);
    static double div(const QList<double>& args);
    static double power(const QList<double>& args);
    static double root(const QList<double>& args);
};

#endif // MATH_H
