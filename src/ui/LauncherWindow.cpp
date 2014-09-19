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
#include <QMessageBox>
#include <QDir>
#include <QFile>
#include <QIODevice>
#include <QByteArray>
#include <QCryptographicHash>
#include <QDesktopServices>

LauncherWindow::LauncherWindow(QWidget *parent) : DraggableWindow(parent),
    m_ui(new Ui::Launcher), m_authenticator(new Authenticator(URL_LOGIN_ENDPOINT)),
    m_updater(new Updater(URL_DOWNLOAD_SERVER))
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
    m_updater->update_manifest(DISTRIBUTION_TOKEN);

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

    // Ensure our launcher is up to date:
    if(!launcher_version.isEmpty() && (launcher_version != VERSION)) {
        QMessageBox message_box;
        message_box.setWindowTitle(ERROR_UPDATE_LAUNCHER_TITLE);
        message_box.setText(ERROR_UPDATE_LAUNCHER_TEXT);
        message_box.setIcon(QMessageBox::Critical);
        message_box.setStandardButtons(QMessageBox::Cancel);
        message_box.exec();

        exit(1);
    }
}

void LauncherWindow::login()
{
    // Disable the launcher's login, and content packs functionality so nothing
    // unexpected happens while we work:
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
        m_ui->line_edit_username->text(), m_ui->line_edit_password->text(),
        DISTRIBUTION);
    if(!login_reply.success) {
        // Update the status label with our error response:
        m_ui->label_status->setText(QString::number(login_reply.error_code) + ": " +
                                    login_reply.response);

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
    this->update_files();

    // Everything is up to date. Launch the game:
    this->launch_game();
}

void LauncherWindow::update_files()
{
    // Build a queue of files to be updated:
    std::queue<QString> file_queue;

    QDir directory = QDir::current();

    std::vector<ManifestDirectory> directories = m_updater->get_directories();
    for(auto it = directories.begin(); it != directories.end(); ++it) {
        QString directory_name = it->get_name();
        if(!directory_name.isEmpty()) {
            // Create the directory if it doesn't exist:
            if(!directory.exists(directory_name)) {
                directory.mkdir(directory_name);
            }

            // Move into the directory:
            directory.cd(directory_name);
        }

        std::vector<ManifestFile> files = it->get_files();
        for(auto it2 = files.begin(); it2 != files.end(); ++it2) {
            QString file_name = it2->get_name();
            QString absolute_path = directory.absoluteFilePath(file_name);
            QString relative_path = QDir::current().relativeFilePath(absolute_path);

            if(!directory.exists(file_name)) {
                file_queue.push(relative_path);
                continue;
            }

            QFile file(absolute_path);
            if(!file.open(QIODevice::ReadOnly)) {
                file_queue.push(relative_path);
                continue;
            }

            if((file.size() != it2->get_size()) ||
               (QCryptographicHash::hash(
                    file.readAll(), QCryptographicHash::Md5) != it2->get_hash())) {
                file_queue.push(relative_path);
            }

            file.close();
        }

        if(!directory_name.isEmpty()) {
            // Move out of the directory:
            directory.cdUp();
        }
    }
}

void LauncherWindow::launch_game()
{
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
    if(!m_ui->line_edit_username->text().isEmpty() &&
       !m_ui->line_edit_password->text().isEmpty()) {
        this->login();
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
        this->login();
    }
}
