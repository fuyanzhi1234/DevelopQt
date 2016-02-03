#include "clientwindow.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);

	QString strHwnd;
	strHwnd.append(argv[1]);

	ClientWindow w(NULL, (HWND)strHwnd.toLong());
	w.show();
	return a.exec();
}
