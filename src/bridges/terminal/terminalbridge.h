#ifndef TERMINALBRIDGE_H
#define TERMINALBRIDGE_H

#include <QObject>
#include <QStringList>
#include "../../kernel/kernel.h"

/**
 * @class TerminalBridge
 * @brief UI-System bridge.
 * * This class acts as the "Face" of the OS. It is the only C++ object exposed
 * to QML. It manages the terminal's visual log and handles the state-machine
 * transitions between active execution and waiting for user input.
 */
class TerminalBridge : public QObject {
    Q_OBJECT
    Q_PROPERTY(QStringList terminalLog READ terminalLog NOTIFY logChanged)
    Q_PROPERTY(bool isWaiting READ isWaiting NOTIFY isWaitingChanged)

public:
    Q_INVOKABLE void sendCommand(const QString &input);

    // Getters
    QStringList terminalLog() const { return m_log; }
    bool isWaiting() const { return m_isWaiting; }

signals:
    void logChanged();
    void isWaitingChanged();

private:
    Kernel m_kernel;
    QStringList m_log;
    bool m_isWaiting = false;
    QString m_waitingForVar;
};
#endif // TERMINALBRIDGE_H
