#ifndef SYNCCLOCK_H
#define SYNCCLOCK_H

#include <QObject>
#include <QReadWriteLock>
#include "singleton.h"

class SyncClock : public QObject
{
    Q_OBJECT
public:
    explicit SyncClock(QObject *parent = nullptr);
    static SyncClock* instance();
    void finishSync(const QString &clock);
    void cancelSync();
    QString startSync();
    QString getClock();
    bool isSynchronized();
signals:

public slots:

private:
    static SyncClock* createInstance();
    QReadWriteLock lock;
    QString m_s_clock;
    bool m_synchronized;
};

#endif // SYNCCLOCK_H
