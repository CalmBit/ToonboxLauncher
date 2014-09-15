#include "Launcher.h"
#include "ui_Launcher.h"

#include "core/constants.h"
#include "patcher/Patcher.h"
#include "login/Authenticator.h"

#include <QWidget>
#include <QMainWindow>
#include <QPoint>
#include <QMessageBox>
#include <QMouseEvent>
#include <QString>
#include <QDesktopServices>
#include <QUrl>

Launcher::Launcher(QWidget *parent) : QMainWindow(parent),
    ui(new Ui::Launcher), patcher(new Patcher(URL_DOWNLOAD_SERVER)),
    authenticator(new Authenticator(URL_ACCOUNT_SERVER_ENDPOINT)),
    m_captured(false), m_last_pos(QPoint(0, 0))
{
    ui->setupUi(this);

    // Get up to date with the manifest:
    this->update_manifest();

    // Remove the window's border, and make the background color translucent:
    this->setWindowFlags(Qt::FramelessWindowHint);
    this->setAttribute(Qt::WA_TranslucentBackground);

    // For ease of logging in, put focus on the username line edit:
    ui->line_edit_username->setFocus();
}

Launcher::~Launcher()
{
    delete ui;
    delete patcher;
    delete authenticator;
}

void Launcher::update_manifest()
{
    patcher->update_manifest(DISTRIBUTION_TOKEN);

    // Update the version labels:
    QString server_version = patcher->get_server_version();
    QString launcher_version = patcher->get_launcher_version();
    if(server_version.isEmpty())
    {
        ui->label_server_version->setText(STRING_NO_VERSION);
    }
    else
    {
        ui->label_server_version->setText(server_version);
    }
    if(launcher_version.isEmpty())
    {
        ui->label_launcher_version->setText(STRING_NO_VERSION);
    }
    else
    {
        ui->label_launcher_version->setText(launcher_version);
    }

    // Ensure our launcher is up to date:
    if(!launcher_version.isEmpty() && (launcher_version != VERSION))
    {
        QMessageBox message_box_out_of_date;
        message_box_out_of_date.setWindowTitle(STRING_OUT_OF_DATE_TITLE);
        message_box_out_of_date.setText(STRING_OUT_OF_DATE_MESSAGE);
        message_box_out_of_date.setIcon(QMessageBox::Critical);
        message_box_out_of_date.setStandardButtons(QMessageBox::Cancel);
        message_box_out_of_date.exec();

        exit(1);
    }
}

void Launcher::launch()
{
    // Disable the launcher's login, and content packs functionality:
    ui->push_button_play->setEnabled(false);
    ui->line_edit_username->setEnabled(false);
    ui->line_edit_password->setEnabled(false);
    // TODO: Disable the content packs button.

    // Let the user know we're working:
    ui->label_status->setText(STRING_LOGIN_WAITING);

    // Get up to date with the manifest:
    this->update_manifest();

    // Verify the account credentials:
    LoginReply login_reply = authenticator->login(
        ui->line_edit_username->text(), ui->line_edit_password->text(),
        DISTRIBUTION);

    if(!login_reply.success)
    {
        // Update the status label with our error response:
        ui->label_status->setText(QString::number(login_reply.error_code) + ": " + login_reply.response);

        // Enable the launcher's login, and content packs functionality:
        ui->push_button_play->setEnabled(true);
        ui->line_edit_username->setEnabled(true);
        ui->line_edit_password->setEnabled(true);
        // TODO: Enable the content packs button.

        // For ease of logging in, put focus on the username line edit:
        ui->line_edit_username->setFocus();

        return;
    }

    // Alright, begin the patching process:
    this->patch();
}

void Launcher::patch()
{
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
    QString url(URL_HOME_PAGE);
    QDesktopServices::openUrl(QUrl(url));
}

void Launcher::on_push_button_report_a_bug_clicked()
{
    QString url(URL_REPORT_A_BUG);
    QDesktopServices::openUrl(QUrl(url));
}

void Launcher::on_push_button_play_clicked()
{
    if(!ui->line_edit_username->text().isEmpty() &&
       !ui->line_edit_password->text().isEmpty())
    {
        this->launch();
    }
}

void Launcher::on_line_edit_username_returnPressed()
{
    if(!ui->line_edit_username->text().isEmpty())
    {
        ui->line_edit_password->setFocus();
    }
}

void Launcher::on_line_edit_password_returnPressed()
{
    if(ui->line_edit_password->text().isEmpty())
    {
        return;
    }
    if(ui->line_edit_username->text().isEmpty())
    {
        ui->line_edit_username->setFocus();
    }
    else
    {
        if(ui->push_button_play->isEnabled())
        {
            this->launch();
        }
    }
}
