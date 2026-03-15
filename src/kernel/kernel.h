#ifndef KERNEL_H
#define KERNEL_H

#include <QObject>
#include "../interpreter/interpreter.h"

/**
 * @class Kernel
 * @brief System Root.
 * * The Kernel owns the "Brain" (Interpreter) and acts as the gatekeeper
 * between the UI Bridge and the Logic Rings. By housing the Interpreter here,
 * the variable memory persists as long as the OS process is alive.
 */
class Kernel : public QObject {
    Q_OBJECT
public:
    /**
     * @brief Initiates a new script execution session.
     * @param script Raw text from the terminal.
     * @return Execution logs or a [SIGNAL] for input.
     */
    QString handleCommand(const QString &script);

    /**
     * @brief Continues execution from a paused state (post-IN command).
     * @param value The user-provided data.
     * @param varName Target variable for the input data.
     * @return The remaining logs of the script.
     */
    QString resumeScript(const QString &value, const QString &varName);

private:
    // Level 1 Hub: The single persistent instance of the Brain.
    static QMap<QString, Variable> memory; // The Nucleus (Raw Data)
    Interpreter m_interpreter;
};

#endif // KERNEL_H
