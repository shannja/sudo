#include "info.h"
#include "../core/core.h"
#include "../filesystem/filesystem.h"

\
const QMap<Commands, Info::InfoOP> Info::operations = {
    {CMD_OUTPUT, Info::echo},
    {CMD_READ,   Info::list},
    {CMD_HELP,   Info::help},
    {CMD_INPUT,  Info::input}
};

/**
 * @brief THE DISPATCHER: Routes Info Ring actions.
 * Updated to use the Dispatch Map pattern for consistency with FL and Math rings.
 */
QString Info::execute(const Instruction &inst, QMap<QString, Variable> &memory) {

    // Special case for CLEAR as it takes no memory/instruction args usually
    if (inst.action == CMD_CLEAR) return Info::clear();

    if (!operations.contains(inst.action)) {
        return "[ERROR] Info Ring: Invalid action.";
    }

    return operations[inst.action](inst, memory);
}

/**
 * @brief Logic for HELP.
 * Expanded to include your new Ring systems (FileSystem and Logic).
 */
QString Info::help(const Instruction &inst, QMap<QString, Variable> &memory) {
    Q_UNUSED(memory);

    if (inst.value.isEmpty()) {
        return "=== SUDO_VM SYSTEM MANUAL ===\n"
               "Usage: HELP [COMMAND]\n\n"
               "FILE SYSTEM (FL):  MKDIR, CD, RMDIR, RNDIR\n"
               "DATA MGMT:         NEW, UPD, DEL, READ\n"
               "LOGIC & MATH:      LOGIC, MATH\n"
               "I/O OPERATIONS:    OUT, IN, CLEAR\n\n"
               "Try: HELP FL or HELP LOGIC";
    }

    QString topic = inst.value.toUpper();

    // 1. Help for FileSystem Ring
    if (topic == "FL") {
        return "SERVICE: FileSystem (Sandbox)\n"
               "Commands:\n"
               "  FL MKDIR [name]       - Create folder\n"
               "  FL CD [path/..]       - Change directory\n"
               "  FL RMDIR [name]       - Remove empty folder\n"
               "  FL RNDIR [old] [new]  - Rename folder";
    }

    // 2. Help for Logic Ring
    if (topic == "LOGIC") {
        return "SERVICE: Logic (Boolean Ops)\n"
               "Syntax: LOGIC [OP] [TARGET_VAR] [VAL1] [VAL2]\n"
               "Ops: EQ, NEQ, GT, LT, AND, OR, NOT\n"
               "Example: LOGIC GT isGreater score 50";
    }

    // 3. Context-sensitive lookup for specific Command Enums
    Commands cmdTopic = commandMap.value(topic, CMD_UNKNOWN);
    switch (cmdTopic) {
    case CMD_CREATE:
        return "COMMAND: NEW\nSyntax: NEW VAR [TYPE] [NAME] [VALUE]\nTypes: NUM, STR, BOOL";
    case CMD_MATH:
        return "COMMAND: MATH\nSyntax: MATH [OP] [TARGET] [V1] [V2]\nOps: ADD, SUB, MUL, DIV, POW, ROOT";
    case CMD_OUTPUT:
        return "COMMAND: OUT\nSyntax: OUT \"Text\" variable EOL\nNote: EOL adds a newline.";
    default:
        return "[ERROR] No manual entry for: " + topic;
    }
}

/**
 * @brief Logic for OUT (Echo).
 */
QString Info::echo(const Instruction &inst, QMap<QString, Variable> &memory) {
    QStringList outputParts;

    for (int i = 0; i < inst.arguments.size(); ++i) {
        const Token &t = inst.arguments[i];
        QString valUpper = t.value.toUpper();

        // 1. System Identifiers (EOL)
        if (systemIdentifiersMap.value(valUpper) == CMD_ENDLINE) {
            outputParts.append("\n");
            continue;
        }

        // 2. Resolve based on Token Type
        if (t.type == Token::IDENTIFIER) {
            QString resolved = Core::resolveValue(t.value, memory);
            if (resolved.startsWith("[UNDEFINED_REF]:")) {
                return "[ERROR] OUT: Undefined variable '" + t.value + "'.";
            }
            outputParts.append(resolved);
        } else {
            outputParts.append(t.value); // Literals (String/Value)
        }

        // 3. Smart Spacing Logic
        if (i < inst.arguments.size() - 1) {
            bool nextIsEol = (systemIdentifiersMap.value(inst.arguments[i+1].value.toUpper()) == CMD_ENDLINE);
            if (systemIdentifiersMap.value(valUpper) != CMD_ENDLINE && !nextIsEol) {
                outputParts.append(" ");
            }
        }
    }
    return outputParts.join("");
}

/**
 * @brief Logic for READ (Memory Inspection).
 */
QString Info::list(const Instruction &inst, QMap<QString, Variable> &memory) {
    if (systemIdentifiersMap.value(inst.name) == CMD_VARIABLES) {
        if (memory.isEmpty()) return "MEM_STATE: EMPTY";
        QString table = "--- SYSTEM MEMORY DUMP ---\n";
        for (auto it = memory.begin(); it != memory.end(); ++it) {
            table += QString("[%1] %2 = %3\n").arg(it.value().type, it.key(), it.value().value);
        }
        return table.trimmed();
    }

    if (memory.contains(inst.name)) {
        return QString("[%1] %2 = %3").arg(memory[inst.name].type, inst.name, memory[inst.name].value);
    }

    if (systemIdentifiersMap.value(inst.name) == CMD_ROOT_DIRECTORY) {
        return FileSystem::getTree();
    }

    return "[ERROR] READ: '" + inst.name + "' not found.";
}

QString Info::input(const Instruction &inst, QMap<QString, Variable> &memory) {
    if (!memory.contains(inst.name)) return "[ERROR] IN: Variable '" + inst.name + "' not declared.";
    return "[SIGNAL_INPUT_REQUIRED]:" + inst.name;
}

QString Info::clear() {
    return "[SIGNAL_CLEAR_LOG]";
}
