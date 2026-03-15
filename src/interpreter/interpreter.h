#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "../definitions.h"

/**
 * @class Interpreter
 * @brief Tokenizes input and checks script grammar.
 * * This class is responsible for the lifecycle of a script:
 * 1. Lexing (Tokenizing)
 * 2. Parsing (Instruction building)
 * 3. Execution (Routing to Rings)
 * * It maintains the "Nucleus" (memory) and the "Instruction Pointer" (m_currentLineIndex).
 */
class Interpreter : public QObject {
    Q_OBJECT
public:
    /**
     * @brief Accepts a full script, splits it into lines, and begins execution.
     */
    QString process(QString rawInput, QMap<QString, Variable> &memory);

    /**
     * @brief Resumes execution from the current index after a [SIGNAL] pause.
     * @param userInput The value provided by the user.
     * @param targetVar The variable name where the value will be stored.
     */
    QString resume(QString userInput, QString targetVar, QMap<QString, Variable> &memory);

private:
    // --- PERSISTENT STATE ---
    const int MAX_STACK_DEPTH = 100;     // Anti-infinite recursion
    int m_currentLineIndex = 0;          // The "Instruction Pointer"
    QStringList m_scriptLines;           // The loaded script in memory
    QStack<int> m_loopStack;             // For lookback on iterative commands (WHILE)
    QMap<QString, int> m_functionMap;    // Store name -> line_index
    QStack<int> m_callStack;             // Store where to jump back to

    /**
     * @brief The main execution loop that iterates through m_scriptLines.
     */
    QString executeScript(QMap<QString, Variable> &memory);
    bool skipBlock(bool skipToEndIf);
    QString validateBlocks();

    // --- PHASE 1: LEXER ---
    QVector<Token> tokenize(QString input);
    Token::Type identifyType(const QString &text);

    // --- PHASE 2: PARSER ---
    Instruction parse(QVector<Token> tokens);

    // --- PHASE 3: EXECUTOR ---
    QString execute(Instruction inst, QMap<QString, Variable> &memory);
};

#endif // INTERPRETER_H
