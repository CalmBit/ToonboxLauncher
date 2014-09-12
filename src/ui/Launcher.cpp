#include "Launcher.h"
#include "ui_Launcher.h"

#include <QMainWindow>

Launcher::Launcher(QWidget *parent) : QMainWindow(parent), ui(new Ui::Launcher)
{
    ui->setupUi(this);

    // Window properties:
    this->setWindowFlags(Qt::FramelessWindowHint);
    this->setAttribute(Qt::WA_TranslucentBackground);
}

Launcher::~Launcher()
{
    delete ui;
}
