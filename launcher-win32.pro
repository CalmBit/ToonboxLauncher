QT += core gui widgets network concurrent
TARGET = launcher
TEMPLATE = app
CONFIG += c++11
INCLUDEPATH += src $$(BZIP2_PATH)

LIBS += $$(BZIP2_PATH)/libbz2.a

SOURCES += \
    src/core/main.cpp \
    src/ui/DraggableWindow.cpp \
    src/ui/LauncherWindow.cpp \
    src/login/Authenticator.cpp \
    src/updater/ManifestFile.cpp \
    src/updater/ManifestDirectory.cpp \
    src/updater/Updater.cpp

HEADERS += \
    src/core/constants.h \
    src/core/localizer.h \
    src/ui/DraggableWindow.h \
    src/ui/LauncherWindow.h \
    src/login/Authenticator.h \
    src/updater/ManifestFile.h \
    src/updater/ManifestDirectory.h \
    src/updater/Updater.h

win32:RC_ICONS += assets/ICON.ico

FORMS += Launcher.ui

RESOURCES += app.qrc
