QT += core gui widgets
TARGET = launcher
TEMPLATE = app
INCLUDEPATH += src

SOURCES += \
    src/core/main.cpp \
    src/ui/Launcher.cpp

HEADERS += \
    src/ui/Launcher.h \
    src/core/constants.h

FORMS += Launcher.ui

RESOURCES += \
    app.qrc
