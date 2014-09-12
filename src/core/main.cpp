#include "ui/Launcher.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    Launcher *launcher = new Launcher;
    launcher->show();

    return app.exec();
}
