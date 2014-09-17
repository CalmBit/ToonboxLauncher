QT += core gui widgets network
TARGET = launcher
TEMPLATE = app
CONFIG += c++11
INCLUDEPATH += src dependencies

SOURCES += \
    src/core/main.cpp \
    src/ui/Launcher.cpp \
    src/patcher/Patcher.cpp \
    src/patcher/PatchFile.cpp \
    src/patcher/PatchDirectory.cpp \
    src/login/Authenticator.cpp

HEADERS += \
    src/core/constants.h \
    src/core/localizer.h \
    src/ui/Launcher.h \
    src/patcher/Patcher.h \
    src/patcher/PatchFile.h \
    src/patcher/PatchDirectory.h \
    src/login/Authenticator.h

FORMS += Launcher.ui

RESOURCES += app.qrc
