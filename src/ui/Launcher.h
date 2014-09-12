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

    private:
        Ui::Launcher *ui;

        bool m_captured;
        QPoint m_last_pos;
};
