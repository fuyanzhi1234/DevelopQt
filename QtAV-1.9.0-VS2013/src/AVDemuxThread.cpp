/******************************************************************************
    QtAV:  Media play library based on Qt and FFmpeg
    Copyright (C) 2012-2015 Wang Bin <wbsecg1@gmail.com>

*   This file is part of QtAV

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
******************************************************************************/

#include "AVDemuxThread.h"
#include <limits>
#include "QtAV/AVClock.h"
#include "QtAV/AVDemuxer.h"
#include "QtAV/AVDecoder.h"
#include "VideoThread.h"
#include <QtCore/QTime>
#include "utils/Logger.h"

#define RESUME_ONCE_ON_SEEK 0

namespace QtAV {

class QueueEmptyCall : public PacketBuffer::StateChangeCallback
{
public:
    QueueEmptyCall(AVDemuxThread* thread):
        mDemuxThread(thread)
    {}
    virtual void call() {
        if (!mDemuxThread)
            return;
        if (mDemuxThread->isEnd())
            return;
        mDemuxThread->updateBufferState(); // ensure detect buffering immediately
        AVThread *thread = mDemuxThread->videoThread();
        //qDebug("try wake up video queue");
        if (thread)
            thread->packetQueue()->blockFull(false);
        //qDebug("try wake up audio queue");
        thread = mDemuxThread->audioThread();
        if (thread)
            thread->packetQueue()->blockFull(false);
    }
private:
    AVDemuxThread *mDemuxThread;
};

AVDemuxThread::AVDemuxThread(QObject *parent) :
    QThread(parent)
  , paused(false)
  , user_paused(false)
  , end(false)
  , m_buffering(false)
  , m_buffer(0)
  , demuxer(0)
  , ademuxer(0)
  , audio_thread(0)
  , video_thread(0)
  , clock_type(-1)
{
    seek_tasks.setCapacity(1);
    seek_tasks.blockFull(false);
}

AVDemuxThread::AVDemuxThread(AVDemuxer *dmx, QObject *parent) :
    QThread(parent)
  , paused(false)
  , end(false)
  , m_buffering(false)
  , m_buffer(0)
  , audio_thread(0)
  , video_thread(0)
{
    setDemuxer(dmx);
    seek_tasks.setCapacity(1);
    seek_tasks.blockFull(false);
}

void AVDemuxThread::setDemuxer(AVDemuxer *dmx)
{
    demuxer = dmx;
}

void AVDemuxThread::setAudioDemuxer(AVDemuxer *demuxer)
{
    //QMutexLocker locker(&buffer_mutex);
    //Q_UNUSED(locker);
    ademuxer = demuxer;
}

void AVDemuxThread::setAVThread(AVThread*& pOld, AVThread *pNew)
{
    if (pOld == pNew)
        return;
    if (pOld) {
        if (pOld->isRunning())
            pOld->stop();
        pOld->disconnect(this, SLOT(onAVThreadQuit()));
    }
    pOld = pNew;
    if (!pNew)
        return;
    pOld->packetQueue()->setEmptyCallback(new QueueEmptyCall(this));
    connect(pOld, SIGNAL(finished()), SLOT(onAVThreadQuit()));
}

void AVDemuxThread::setAudioThread(AVThread *thread)
{
    setAVThread(audio_thread, thread);
}

void AVDemuxThread::setVideoThread(AVThread *thread)
{
    setAVThread(video_thread, thread);
}

AVThread* AVDemuxThread::videoThread()
{
    return video_thread;
}

AVThread* AVDemuxThread::audioThread()
{
    return audio_thread;
}

void AVDemuxThread::stepBackward()
{
    if (!video_thread)
        return;
    AVThread *t = video_thread;
    const qreal pre_pts = video_thread->previousHistoryPts();
    if (pre_pts == 0.0) {
        qWarning("can not get previous pts");
        return;
    }
    end = false;
    // queue maybe blocked by put()
    if (audio_thread) {
        audio_thread->packetQueue()->clear(); // will put new packets before task run
    }

    class stepBackwardTask : public QRunnable {
    public:
        stepBackwardTask(AVDemuxThread *dt, qreal t)
            : demux_thread(dt)
            , pts(t)
        {}
        void run() {
            AVThread *avt = demux_thread->videoThread();
            avt->packetQueue()->clear(); // clear here
            if (pts <= 0) {
                demux_thread->demuxer->seek(qint64(-pts*1000.0) - 1000LL);
                QVector<qreal> ts;
                qreal t = -1.0;
                while (t < -pts) {
                    demux_thread->demuxer->readFrame();
                    if (demux_thread->demuxer->stream() != demux_thread->demuxer->videoStream())
                        continue;
                    t = demux_thread->demuxer->packet().pts;
                    ts.push_back(t);
                }
                ts.pop_back();
                pts = ts.back();
                // FIXME: sometimes can not seek to the previous pts, the result pts is always current pts
            }
            qDebug("step backward: %lld, %f", qint64(pts*1000.0), pts);
            demux_thread->video_thread->setDropFrameOnSeek(false);
            demux_thread->seekInternal(qint64(pts*1000.0), AccurateSeek);
        }
    private:
        AVDemuxThread *demux_thread;
        qreal pts;
    };

    pause(true);
    // set clock first
    if (clock_type < 0)
        clock_type = (int)video_thread->clock()->isClockAuto() + 2*(int)video_thread->clock()->clockType();
    video_thread->clock()->setClockType(AVClock::VideoClock);
    t->packetQueue()->clear(); // will put new packets before task run
    t->packetQueue();
    Packet pkt;
    pkt.pts = pre_pts;
    t->packetQueue()->put(pkt); // clear and put a seek packet to ensure not frames other than previous frame will be decoded and rendered
    video_thread->pause(false);
    newSeekRequest(new stepBackwardTask(this, pre_pts));
}

void AVDemuxThread::seek(qint64 pos, SeekType type)
{
    end = false;
    // queue maybe blocked by put()
    if (audio_thread) {
        audio_thread->packetQueue()->clear();
    }
    if (video_thread) {
        video_thread->packetQueue()->clear();
    }
    class SeekTask : public QRunnable {
    public:
        SeekTask(AVDemuxThread *dt, qint64 t, SeekType st)
            : demux_thread(dt)
            , type(st)
            , position(t)
        {}
        void run() {
            if (demux_thread->video_thread)
                demux_thread->video_thread->setDropFrameOnSeek(true);
            demux_thread->seekInternal(position, type);
        }
    private:
        AVDemuxThread *demux_thread;
        SeekType type;
        qint64 position;
    };
    newSeekRequest(new SeekTask(this, pos, type));
}

void AVDemuxThread::seekInternal(qint64 pos, SeekType type)
{
    AVThread* av[] = { audio_thread, video_thread};
    qDebug("seek to %s %lld ms (%f%%)", QTime(0, 0, 0).addMSecs(pos).toString().toUtf8().constData(), pos, double(pos - demuxer->startTime())/double(demuxer->duration())*100.0);
    demuxer->setSeekType(type);
    demuxer->seek(pos);
    if (ademuxer) {
        ademuxer->setSeekType(type);
        ademuxer->seek(pos);
    }
    AVThread *watch_thread = 0;
    // TODO: why queue may not empty?
    for (size_t i = 0; i < sizeof(av)/sizeof(av[0]); ++i) {
        AVThread *t = av[i];
        if (!t)
            continue;
        t->packetQueue()->clear();
        // TODO: the first frame (key frame) will not be decoded correctly if flush() is called.
        //PacketBuffer *pb = t->packetQueue();
        //qDebug("%s put seek packet. %d/%d-%.3f, progress: %.3f", t->metaObject()->className(), pb->buffered(), pb->bufferValue(), pb->bufferMax(), pb->bufferProgress());
        t->packetQueue()->setBlocking(false); // aqueue bufferValue can be small (1), we can not put and take
        Packet pkt;
        pkt.pts = qreal(pos)/1000.0;
        t->packetQueue()->put(pkt);
        t->packetQueue()->setBlocking(true); // blockEmpty was false when eof is read.
        if (isPaused()) {
            t->pause(false);
            watch_thread = t;
        }
    }
    if (watch_thread) {
        pauseInternal(false);
        emit requestClockPause(false); // need direct connection
        // direct connection is fine here
        connect(watch_thread, SIGNAL(seekFinished(qint64)), this, SLOT(seekOnPauseFinished()), Qt::DirectConnection);
    }
}

void AVDemuxThread::newSeekRequest(QRunnable *r)
{
    if (seek_tasks.size() >= seek_tasks.capacity()) {
        QRunnable *r = seek_tasks.take();
        if (r && r->autoDelete())
            delete r;
    }
    seek_tasks.put(r);
}

void AVDemuxThread::processNextSeekTask()
{
    if (seek_tasks.isEmpty())
        return;
    QRunnable *task = seek_tasks.take();
    if (!task)
        return;
    task->run();
    if (task->autoDelete())
        delete task;
}

void AVDemuxThread::pauseInternal(bool value)
{
    paused = value;
}

bool AVDemuxThread::isPaused() const
{
    return paused;
}

bool AVDemuxThread::isEnd() const
{
    return end;
}

PacketBuffer* AVDemuxThread::buffer()
{
    return m_buffer;
}

void AVDemuxThread::updateBufferState()
{
    if (!m_buffer)
        return;
    if (m_buffering) { // always report progress when buffering
        Q_EMIT bufferProgressChanged(m_buffer->bufferProgress());
    }
    if (m_buffering == m_buffer->isBuffering())
        return;
    m_buffering = m_buffer->isBuffering();
    Q_EMIT mediaStatusChanged(m_buffering ? QtAV::BufferingMedia : QtAV::BufferedMedia);
    // state change to buffering, report progress immediately. otherwise we have to wait to read 1 packet.
    if (m_buffering) {
        Q_EMIT bufferProgressChanged(m_buffer->bufferProgress());
    }
}

//No more data to put. So stop blocking the queue to take the reset elements
void AVDemuxThread::stop()
{
    //this will not affect the pause state if we pause the output
    //TODO: why remove blockFull(false) can not play another file?
    AVThread* av[] = { audio_thread, video_thread};
    for (size_t i = 0; i < sizeof(av)/sizeof(av[0]); ++i) {
        AVThread* t = av[i];
        if (!t)
            continue;
        t->packetQueue()->clear();
        t->packetQueue()->blockFull(false); //??
        while (t->isRunning()) {
            qDebug() << "stopping thread " << t;
            t->stop();
            t->wait(500);
        }
    }
    pause(false);
    cond.wakeAll();
    qDebug("all avthread finished. try to exit demux thread<<<<<<");
    end = true;
}

void AVDemuxThread::pause(bool p, bool wait)
{
    if (paused == p)
        return;
    paused = p;
    user_paused = paused;
    if (!paused)
        cond.wakeAll();
    else {
        if (wait) {
            // block until current loop finished
            buffer_mutex.lock();
            buffer_mutex.unlock();
        }
    }
}

void AVDemuxThread::nextFrame()
{
    // clock type will be wrong if no lock because slot frameDeliveredNextFrame() is in video thread
    QMutexLocker locker(&next_frame_mutex);
    Q_UNUSED(locker);
    pause(true); // must pause AVDemuxThread (set user_paused true)
    AVThread* av[] = {video_thread, audio_thread};
    bool connected = false;
    for (size_t i = 0; i < sizeof(av)/sizeof(av[0]); ++i) {
        AVThread *t = av[i];
        if (!t)
            continue;
        // set clock first
        if (clock_type < 0)
            clock_type = (int)t->clock()->isClockAuto() + 2*(int)t->clock()->clockType();
        t->clock()->setClockType(AVClock::VideoClock);
        t->scheduleFrameDrop(false);
        t->pause(false);
        t->packetQueue()->blockFull(false);
        if (!connected) {
            connect(t, SIGNAL(frameDelivered()), this, SLOT(frameDeliveredNextFrame()), Qt::DirectConnection);
            connected = true;
        }
    }
    emit requestClockPause(false);
    pauseInternal(false);
}

void AVDemuxThread::seekOnPauseFinished()
{
    AVThread *thread = video_thread ? video_thread : audio_thread;
    Q_ASSERT(thread);
    disconnect(thread, SIGNAL(seekFinished(qint64)), this, SLOT(seekOnPauseFinished()));
    if (user_paused) {
        pause(true); // restore pause state
        emit requestClockPause(true); // need direct connection
    // pause video/audio thread
        if (video_thread)
            video_thread->pause(true);
        if (audio_thread)
            audio_thread->pause(true);
    }
    if (clock_type >= 0) {
        thread->clock()->setClockAuto(clock_type & 1);
        thread->clock()->setClockType(AVClock::ClockType(clock_type/2));
        clock_type = -1;
    }
}

void AVDemuxThread::frameDeliveredNextFrame()
{
    AVThread *thread = video_thread ? video_thread : audio_thread;
    Q_ASSERT(thread);
    QMutexLocker locker(&next_frame_mutex);
    Q_UNUSED(locker);
    disconnect(thread, SIGNAL(frameDelivered()), this, SLOT(frameDeliveredNextFrame()));
    if (user_paused) {
        pause(true); // restore pause state
        emit requestClockPause(true); // need direct connection
    // pause both video and audio thread
        if (video_thread)
            video_thread->pause(true);
        if (audio_thread)
            audio_thread->pause(true);
    }
    if (clock_type >= 0) {
        thread->clock()->setClockAuto(clock_type & 1);
        thread->clock()->setClockType(AVClock::ClockType(clock_type/2));
        clock_type = -1;
    }
}

void AVDemuxThread::onAVThreadQuit()
{
    AVThread* av[] = { audio_thread, video_thread};
    for (size_t i = 0; i < sizeof(av)/sizeof(av[0]); ++i) {
        if (!av[i])
            continue;
        if (av[i]->isRunning())
            return;
    }
    end = true; //(!audio_thread || !audio_thread->isRunning()) &&
}

void AVDemuxThread::run()
{
    m_buffering = false;
    end = false;
    if (audio_thread && !audio_thread->isRunning())
        audio_thread->start(QThread::HighPriority);
    if (video_thread && !video_thread->isRunning())
        video_thread->start();

    int stream = 0;
    Packet pkt;
    pause(false);
    qDebug("get av queue a/v thread = %p %p", audio_thread, video_thread);
    PacketBuffer *aqueue = audio_thread ? audio_thread->packetQueue() : 0;
    PacketBuffer *vqueue = video_thread ? video_thread->packetQueue() : 0;
    // aqueue as a primary buffer: music with/without cover
    AVThread* thread = !video_thread || (audio_thread && demuxer->hasAttacedPicture()) ? audio_thread : video_thread;
    m_buffer = thread->packetQueue();
    const qint64 buf2 = aqueue ? aqueue->bufferValue() : 1; // TODO: may be changed by user. Deal with audio track change
    if (aqueue) {
        aqueue->clear();
        aqueue->setBlocking(true);
    }
    if (vqueue) {
        vqueue->clear();
        vqueue->setBlocking(true);
    }
    connect(thread, SIGNAL(seekFinished(qint64)), this, SIGNAL(seekFinished(qint64)), Qt::DirectConnection);
    seek_tasks.clear();
    bool was_end = false;
    if (ademuxer) {
        ademuxer->seek(0LL);
    }
    while (!end) {
        processNextSeekTask();
        //vthread maybe changed by AVPlayer.setPriority() from no dec case
        vqueue = video_thread ? video_thread->packetQueue() : 0;
        if (demuxer->atEnd()) {
            // if avthread may skip 1st eof packet because of a/v sync
            if (aqueue && (!was_end || aqueue->isEmpty())) {
                aqueue->put(Packet::createEOF());
                aqueue->blockEmpty(false); // do not block if buffer is not enough. block again on seek
            }
            if (vqueue && (!was_end || vqueue->isEmpty())) {
                vqueue->put(Packet::createEOF());
                vqueue->blockEmpty(false);
            }
            m_buffering = false;
            Q_EMIT mediaStatusChanged(QtAV::BufferedMedia);
            was_end = true;
            // wait for a/v thread finished
            msleep(100);
            continue;
        }
        was_end = false;
        if (tryPause()) {
            continue; //the queue is empty and will block
        }
        updateBufferState();
        if (!demuxer->readFrame()) {
            continue;
        }
        stream = demuxer->stream();
        pkt = demuxer->packet();
        Packet apkt;
        bool audio_has_pic = demuxer->hasAttacedPicture();
        int a_ext = 0;
        if (ademuxer) {
            QMutexLocker locker(&buffer_mutex);
            Q_UNUSED(locker);
            if (ademuxer) {
                a_ext = -1;
                audio_has_pic = ademuxer->hasAttacedPicture();
                // FIXME: buffer full but buffering!!!
                // avoid read external track everytime. aqueue may not block full
                // vqueue will not block if aqueue is not enough
                if (!aqueue->isFull() || aqueue->isBuffering()) {
                    if (ademuxer->readFrame()) {
                        if (ademuxer->stream() == ademuxer->audioStream()) {
                            a_ext = 1;
                            apkt = ademuxer->packet();
                        }
                    }
                    // no continue otherwise. ademuxer finished earlier than demuxer
                }
            }
        }
        //qDebug("vqueue: %d, aqueue: %d/isbuffering %d isfull: %d, buffer: %d/%d", vqueue->size(), aqueue->size(), aqueue->isBuffering(), aqueue->isFull(), aqueue->buffered(), aqueue->bufferValue());

        //QMutexLocker locker(&buffer_mutex); //TODO: seems we do not need to lock
        //Q_UNUSED(locker);
        /*1 is empty but another is enough, then do not block to
          ensure the empty one can put packets immediatly.
          But usually it will not happen, why?
        */
        /* demux thread will be blocked only when 1 queue is full and still put
         * if vqueue is full and aqueue becomes empty, then demux thread
         * will be blocked. so we should wake up another queue when empty(or threshold?).
         * TODO: the video stream and audio stream may be group by group. provide it
         * stream data: aaaaaaavvvvvvvaaaaaaaavvvvvvvvvaaaaaa, it happens
         * stream data: aavavvavvavavavavavavavavvvaavavavava, it's ok
         */
        //TODO: use cache queue, take from cache queue if not empty?
        const bool a_internal = stream == demuxer->audioStream();
        if (a_internal || a_ext > 0) {//apkt.isValid()) {
            if (a_internal && !a_ext) // internal is always read even if external audio used
                apkt = demuxer->packet();
            /* if vqueue if not blocked and full, and aqueue is empty, then put to
             * vqueue will block demuex thread
             */
            if (aqueue) {
                if (!audio_thread || !audio_thread->isRunning()) {
                    aqueue->clear();
                    continue;
                }
                // must ensure bufferValue set correctly before continue
                if (m_buffer != aqueue)
                    aqueue->setBufferValue(m_buffer->isBuffering() ? std::numeric_limits<qint64>::max() : buf2);
                // always block full if no vqueue because empty callback may set false
                // attached picture is cover for song, 1 frame
                aqueue->blockFull(!video_thread || !video_thread->isRunning() || !vqueue || audio_has_pic);
                // external audio: a_ext < 0, stream = audio_idx=>put invalid packet
                if (a_ext >= 0)
                    aqueue->put(apkt); //affect video_thread
            }
        }
        // always check video stream if use external audio
        if (stream == demuxer->videoStream()) {
            if (vqueue) {
                if (!video_thread || !video_thread->isRunning()) {
                    vqueue->clear();
                    continue;
                }
                vqueue->blockFull(!audio_thread || !audio_thread->isRunning() || !aqueue || aqueue->isEnough());
                vqueue->put(pkt); //affect audio_thread
            }
        } else if (demuxer->subtitleStreams().contains(stream)) { //subtitle
            Q_EMIT internalSubtitlePacketRead(demuxer->subtitleStreams().indexOf(stream), pkt);
        } else {
            continue;
        }
    }
    m_buffering = false;
    m_buffer = 0;
    while (audio_thread && audio_thread->isRunning()) {
        qDebug("waiting audio thread.......");
        aqueue->blockEmpty(false); //FIXME: why need this
        audio_thread->wait(500);
    }
    while (video_thread && video_thread->isRunning()) {
        qDebug("waiting video thread.......");
        vqueue->blockEmpty(false);
        video_thread->wait(500);
    }
    thread->disconnect(this, SIGNAL(seekFinished(qint64)));
    qDebug("Demux thread stops running....");
    emit mediaStatusChanged(QtAV::EndOfMedia);
}

bool AVDemuxThread::tryPause(unsigned long timeout)
{
    if (!paused)
        return false;
    QMutexLocker lock(&buffer_mutex);
    Q_UNUSED(lock);
    cond.wait(&buffer_mutex, timeout);
    return true;
}

} //namespace QtAV