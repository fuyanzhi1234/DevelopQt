#ifndef SERVERWINDOW_H
#define SERVERWINDOW_H

#include <QtWidgets/QMainWindow>
#include "ui_serverwindow.h"
#include <QLocalSocket>
#include <QLocalServer>
#include <QProcess>

class ServerWindow : public QMainWindow
{
	Q_OBJECT

public:
	ServerWindow(QWidget *parent = 0);
	~ServerWindow();

	private slots:
	void sendToClinet();
	void newClientConnetion();
	void readFromClient();
	void socketDisconnect();

private:
	Ui::ServerWindowClass ui;
	QLocalSocket *m_LocalSocket;
	QLocalServer *m_LocalServer;
	QProcess process;
};

#endif // SERVERWINDOW_H
