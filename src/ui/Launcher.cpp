#include "Launcher.h"
#include "ui_Launcher.h"

#include "core/constants.h"

#include "patcher/Patcher.h"

#include <QWidget>
#include <QMainWindow>
#include <QPoint>
#include <QMouseEvent>
#include <QString>
#include <QDesktopServices>
#include <QUrl>

Launcher::Launcher(QWidget *parent) : QMainWindow(parent),
    ui(new Ui::Launcher), patcher(new Patcher(DOWNLOAD_SERVER_URL.c_str())),
    m_captured(false), m_last_pos(QPoint(0, 0))
{
    ui->setupUi(this);

    // Window properties:
    this->setWindowFlags(Qt::FramelessWindowHint);
    this->setAttribute(Qt::WA_TranslucentBackground);

    // Put focus on the username line edit:
    ui->line_edit_username->setFocus();
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

void Launcher::on_push_button_minimize_clicked()
{
    this->setWindowState(Qt::WindowMinimized);
}

void Launcher::on_push_button_close_clicked()
{
    this->close();
}

void Launcher::on_push_button_home_page_clicked()
{
    QString url(HOME_PAGE_URL.c_str());
    QDesktopServices::openUrl(QUrl(url));
}

void Launcher::on_push_button_report_a_bug_clicked()
{
    QString url(REPORT_A_BUG_URL.c_str());
    QDesktopServices::openUrl(QUrl(url));
}

void Launcher::on_line_edit_username_returnPressed()
{
    if(ui->line_edit_username->text().length() > 0)
    {
        ui->line_edit_password->setFocus();
    }
}

void Launcher::on_line_edit_password_returnPressed()
{
    if(ui->line_edit_password->text().length() < 1)
    {
        return;
    }
    if(ui->line_edit_username->text().length() < 1)
    {
        ui->line_edit_username->setFocus();
    }
    else
    {
        // TODO: Start the login thread.
    }
}
