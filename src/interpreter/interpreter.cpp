#include "interpreter.h"
#include "core/core.h"
#include "info/info.h"
#include "math/math.h"
#include "logic/logic.h"

/**
 * @brief Entry point for script execution.
 * Splits raw text into lines and performs a structural validation pass.
 */
QString Interpreter::process(QString rawInput, QMap<QString, Variable> &memory) {
    // 1. Load the script into our persistent state
    m_scriptLines = rawInput.split(QRegularExpression("[\r\n]+"), Qt::SkipEmptyParts);
    m_currentLineIndex = 0;

    if (m_scriptLines.isEmpty()) return "";

    // 2. VALIDATION PASS: Ensure all IF/WHILE blocks are balanced before executing.
    QString validationRes = validateBlocks();
    if (validationRes != "[OK]") {
        return validationRes;
    }

    return executeScript(memory);
}

/**
 * @brief The Core Loop.
 * Manages the "Instruction Pointer" (m_currentLineIndex).
 */
QString Interpreter::executeScript(QMap<QString, Variable> &memory) {
    QStringList outputLog;

    while (m_currentLineIndex < m_scriptLines.size()) {
        QString trimmedLine = m_scriptLines[m_currentLineIndex].trimmed();

        // Skip empty lines or comments starting with '#'
        if (trimmedLine.isEmpty() || trimmedLine.startsWith("#")) {
            m_currentLineIndex++;
            continue;
        }

        auto tokens = tokenize(trimmedLine);
        auto command = parse(tokens);

        // If the Parser flagged the instruction as invalid (Case mismatch, wrong syntax, etc.)
        if (!command.isValid) {
            outputLog.append("[ERROR] Line " + QString::number(m_currentLineIndex + 1) + ": Invalid syntax or case violation.");
            m_currentLineIndex++;
            continue;
        }

        if (command.action == CMD_IF) {
            // 1. Strict Existence Check
            if (!memory.contains(command.name)) {
                return "[ERROR] Line " + QString::number(m_currentLineIndex + 1) +
                       ": IF condition variable '" + command.name + "' not found.";
            }

            // 2. Strict Type Check (Optional but recommended)
            if (memory[command.name].type != "BOOL") {
                return "[ERROR] Line " + QString::number(m_currentLineIndex + 1) +
                       ": IF requires a BOOL variable. '" + command.name + "' is " + memory[command.name].type;
            }

            bool condition = (memory[command.name].value == "true");
            if (!condition) {
                if (!skipBlock(false)) {
                    return "[ERROR] Line " + QString::number(m_currentLineIndex + 1) + ": IF block never closed.";
                }
            }
            m_currentLineIndex++;
            continue;
        }
        else if (command.action == CMD_ELSE) {
            // If we hit ELSE manually, it means we just finished the 'True' part
            // of an IF. We must now skip the 'False' part until we find ENDIF.
            if (!skipBlock(true)) {
                outputLog.append("[ERROR] Line " + QString::number(m_currentLineIndex + 1) + ": ELSE without ENDIF.");
                break;
            }
            m_currentLineIndex++;
            continue;
        }
        else if (command.action == CMD_ENDIF) {
            // ADD THIS: Simply consume the token and move to the next line.
            m_currentLineIndex++;
            continue;
        }
        else if (command.action == CMD_WHILE) {
            bool condition = (memory.contains(command.name) && memory[command.name].value == "true");
            if (condition) {
                m_loopStack.push(m_currentLineIndex);
                m_currentLineIndex++;
            } else {
                // SAFETY: Catch unclosed WHILE blocks
                if (!skipBlock(false)) {
                    outputLog.append("[ERROR] Line " + QString::number(m_currentLineIndex + 1) + ": WHILE block never closed.");
                    break;
                }
                m_currentLineIndex++;
            }
            continue;
        }
        else if (command.action == CMD_ENDWHILE) {
            // SAFETY: Check for Orphaned ENDWHILE
            if (m_loopStack.isEmpty()) {
                outputLog.append("[ERROR] Line " + QString::number(m_currentLineIndex + 1) + ": ENDWHILE found without a matching WHILE.");
                m_currentLineIndex++;
            } else {
                m_currentLineIndex = m_loopStack.pop();
            }
            continue;
        }
        QString res = execute(command, memory);

        if (res.startsWith("[SIGNAL_INPUT_REQUIRED]")) {
            QString previousOutputs = outputLog.join("\n");
            if (!previousOutputs.isEmpty()) {
                // Ensure there is a clean newline between output and signal
                return previousOutputs + "\n" + res;
            }
            return res;
        }

        // Standard logging logic
        bool isError = res.startsWith("[ERROR]");
        bool isExplicitOutput = (command.action == CMD_OUTPUT ||
                                 command.action == CMD_READ   ||
                                 command.action == CMD_HELP   ||
                                 command.action == CMD_CLEAR);

        if (isError || isExplicitOutput) {
            outputLog.append(res);
        }

        m_currentLineIndex++;
    }

    return outputLog.join("\n");
}

/**
 * @brief Performs a pre-execution scan to ensure all control blocks are balanced.
 * This prevents partial execution of malformed scripts.
 */
QString Interpreter::validateBlocks() {
    QStack<QString> blockStack;

    for (int i = 0; i < m_scriptLines.size(); ++i) {
        QString line = m_scriptLines[i].trimmed();
        if (line.isEmpty() || line.startsWith("#")) continue;

        auto tokens = tokenize(line);
        if (tokens.isEmpty()) continue;

        QString cmd = tokens[0].value.toUpper();

        // 1. Track Block Openings
        if (cmd == "IF" || cmd == "WHILE") {
            blockStack.push(cmd + " (Line " + QString::number(i + 1) + ")");
        }
        // 2. Validate Block Closings
        else if (cmd == "ENDIF") {
            if (blockStack.isEmpty() || !blockStack.top().startsWith("IF")) {
                return "[SYNTAX_ERROR] ENDIF without matching IF at line " + QString::number(i + 1);
            }
            blockStack.pop();
        }
        else if (cmd == "ENDWHILE") {
            if (blockStack.isEmpty() || !blockStack.top().startsWith("WHILE")) {
                return "[SYNTAX_ERROR] ENDWHILE without matching WHILE at line " + QString::number(i + 1);
            }
            blockStack.pop();
        }
    }

    // 3. Final Check: Did any blocks stay open?
    if (!blockStack.isEmpty()) {
        return "[SYNTAX_ERROR] Unclosed block detected: " + blockStack.top();
    }

    return "[OK]";
}

/**
 * @brief Skips lines until it finds a matching ELSE or ENDIF, useful for WHILE and ENDWHILE.
 * @param skipToEndIf If true, ignores ELSE and looks only for ENDIF, same for WHILE and ENDWHILE.
 */
bool Interpreter::skipBlock(bool skipToEndIf) {
    int nestingLevel = 0;
    m_currentLineIndex++; // Start looking from the next line

    while (m_currentLineIndex < m_scriptLines.size()) {
        QString line = m_scriptLines[m_currentLineIndex].trimmed();
        auto tokens = tokenize(line);
        if (tokens.isEmpty() || line.startsWith("#")) {
            m_currentLineIndex++;
            continue;
        }

        QString cmd = tokens[0].value.toUpper();

        // Track internal nesting so we don't exit early on a sub-block
        if (cmd == "IF" || cmd == "WHILE") {
            nestingLevel++;
        }
        else if (cmd == "ENDIF" || cmd == "ENDWHILE") {
            if (nestingLevel > 0) {
                nestingLevel--;
            } else {
                return true; // SUCCESS: Found the matching end
            }
        }
        else if (cmd == "ELSE" && nestingLevel == 0 && !skipToEndIf) {
            return true; // SUCCESS: Found the ELSE we were looking for
        }

        m_currentLineIndex++;
    }

    return false; // FAILURE: Reached end of script without finding the closing tag
}

/**
 * @brief Resumes a script after the UI provides input for an 'IN' command.
 */
QString Interpreter::resume(QString userInput, QString targetVar, QMap<QString, Variable> &memory) {
    // Create a virtual 'UPD' instruction
    Instruction updateCmd;
    updateCmd.action = CMD_UPDATE;
    updateCmd.category = CMD_VARIABLE;
    updateCmd.name = targetVar;
    updateCmd.value = userInput.trimmed();

    // Send it to Core (Level 0)
    QString res = Core::execute(updateCmd, memory);

    // If Core returns an [ERROR], it means validation failed (e.g., non-numeric in a NUM)
    if (res.startsWith("[ERROR]")) {
        // Simply pass the Core's specific error back to the user and re-request input
        return res + "\n[SIGNAL_INPUT_REQUIRED]:" + targetVar;
    }

    m_currentLineIndex++;
    return executeScript(memory);
}

// --- PHASE 1: LEXER ---

/**
 * @brief Classifies a text chunk into a Token Type.
 * Note: Lexer identifies the POSSIBLITY of a keyword using .toUpper(),
 * but the Parser enforces the STRICT CASE rule.
 */
Token::Type Interpreter::identifyType(const QString &text) {
    QString upperText = text.toUpper();

    // Check Maps
    if (commandMap.contains(upperText)) return Token::KEYWORD;
    if (typeMap.contains(upperText))    return Token::TYPE;
    if (dataTypeMap.contains(upperText)) return Token::DATA_TYPE;
    if (systemIdentifiersMap.contains(upperText)) return Token::IDENTIFIER;

    // Numeric Literals
    bool isNum;
    text.toDouble(&isNum);
    if (isNum) return Token::VALUE;

    // Valid Identifiers (Variables)
    static QRegularExpression identRegex("^[A-Za-z_][A-Za-z0-9_]*$");
    if (identRegex.match(text).hasMatch()) {
        return Token::IDENTIFIER;
    }

    return Token::END;
}

/**
 * @brief Tokenizes a line while respecting string literals (" ").
 */
QVector<Token> Interpreter::tokenize(QString input) {
    input = input.trimmed();
    QVector<Token> tokens;
    QString current;
    bool inQuotes = false;

    for (int i = 0; i < input.length(); ++i) {
        QChar c = input[i];

        if (c == '#' && !inQuotes) {
            if (!current.isEmpty()) {
                tokens.append({identifyType(current.trimmed()), current.trimmed()});
                current.clear();
            }
            break;
        }

        if (c == '\"') {
            if (inQuotes) {
                tokens.append({Token::STRING, current});
                current.clear();
            }
            inQuotes = !inQuotes;
            continue;
        }

        if (!inQuotes && c.isSpace()) {
            if (!current.isEmpty()) {
                tokens.append({identifyType(current), current});
                current.clear();
            }
            continue;
        }
        current.append(c);
    }

    if (inQuotes) {
        tokens.clear();
        tokens.append({Token::END, "[ERROR] Unclosed string literal."});
        return tokens;
    }

    if (!current.isEmpty()) {
        tokens.append({identifyType(current.trimmed()), current.trimmed()});
    }

    return tokens;
}

// --- PHASE 2: PARSER ---

/**
 * @brief The Logic Gate. Enforces strict casing for Keywords and reserved names for Identifiers.
 */
Instruction Interpreter::parse(QVector<Token> tokens) {
    Instruction inst;
    if (tokens.isEmpty()) return inst;

    // --- RULE: STRICT KEYWORD CASE ---
    QString rawVerb = tokens[0].value;
    QString upperVerb = rawVerb.toUpper();

    if (commandMap.contains(upperVerb)) {
        // ERROR: Commands must be strictly ALL CAPS (e.g., NEW is valid, new is not)
        if (rawVerb != upperVerb) {
            inst.isValid = false;
            return inst;
        }
        inst.action = commandMap.value(upperVerb);
    } else {
        inst.action = CMD_UNKNOWN;
    }

    // --- COMMAND STRUCTURES ---
    switch (inst.action) {
    case CMD_OUTPUT: // OUT [args...]
        if (tokens.size() >= 2) {
            for (int i = 1; i < tokens.size(); ++i) inst.arguments.append(tokens[i]);
            inst.isValid = true;
        }
        break;

    case CMD_CREATE: // NEW VAR [DATA_TYPE] [NAME] [VALUE]
        if (tokens.size() == 5) {
            QString varName = tokens[3].value;
            inst.category = typeMap.value(tokens[1].value.toUpper(), CMD_VARIABLE);
            inst.dataType = dataTypeMap.value(tokens[2].value.toUpper(), CMD_NUMBER);
            inst.name     = varName;
            inst.value    = tokens[4].value;
            inst.isValid  = true;
        }
        break;

    case CMD_UPDATE: // UPD VAR [NAME] [VALUE]
        if (tokens.size() == 4) {
            inst.category = typeMap.value(tokens[1].value.toUpper(), CMD_VARIABLE);
            inst.name = tokens[2].value;
            inst.value = tokens[3].value;
            inst.isValid = true;
        }
        break;

    case CMD_MATH: // MATH [OP] [TARGET] [ARGS...]
        if (tokens.size() >= 4) {
            inst.value = tokens[1].value.toUpper();
            inst.name = tokens[2].value;
            for (int i = 3; i < tokens.size(); ++i) inst.arguments.append(tokens[i]);
            inst.isValid = true;
        }
        break;

    case CMD_READ: // READ [NAME]
        if (tokens.size() == 2) {
            inst.name = tokens[1].value;
            inst.isValid = true;
        }
        break;

    case CMD_INPUT: // IN [NAME]
        if (tokens.size() == 2) {
            inst.name = tokens[1].value;
            inst.isValid = true;
        }
        break;

    case CMD_DELETE: // DEL [NAME1] [NAME2...]
        if (tokens.size() >= 2) {
            for (int i = 1; i < tokens.size(); ++i) inst.arguments.append(tokens[i]);
            inst.isValid = true;
        }
        break;

    case CMD_HELP: // HELP [KEYWORD] or HELP
        inst.isValid = true;
        if (tokens.size() == 2) inst.value = tokens[1].value.toUpper();
        break;

    case CMD_CLEAR:
        if (tokens.size() == 1) inst.isValid = true;;
        break;

    case CMD_LOGIC: // LGC [TYPE] [TARGET] [ARG 1] (optional [ARG 2])
        // Allow 4 tokens (for NOT) or 5 tokens (for EQ, AND, etc.)
        if (tokens.size() >= 4 && tokens.size() <= 5) {
            inst.value = tokens[1].value.toUpper(); // e.g., "EQ", "AND", "NOT"
            inst.name = tokens[2].value;           // The result variable
            for (int i = 3; i < tokens.size(); ++i) {
                inst.arguments.append(tokens[i]);
            }
            inst.isValid = true;
        }
        break;

    case CMD_IF: // IF [VARIABLE_NAME]
        if (tokens.size() == 2) {
            inst.name = tokens[1].value;
            inst.isValid = true;
        }
        break;

    case CMD_ELSE:
    case CMD_ENDIF:
        if (tokens.size() == 1) inst.isValid = true;
        break;

    case CMD_WHILE: // WHILE [VARIABLE_NAME]
        if (tokens.size() == 2) {
            inst.name = tokens[1].value;
            inst.isValid = true;
        }
        break;

    case CMD_ENDWHILE:
        if (tokens.size() == 1) inst.isValid = true;
        break;

    default:
        inst.isValid = false;
        break;
    }

    return inst;
}

// --- PHASE 3: EXECUTOR ---
/**
 * @brief Routing module that delegates logic to the specialized entity.
 */
QString Interpreter::execute(Instruction inst, QMap<QString, Variable> &memory) {
    if (!inst.isValid) return "[ERROR] Malformed command or case violation.";

    // Route logic.
    switch (inst.action) {
        case CMD_CREATE:
        case CMD_UPDATE:
        case CMD_DELETE:
            return Core::execute(inst, memory);
            break;

        case CMD_OUTPUT:
        case CMD_READ:
        case CMD_HELP:
        case CMD_INPUT:
        case CMD_CLEAR:
            return Info::execute(inst, memory);
            break;

        case CMD_MATH:
            return Math::execute(inst, memory);
            break;

        case CMD_LOGIC:
            return Logic::execute(inst, memory);
            break;

        case CMD_UNKNOWN:
            return "[ERROR] For developer. Command non-existent.";
            break;
    }

    return "[ERROR] Cannot find command.";
}
