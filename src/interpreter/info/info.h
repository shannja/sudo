#ifndef INFO_H
#define INFO_H

#include "../../definitions.h"

/**
 * @class Info
 * @brief Reads the memory without writing on it, accepts input but not the one who writes it to memory.
 * * This module is responsible for "Reading" from the Nucleus and sending
 * information back to the Bridge (Kernel). It also handles the 'IN'
 * command, which sends a special signal to the QML layer to request data.
 */
class Info {
public:
    /**
     * @brief Entry point for the Info Ring.
     * Routes to echo (OUT), list (READ), help (HELP), or input (IN).
     */
    static QString execute(const Instruction &inst, QMap<QString, Variable> &memory);

private:
    /**
     * @brief Logic for 'OUT'.
     * Concatenates literals and variable values into a single display string.
     */
    static QString echo(const Instruction &inst, QMap<QString, Variable> &memory);

    /**
     * @brief Logic for 'READ'.
     * Inspects a specific variable or dumps the entire memory (MEM).
     */
    static QString list(const Instruction &inst, QMap<QString, Variable> &memory);

    /**
     * @brief Logic for 'IN'.
     * Returns a [SIGNAL] that tells the Kernel/QML to open an input prompt.
     */
    static QString input(const Instruction &inst, QMap<QString, Variable> &memory);

    /**
     * @brief Logic for 'HELP'.
     * Provides syntax documentation for the user inside the terminal.
     */
    static QString help(const Instruction &inst, QMap<QString, Variable> &memory);

    /**
     * @brief Logic for 'CLEAR'.
     * Clears terminal logs.
     */
    static QString clear();
};

#endif
