#ifndef CORE_H
#define CORE_H

#include "../../definitions.h"

/**
 * @class Core
 * @brief Memory management.
 * * This class handles the lifecycle of variables. It is designed as a static
 * utility so it doesn't hold state itself; instead, it operates on the
 * 'memory' map provided by the Interpreter.
 */
class Core {
public:
    /**
     * @brief Main entry point for the Core Ring.
     * Routes the instruction to create, remove, or change functions based on the Action.
     */
    static QString execute(const Instruction &inst, QMap<QString, Variable> &memory);

    /**
     * @brief Helper for analyzing variable and literals.
     * It checks if value is a variable or a literal.
     */
    static QString resolveValue(const QString &rawVal, const QMap<QString, Variable> &memory);

private:
    /**
     * @brief Logic for 'NEW' command.
     * Validates if the variable already exists before allocation.
     */
    static QString create(const Instruction &inst, QMap<QString, Variable> &memory);

    /**
     * @brief Logic for 'DEL' command.
     * Can handle single or multiple variable deletions via the 'arguments' list.
     */
    static QString remove(const Instruction &inst, QMap<QString, Variable> &memory);

    /**
     * @brief Logic for 'UPD' command.
     * Modifies the value of an existing variable while preserving its original Type.
     */
    static QString change(const Instruction &inst, QMap<QString, Variable> &memory);

    /**
     * @brief Helper for change and create.
     * So that no duplicate code.
     */
    static QString validateType(DataTypes type, const QString &val);

};

#endif
