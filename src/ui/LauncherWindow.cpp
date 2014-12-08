#include "LauncherWindow.h"
#include "ui_Launcher.h"
#include "DraggableWindow.h"

#include "core/constants.h"
#include "core/localizer.h"
#include "updater/Updater.h"
#include "updater/ManifestDirectory.h"
#include "updater/ManifestFile.h"
#include "login/Authenticator.h"

#include <queue>
#include <vector>

#include <QWidget>
#include <Qt>
#include <QString>
#include <QStringList>
#include <QMessageBox>
#include <QObject>
#include <QDir>
#include <QFile>
#include <QIODevice>
#include <QByteArray>
#include <QCryptographicHash>
#include <QtGlobal>
#include <Qt>
#include <QDesktopServices>
#include <QProcess>

LauncherWindow::LauncherWindow(QWidget *parent) : DraggableWindow(parent),
    m_ui(new Ui::Launcher), m_authenticator(new Authenticator(URL_LOGIN_ENDPOINT)),
    m_updater(new Updater(URL_DOWNLOAD_MIRROR))
{
    m_ui->setupUi(this);

    // Get up to date with the manifest:
    this->update_manifest();

    // Remove the window's border, and make the background color translucent:
    this->setWindowFlags(Qt::FramelessWindowHint);
    this->setAttribute(Qt::WA_TranslucentBackground);

    // For ease of logging in, put focus on the username line edit:
    m_ui->line_edit_username->setFocus();
}

LauncherWindow::~LauncherWindow()
{
    delete m_updater;
    delete m_authenticator;
    delete m_ui;
}

void LauncherWindow::update_manifest()
{
    m_updater->update_manifest();

    // Update the version labels:
    QString server_version = m_updater->get_server_version();
    QString launcher_version = m_updater->get_launcher_version();
    if(server_version.isEmpty()) {
        m_ui->label_server_version->setText(GUI_NO_VERSION);
    } else {
        m_ui->label_server_version->setText(server_version);
    }
    if(launcher_version.isEmpty()) {
        m_ui->label_launcher_version->setText(GUI_NO_VERSION);
    } else {
        m_ui->label_launcher_version->setText(launcher_version);
    }

    // Ensure our launcher is at the latest version:
    if(!launcher_version.isEmpty() && (launcher_version != VERSION)) {
        QMessageBox message_box;
        message_box.setWindowTitle(ERROR_OUT_OF_DATE_TITLE);
        message_box.setText(ERROR_OUT_OF_DATE_TEXT);
        message_box.setIcon(QMessageBox::Critical);
        message_box.setStandardButtons(QMessageBox::Cancel);
        message_box.exec();

        exit(1);
    }
}

bool LauncherWindow::update_game() {
    // Connect the required slots:
    QObject::connect(m_updater, SIGNAL(download_error(int, const QString &)),
                     this, SLOT(download_error(int, const QString &)));
    QObject::connect(m_updater, SIGNAL(download_progress(qint64, qint64, const QString &)),
                     this, SLOT(download_progress(qint64, qint64, const QString &)));

    // Begin downloading the updated files:
    return m_updater->update();
}

void LauncherWindow::launch_game(const QString &login_token)
{
    // Let the user know we're starting the game up:
    m_ui->label_status->setText(GUI_STARTING_GAME);

    // Set the necessary environment variables:
    _putenv(("TTI_PLAYCOOKIE=" + login_token).toStdString().c_str());
    _putenv(("TTI_GAMESERVER=" + m_updater->get_client_agent()).toStdString().c_str());

    // Start the runtime application:
    QStringList arguments;
    arguments.append("--play-token");
    arguments.append(login_token);
    arguments.append(m_updater->get_client_agent());
    QProcess::startDetached("infinite.exe", arguments);

    // We've served our purpose. Have fun!
    this->close();
}

void LauncherWindow::on_push_button_close_clicked()
{
    this->close();
}

void LauncherWindow::on_push_button_minimize_clicked()
{
    this->setWindowState(Qt::WindowMinimized);
}

void LauncherWindow::on_push_button_play_clicked()
{
    if(m_ui->line_edit_username->text().isEmpty() &&
       m_ui->line_edit_password->text().isEmpty()) {
        return;
    }

    // Disable the launcher's login, and content packs functionality so that
    // nothing unexpected happens while we work:
    m_ui->push_button_play->setEnabled(false);
    m_ui->line_edit_username->setEnabled(false);
    m_ui->line_edit_password->setEnabled(false);
    // TODO: Disable the content packs button.

    // Let the user know we're working:
    m_ui->label_status->setText(GUI_LOGIN_WAITING);

    // Get up to date with the manifest:
    this->update_manifest();

    // Verify the account credentials:
    LoginReply login_reply = m_authenticator->login(
                m_ui->line_edit_username->text(),
                m_ui->line_edit_password->text());
    if(!login_reply.success) {
        // Update the status label with our error response:
        m_ui->label_status->setText(
            QString::number(login_reply.error_code) + ": " +
            login_reply.data);

        // Re-enable the launcher's login, and content packs functionality:
        m_ui->push_button_play->setEnabled(true);
        m_ui->line_edit_username->setEnabled(true);
        m_ui->line_edit_password->setEnabled(true);
        // TODO: Re-enable the content packs button.

        // For ease of logging in, put focus on the username line edit:
        m_ui->line_edit_username->setFocus();

        return;
    }

    // Alright, the login was a success. Begin the update process:
    if(!this->update_game()) {
        return;
    }
}

void LauncherWindow::on_push_button_report_a_bug_clicked()
{
    QDesktopServices::openUrl(QUrl(URL_REPORT_A_BUG));
}

void LauncherWindow::on_push_button_home_page_clicked()
{
    QDesktopServices::openUrl(QUrl(URL_HOME_PAGE));
}

void LauncherWindow::on_line_edit_username_returnPressed()
{
    if(!m_ui->line_edit_username->text().isEmpty()) {
        m_ui->line_edit_password->setFocus();
    }
}

void LauncherWindow::on_line_edit_password_returnPressed()
{
    if(m_ui->line_edit_password->text().isEmpty()) {
        return;
    }
    if(m_ui->line_edit_username->text().isEmpty()) {
        m_ui->line_edit_username->setFocus();
    } else if(m_ui->push_button_play->isEnabled()) {
        this->on_push_button_play_clicked();
    }
}

void LauncherWindow::download_error(int error_code, const QString &error_string)
{
    m_ui->label_status->setText(QString::number(error_code) + ": " + error_string);
}

void LauncherWindow::download_progress(qint64 bytes_read, qint64 bytes_total,
                                       const QString &status)
{
    m_ui->label_status->setText(status);
    m_ui->progress_bar->setMaximum(bytes_total);
    m_ui->progress_bar->setValue(bytes_read);
}
