#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "src/bridges/terminal/terminalbridge.h"

/**
 * @brief Entry point for Sudo_VM.
 * This file initializes the Qt application, loads the QML UI,
 * and injects the Kernel into the QML context.
 */
int main(int argc, char *argv[])
{
    // 1. Initialize the GUI application
    QGuiApplication app(argc, argv);

    // 2. Setup the QML Engine
    QQmlApplicationEngine engine;

    // Safety check: Exit if the QML fails to load (module naming must match CMake/Project file)
    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);

    // Instantiate the connections.
    TerminalBridge terminalBridge;

    /**
     * @brief Initiate connection
     * Registers the bridges to QML.
     */
    qmlRegisterType<TerminalBridge>("SudoVM.Core", 1, 0, "TerminalBridge");

    // 4. Load the Main QML file from the module
    engine.loadFromModule("Sudo_OS", "Main");

    // 5. Enter the main event loop
    return app.exec();
}
