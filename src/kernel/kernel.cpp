#include "kernel.h"

QMap<QString, Variable> Kernel::memory;

/**
 * @brief Handshake.
 * Passes raw script data to the Interpreter's process engine.
 */
QString Kernel::handleCommand(const QString &script) {
    if (script.trimmed().isEmpty()) {
        return "";
    }

    // The Kernel delegates all logic to the Interpreter (The Hub).
    return m_interpreter.process(script.trimmed(), memory);
}

/**
 * @brief Resumption.
 * Feeds user-provided data back into the system to unblock execution.
 */
QString Kernel::resumeScript(const QString &value, const QString &varName) {
    // Updates memory and moves the instruction pointer forward.
    return m_interpreter.resume(value, varName, memory);
}
