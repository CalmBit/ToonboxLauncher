#pragma once
#include "ui/DraggableWindow.h"
#include "login/Authenticator.h"
#include "updater/Updater.h"

#include <QWidget>

namespace Ui
{
class Launcher;
}

class LauncherWindow : public DraggableWindow
{
    Q_OBJECT

  public:
    explicit LauncherWindow(QWidget *parent = 0);
    ~LauncherWindow();

    void update_manifest();
    void login();
    void update_files();
    void launch_game();

  private slots:
    void on_push_button_close_clicked();
    void on_push_button_minimize_clicked();
    void on_push_button_play_clicked();
    void on_push_button_report_a_bug_clicked();
    void on_push_button_home_page_clicked();

    void on_line_edit_username_returnPressed();
    void on_line_edit_password_returnPressed();

  private:
    Ui::Launcher *m_ui;
    Authenticator *m_authenticator;
    Updater *m_updater;
};
