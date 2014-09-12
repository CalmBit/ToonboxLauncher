#pragma once
#include <QMainWindow>

namespace Ui {
class Launcher;
}

class Launcher : public QMainWindow
{
        Q_OBJECT

    public:
        explicit Launcher(QWidget *parent = 0);
        ~Launcher();

    private:
        Ui::Launcher *ui;
};
