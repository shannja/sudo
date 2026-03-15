#ifndef DEFINITIONS_H
#define DEFINITIONS_H

#include <QString>
#include <QMap>
#include <QObject>
#include <QVector>
#include <QRegularExpression>
#include <QVariant>
#include <QStack>

/**
 * @struct Variable
 * @brief Represents a stored piece of data in the system memory.
 * Flow: The 'type' helps the Executor decide if it should perform
 * mathematical operations or string concatenations on the 'value'.
 */
struct Variable {
    QString value;
    QString type; // e.g., "NUM", "STR"
};

/**
 * @struct Token
 * @brief The smallest unit of meaning identified by the Lexer.
 * Flow: The Lexer breaks a raw string into these Tokens so the Parser
 * doesn't have to deal with raw text characters.
 */
struct Token {
    enum Type {
        KEYWORD,        // Command verbs: NEW, MATH, OUT, etc.
        TYPE,           // Object categories: VAR, APP, etc.
        DATA_TYPE,      // Value types: NUM, STR, BOOL
        IDENTIFIER,     // User-defined names (variable names)
        VALUE,          // Numeric literals or raw unquoted data
        STRING,         // Character data encapsulated in quotes
        ERROR,          // Used for invalid lexing (e.g., unclosed quotes)
        END             // Signals the end of a token stream
    };

    Type type;
    QString value;
};

// --- ENUMERATIONS ---
// These provide a type-safe way for the C++ backend to "switch"
// between different commands without doing string comparisons.

enum Commands {
    CMD_UNKNOWN,

    CMD_CREATE, CMD_UPDATE, CMD_DELETE,
    CMD_READ, CMD_OUTPUT, CMD_HELP, CMD_INPUT,
    CMD_IF, CMD_ELSE, CMD_ENDIF,
    CMD_WHILE, CMD_ENDWHILE,
    CMD_FUNC, CMD_ENDFUNC, CMD_CALL,

    CMD_CLEAR,

    CMD_MATH, CMD_LOGIC,
};

enum Types { CMD_VARIABLE };

enum DataTypes { CMD_NUMBER, CMD_STRING, CMD_BOOLEAN };

enum SystemIdentifiers { CMD_ENDLINE, CMD_VARIABLES };

enum MathOperations {
    CMD_MATH_INVALID = -1,
    CMD_ADDITION, CMD_SUBTRACTION, CMD_MULTIPLICATION,
    CMD_DIVISION, CMD_RAISE_TO_POWER, CMD_SQUARE_ROOT
};

enum CompareOperations {
    CMD_COMPARE_INVALID = -1,
    CMD_EQUAL, CMD_NEQUAL, CMD_LESS_THAN, CMD_GREATER_THAN, CMD_LESS_THAN_EQUAL, CMD_GREATER_THAN_EQUAL,
};

enum LogicOperations {
    CMD_LOGIC_INVALID = -1,
    CMD_NOT, CMD_AND, CMD_OR
};

/**
 * @struct Instruction
 * @brief The final "Blueprint" produced by the Parser.
 * Flow: The Parser takes a QVector<Token> and fills this struct.
 * The Executor then reads this struct to perform the actual logic.
 */
struct Instruction {
    Commands action = CMD_UNKNOWN;
    Types category = CMD_VARIABLE;
    DataTypes dataType = CMD_NUMBER;
    QString name;               // Usually the target variable name
    QString value;              // Primary value or sub-command (e.g., MATH operation)
    QVector<Token> arguments;   // Extra data (e.g., list of values to add in MATH)

    bool isValid = false;       // Logic gate: if false, the Interpreter stops and reports an error
};

/**
 * Global Mapping Declarations
 * These maps bridge the gap between user-typed strings (like "NEW")
 * and internal Enums (like CMD_CREATE).
 */
extern const QMap<QString, Commands> commandMap;
extern const QMap<QString, Types> typeMap;
extern const QMap<QString, DataTypes> dataTypeMap;
extern const QMap<QString, SystemIdentifiers> systemIdentifiersMap;
extern const QMap<QString, MathOperations> mathOperationsMap;
extern const QMap<QString, CompareOperations> compareOperationsMap;
extern const QMap<QString, LogicOperations> logicOperationsMap;
extern const QStringList reservedWords;

#endif // DEFINITIONS_H
