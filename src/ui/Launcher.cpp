#include "Launcher.h"
#include "ui_Launcher.h"

#include <QMainWindow>
#include <QMouseEvent>

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

void Launcher::mousePressEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
    {
        m_last_pos = event->pos();
        m_captured = true;
    }
}

void Launcher::mouseMoveEvent(QMouseEvent *event)
{
    if(m_captured)
    {
        this->move(this->pos() + (event->pos()-m_last_pos));
    }
}

void Launcher::mouseReleaseEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
    {
        m_captured = false;
    }
}

void Launcher::on_push_button_close_clicked()
{
    this->close();
}
