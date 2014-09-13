QT += core gui widgets network
TARGET = launcher
TEMPLATE = app
INCLUDEPATH += src dependencies

SOURCES += \
    src/core/main.cpp \
    src/ui/Launcher.cpp \
    src/patcher/Patcher.cpp \
    src/login/Authenticator.cpp

HEADERS += \
    src/ui/Launcher.h \
    src/core/constants.h \
    src/patcher/Patcher.h \
    src/login/Authenticator.h

FORMS += Launcher.ui

RESOURCES += \
    app.qrc
