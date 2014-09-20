QT += core gui widgets network concurrent
TARGET = launcher
TEMPLATE = app
CONFIG += c++11
INCLUDEPATH += src $$(BOOST_ROOT)

LIBS += $$(BOOST_ROOT)/stage/lib/libboost_iostreams-mgw48-mt-1_56.a
LIBS += $$(BOOST_ROOT)/stage/lib/libboost_bzip2-mgw48-mt-1_56.a

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

FORMS += Launcher.ui

RESOURCES += app.qrc
