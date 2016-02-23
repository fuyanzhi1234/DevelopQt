#ifndef NEWCEFWIN_H
#define NEWCEFWIN_H

#include <QDialog>
#include "ui_newcefwin.h"

class NewCefWin : public QDialog
{
	Q_OBJECT

public:
	NewCefWin(QWidget *parent = 0);
	~NewCefWin();

	static NewCefWin *getInstance();

private:
	Ui::NewCefWin ui;
	static NewCefWin *instance;
};

#endif // NEWCEFWIN_H
