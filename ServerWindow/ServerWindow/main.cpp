#include "serverwindow.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	ServerWindow w;
	w.show();
	return a.exec();
}