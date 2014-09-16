QT += core gui widgets network
CONFIG += c++11
TARGET = launcher
TEMPLATE = app
INCLUDEPATH += src dependencies

SOURCES += \
    src/core/main.cpp \
    src/ui/Launcher.cpp \
    src/patcher/Patcher.cpp \
    src/login/Authenticator.cpp \
    src/patcher/PatchFile.cpp \
    src/patcher/PatchDirectory.cpp

HEADERS += \
    src/ui/Launcher.h \
    src/core/constants.h \
    src/patcher/Patcher.h \
    src/login/Authenticator.h \
    src/patcher/PatchDirectory.h \
    src/patcher/PatchFile.h

FORMS += Launcher.ui

RESOURCES += \
    app.qrc
