#ifndef CLIENTWINDOW_H
#define CLIENTWINDOW_H

#include <QtWidgets/QMainWindow>
#include "ui_clientwindow.h"
#include <QLocalSocket>

class ClientWindow : public QMainWindow
{
	Q_OBJECT

public:
	ClientWindow(QWidget *parent, HWND hParent);
	~ClientWindow();

private slots:

void sendToServer();

void connectedToServer();

void disConnectedToServer();

void readFromServer();

void socketError(QAbstractSocket::SocketError);

private:
	Ui::ClientWindowClass ui;
	QLocalSocket *m_LocalSocket;

};

#endif // CLIENTWINDOW_H
