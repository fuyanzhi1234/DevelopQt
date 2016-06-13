#ifndef QTAVSIMPLE_H
#define QTAVSIMPLE_H

#include <QtWidgets/QMainWindow>
#include "ui_qtavsimple.h"
#include <QtAV\QtAV>
#include <QtAVWidgets/QtAVWidgets>

class QtAVSimple : public QMainWindow
{
	Q_OBJECT

public:
	QtAVSimple(QWidget *parent = 0);
	~QtAVSimple();

private:
	Ui::QtAVSimpleClass ui;

	QtAV::WidgetRenderer *m_renderer;
	// ÊÓÆµ½âÂëÄÚºË
	QtAV::AVPlayer *m_mediaPlayer;

};

#endif // QTAVSIMPLE_H
