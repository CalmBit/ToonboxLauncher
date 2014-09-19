#include "ui/LauncherWindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    LauncherWindow *launcher_window = new LauncherWindow;
    launcher_window->show();

    return app.exec();
}
