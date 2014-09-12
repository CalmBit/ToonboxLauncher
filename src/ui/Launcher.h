#pragma once
#include <QMainWindow>
#include <QMouseEvent>
#include <QPoint>

namespace Ui {
class Launcher;
}

class Launcher : public QMainWindow
{
        Q_OBJECT

    public:
        explicit Launcher(QWidget *parent = 0);
        ~Launcher();

    protected:
        void mousePressEvent(QMouseEvent *event);
        void mouseMoveEvent(QMouseEvent *event);
        void mouseReleaseEvent(QMouseEvent *event);

private slots:
        void on_push_button_close_clicked();

        void on_push_button_minimize_clicked();

        void on_line_edit_username_returnPressed();

        void on_line_edit_password_returnPressed();

private:
        Ui::Launcher *ui;

        bool m_captured;
        QPoint m_last_pos;
};
