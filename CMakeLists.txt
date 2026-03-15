cmake_minimum_required(VERSION 3.16)

project(SudoVM VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Quick)

qt_standard_project_setup(REQUIRES 6.8)

qt_add_executable(appSudoVM
    main.cpp
)

qt_add_qml_module(appSudoVM
    URI SudoVM
    QML_FILES
        Main.qml
        elements/core_screens/Splash.qml
        elements/core_screens/Desktop.qml

        elements/core_components/BaseWindow.qml

        elements/core_apps/Terminal.qml

    RESOURCES
        assets/textures/personalization/theme/dark/logo/default_dark.png
        assets/textures/personalization/theme/dark/backgrounds/default_dark.png
        assets/textures/personalization/theme/dark/icons/maximize.png
        assets/textures/personalization/theme/dark/icons/minimize.png
        assets/textures/personalization/theme/dark/icons/quit.png
        assets/textures/personalization/theme/dark/icons/terminal.png

        assets/textures/personalization/theme/light/logo/default_light.png
        assets/textures/personalization/theme/light/backgrounds/default_light.png
        assets/textures/personalization/theme/light/icons/maximize.png
        assets/textures/personalization/theme/light/icons/minimize.png
        assets/textures/personalization/theme/light/icons/quit.png
        assets/textures/personalization/theme/light/icons/terminal.png

    SOURCES
        src/kernel/kernel.h src/kernel/kernel.cpp
        src/interpreter/interpreter.h src/interpreter/interpreter.cpp
        src/interpreter/core/core.h src/interpreter/core/core.cpp
        src/definitions.h src/definitions.cpp
        src/interpreter/info/info.h src/interpreter/info/info.cpp
        src/interpreter/math/math.h src/interpreter/math/math.cpp
        src/bridges/terminal/terminalbridge.h src/bridges/terminal/terminalbridge.cpp
        SOURCES src/interpreter/logic/logic.h src/interpreter/logic/logic.cpp
)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
set_target_properties(appSudoVM PROPERTIES
#    MACOSX_BUNDLE_GUI_IDENTIFIER com.example.appSudo_OS
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

target_link_libraries(appSudoVM
    PRIVATE Qt6::Quick
)

include(GNUInstallDirs)
install(TARGETS appSudoVM
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
