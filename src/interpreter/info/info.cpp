#include "info.h"
#include "../core/core.h"

/**
 * @brief Logic Routing for Info Ring.
 */
QString Info::execute(const Instruction &inst, QMap<QString, Variable> &memory) {
    switch (inst.action) {
    case CMD_OUTPUT: return echo(inst, memory);
    case CMD_READ:   return list(inst, memory);
    case CMD_HELP:   return help(inst, memory);
    case CMD_INPUT:  return input(inst, memory);
    case CMD_CLEAR:  return clear();
    default: break;
    }
    return "[ERROR] Invalid command action.";
}

/**
 * @brief Logic for OUT.
 * Features smart-spacing and System Identifier support (like EOL).
 */
/**
 * @brief Logic for OUT.
 * Features smart-spacing and System Identifier support (like EOL).
 */
QString Info::echo(const Instruction &inst, QMap<QString, Variable> &memory) {
    QStringList outputParts;

    for (int i = 0; i < inst.arguments.size(); ++i) {
        const Token &t = inst.arguments[i];
        QString valUpper = t.value.toUpper();

        // 1. System Identifiers (EOL)
        if (systemIdentifiersMap.contains(valUpper)) {
            if (systemIdentifiersMap.value(valUpper) == CMD_ENDLINE) {
                outputParts.append("\n");
                continue;
            }
        }

        // 2. String Literals
        if (t.type == Token::STRING) {
            outputParts.append(t.value);
        }

        // 3. Variable Identifiers
        else if (t.type == Token::IDENTIFIER) {
            QString resolved = Core::resolveValue(t.value, memory);
            if (resolved.startsWith("[UNDEFINED_REF]:")) {
                return "[ERROR] OUT: Undefined variable '" + t.value + "'.";
            }
            outputParts.append(resolved);
        }

        // 4. Numeric Literals (Directly in the OUT command)
        else if (t.type == Token::VALUE) {
            outputParts.append(t.value);
        }

        // 5. Smart Spacing
        if (i < inst.arguments.size() - 1) {
            // Only add space if current is not newline AND next is not newline
            bool currentIsEol = (systemIdentifiersMap.value(valUpper) == CMD_ENDLINE);
            bool nextIsEol = (systemIdentifiersMap.value(inst.arguments[i+1].value.toUpper()) == CMD_ENDLINE);

            if (!currentIsEol && !nextIsEol) {
                outputParts.append(" ");
            }
        }
    }

    return outputParts.join("");
}

/**
 * @brief Logic for READ.
 * Supports individual variable inspection or a full memory dump (READ MEM).
 */
QString Info::list(const Instruction &inst, QMap<QString, Variable> &memory) {
    // Check for "MEM" identifier to trigger a full system dump
    if (systemIdentifiersMap.value(inst.name.toUpper()) == CMD_VARIABLES) {
        if (memory.isEmpty()) return "MEM_STATE: EMPTY";

        QString table = "--- SYSTEM MEMORY DUMP ---\n";
        for (auto it = memory.begin(); it != memory.end(); ++it) {
            table += QString("[%1] %2 = %3\n").arg(it.value().type, it.key(), it.value().value);
        }
        return table.trimmed();
    }

    // Standard lookup
    if (memory.contains(inst.name)) {
        return QString("[%1] %2 = %3").arg(memory[inst.name].type, inst.name, memory[inst.name].value);
    }

    return "[ERROR] READ target '" + inst.name + "' not found.";
}

/**
 * @brief Logic for IN.
 * Returns a signal string that the Kernel/Interpreter loop recognizes as a PAUSE command.
 * Part of the Level 1 (Info Ring) Logic.
 */
QString Info::input(const Instruction &inst, QMap<QString, Variable> &memory) {
    // 1. Validate Command Type
    if (inst.action != CMD_INPUT) {
        return "[INTERNAL_ERROR]: Info::input called with non-input instruction.";
    }

    // 2. Strict Memory Check (Level 0/1 Constraint)
    if (!memory.contains(inst.name)) {
        return "[ERROR]: Variable '" + inst.name + "' not declared. Use NEW VAR first.";
    }

    // 3. Initiate U-Turn / Pause Flow
    // Signals Level 4 (TerminalBridge) to halt the m_currentLineIndex
    // and wait for user data to be sent back via CMD_UPDATE.
    return "[SIGNAL_INPUT_REQUIRED]:" + inst.name;
}

QString Info::clear() {
    return "[SIGNAL_CLEAR_LOG]";
}

/**
 * @brief Logic for HELP.
 * Uses the commandMap to provide context-sensitive help for specific commands.
 */
QString Info::help(const Instruction &inst, QMap<QString, Variable> &memory) {
    if (inst.value.isEmpty()) {
        return "=== Sudo_OS SYSTEM MANUAL ===\n"
               "Usage: HELP [COMMAND]\n\n"
               "DATA MANAGEMENT:   NEW, UPD, DEL, READ\n"
               "COMMUNICATION:     OUT, IN, HELP\n"
               "OPERATIONS:        MATH\n\n"
               "Example: HELP MATH";
    }

    Commands topic = commandMap.value(inst.value, CMD_UNKNOWN);

    switch (topic) {
    case CMD_CREATE:
        return "COMMAND: NEW\n"
               "Description: Creates and initializes a new variable.\n"
               "Syntax: NEW VAR [TYPE] [NAME] [VALUE]\n"
               "Types: NUM, STR, BOOL\n"
               "Example: NEW VAR NUM score 0";

    case CMD_UPDATE:
        return "COMMAND: UPD\n"
               "Description: Updates an existing variable.\n"
               "Syntax: UPD VAR [NAME] [NEW_VALUE]\n"
               "Example: UPD VAR score 100\n"
               "Note: Variable must exist first.";

    case CMD_MATH:
        return "COMMAND: MATH\n"
               "Description: Performs calculations and saves result.\n"
               "Syntax: MATH [OP] [TARGET] [VAL1] [VAL2...]\n"
               "Ops: ADD, SUB, MUL, DIV, POW, SQRT\n"
               "Example: MATH ADD total price tax";

    default:
        return "[ERROR] Command '" + inst.value + "' is not recognized.";
    }
}
