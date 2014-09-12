#include "Launcher.h"
#include "ui_Launcher.h"

#include <QMainWindow>

Launcher::Launcher(QWidget *parent) : QMainWindow(parent), ui(new Ui::Launcher)
{
    ui->setupUi(this);
}

Launcher::~Launcher()
{
    delete ui;
}
