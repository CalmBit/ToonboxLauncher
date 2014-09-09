#include "ui/MainWindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
	QApplication application(argc, argv);

	MainWindow *main_window = new MainWindow;
	main_window->show();

	return application.exec();
}
