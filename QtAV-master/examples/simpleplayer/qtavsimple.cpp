#include "qtavsimple.h"

QtAVSimple::QtAVSimple(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
	m_renderer = new QtAV::WidgetRenderer(this);
	m_mediaPlayer = new QtAV::AVPlayer(this);
	m_mediaPlayer->setRenderer(m_renderer);
	m_mediaPlayer->play("D://1.rmvb");
}

QtAVSimple::~QtAVSimple()
{

}
