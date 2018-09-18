#include "syncclock.h"
#include <QDebug>
#include <QTimerEvent>

SyncClock::SyncClock(QObject *parent) : QObject(parent)
{

}

SyncClock* SyncClock::createInstance()
{
    return new SyncClock();
}

SyncClock* SyncClock::instance()
{
    return Singleton<SyncClock>::instance(SyncClock::createInstance);
}

void SyncClock::finishSync(const QString &clock){
    QWriteLocker locker(&lock);
    m_s_clock = clock;
    m_synchronized = true;
}

void SyncClock::cancelSync(){
    QWriteLocker locker(&lock);
    m_synchronized = true;
}

QString SyncClock::startSync(){

    QWriteLocker locker(&lock);
    m_synchronized = false;
    return m_s_clock;

}

bool SyncClock::isSynchronized(){
    QReadLocker locker(&lock);
    return m_synchronized;
}

QString SyncClock::getClock(){
    QReadLocker locker(&lock);
    return m_s_clock;
}
