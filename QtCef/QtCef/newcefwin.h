#ifndef NEWCEFWIN_H
#define NEWCEFWIN_H

#include <QWidget>
#include "ui_newcefwin.h"

class NewCefWin : public QWidget
{
	Q_OBJECT

public:
	NewCefWin(QWidget *parent = 0);
	~NewCefWin();

private:
	Ui::NewCefWin ui;
};

#endif // NEWCEFWIN_H
