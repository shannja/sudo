#include "definitions.h"

/**
 * @brief Primary Command Map.
 * Maps the ALL-CAPS keyword to the internal Command Enum.
 * These are the entry points for every line of code.
 */
const QMap<QString, Commands> commandMap = {
    // Core commands that are the ONLY ones who manage the memory.
    {"NEW", CMD_CREATE},
    {"UPD", CMD_UPDATE},
    {"DEL", CMD_DELETE},

    // System commands to read memory and provide instructions to the Core to write into.
    {"OUT", CMD_OUTPUT},
    {"READ", CMD_READ},
    {"HELP", CMD_HELP},
    {"IN", CMD_INPUT},

    // Control Flow commands to route logic.
    {"IF", CMD_IF},
    {"ELSE", CMD_ELSE},
    {"ENDIF", CMD_ENDIF},

    // Iteration commands.
    {"WHILE", CMD_WHILE},
    {"ENDWHILE", CMD_ENDWHILE},

    // Grouping commands.
    {"FUNC", CMD_FUNC},
    {"ENDFUNC", CMD_ENDFUNC},
    {"CALL", CMD_CALL},

    // Separate entity commands that does a specific action for a time.
    {"MATH", CMD_MATH},
    {"LGC", CMD_LOGIC},

    // Application-specific command.
    {"CLR", CMD_CLEAR}, // Clears the terminal logs.
};

/**
 * @brief Component Type Map.
 * Used with the 'NEW' command to specify what is being created.
 */
const QMap<QString, Types> typeMap = {
    {"VAR", CMD_VARIABLE}
};

/**
 * @brief Data Type Map.
 * Defines the strict typing system for Variables.
 */
const QMap<QString, DataTypes> dataTypeMap = {
    {"NUM", CMD_NUMBER},  // Double-precision floating point
    {"BOOL", CMD_BOOLEAN}, // Boolean (true/false) logic
    {"STR", CMD_STRING}    // Text literals
};

/**
 * @brief System Identifiers.
 * Special keywords that represent global states or formatting controls.
 */
const QMap<QString, SystemIdentifiers> systemIdentifiersMap = {
    {"MEM", CMD_VARIABLES}, // Target for READ to dump all memory
    {"EOL", CMD_ENDLINE}    // Injected into OUT to create a newline (\n)
};

/**
 * @brief Math Operation Map.
 * Specifically for the 'MATH' command to route calculation logic.
 */
const QMap<QString, MathOperations> mathOperationsMap = {
    {"ADD", CMD_ADDITION},
    {"SUB", CMD_SUBTRACTION},
    {"MUL", CMD_MULTIPLICATION},
    {"DIV", CMD_DIVISION},
    {"POW", CMD_RAISE_TO_POWER},
    {"SQRT", CMD_SQUARE_ROOT}
};

/**
 * @brief Logic Operation Map.
 * Specifically for the 'LOGIC' command to route calculation logic.
 */
const QMap<QString, CompareOperations> compareOperationsMap = {
    // Comparison operators.
    {"EQ", CMD_EQUAL},
    {"NEQ", CMD_NEQUAL},
    {"GT", CMD_GREATER_THAN},
    {"LT", CMD_LESS_THAN},
    {"GTE", CMD_GREATER_THAN_EQUAL},
    {"LTE", CMD_LESS_THAN_EQUAL}
};

const QMap<QString, LogicOperations> logicOperationsMap = {
    // Logical operators.
    {"AND", CMD_AND},
    {"OR", CMD_OR},
    {"NOT", CMD_NOT}
};

/**
 * @brief Master Reserved Word List.
 * A collection of every keyword in the language.
 * Core uses this to prevent users from naming a variable
 * something that would break the Parser (e.g., NEW VAR NUM MATH 10).
 */
const QStringList reservedWords =
    commandMap.keys() +
    typeMap.keys() +
    dataTypeMap.keys() +
    systemIdentifiersMap.keys() +
    mathOperationsMap.keys();
