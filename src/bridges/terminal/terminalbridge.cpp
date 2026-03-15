#include "terminalbridge.h"

/**
 * @brief Orchestrates the flow between QML input and Kernel execution.
 */
void TerminalBridge::sendCommand(const QString &input) {
    QString result;
    bool wasCleared = false; // Local flag for this execution

    if (m_isWaiting) {
        QString currentVar = m_waitingForVar;
        if (!m_log.isEmpty() && m_log.last().startsWith("INPUT_REQUEST:")) {
            m_log.removeLast();
            m_log.append("[HISTORY]Input " + currentVar + ": " + input);
        }
        m_isWaiting = false;
        m_waitingForVar = "";
        result = m_kernel.resumeScript(input, currentVar);
    } else {
        result = m_kernel.handleCommand(input);
    }

    // Process results and signals
    QStringList lines = result.split("\n", Qt::SkipEmptyParts);
    for (const QString& line : std::as_const(lines)) {
        QString cleanLine = line.trimmed();

        if (cleanLine == "[SIGNAL_CLEAR_LOG]") {
            m_log.clear();
            wasCleared = true; // Mark that we shouldn't show "Finished"
        } else if (cleanLine.startsWith("[SIGNAL_INPUT_REQUIRED]")) {
            m_waitingForVar = cleanLine.split(":")[1];
            m_isWaiting = true;
            m_log.append("INPUT_REQUEST:" + m_waitingForVar);
        } else {
            m_log.append("> " + line);
        }
    }

    // ALWAYS add the finished message unless we cleared the screen
    // or we are currently paused waiting for more input.
    if (!wasCleared && !m_isWaiting) {
        m_log.append("> Program Finished...");
    }

    emit isWaitingChanged();
    emit logChanged();
}
