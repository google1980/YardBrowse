#ifndef SENDTASK_H
#define SENDTASK_H

#include <QObject>
#include <QThread>
#include <QTcpSocket>
#include <QMutex>
#include <QTimer>
#include "sqltablemodel.h"
#include "global.h"
#include "syncclock.h"

class SendTask : public QObject
{
    Q_OBJECT
public:
    explicit SendTask(SqlTableModel *model,QObject *parent = nullptr);
    void init();
    void connectServer();
    void sendMessage();
    QTimer * getTimer();

signals:
    void socketerror(const QString & str);
    void connected();
    void sendReady();

public slots:
    void displayError(QAbstractSocket::SocketError socketError);
    void onReceiveMessage();
    void onSocketConnected();
    void onSocketConnect();
    void onSocketDisconnect();
    void onPut(const QString &str);
    void onSync();
    void onTimeout();

private:

    void updateModel(const QJsonObject &json);
    QTcpSocket * m_client_socket;
    QByteArray m_buffer;
    QList<QByteArray> m_list;
    QMutex listMutex;
    QString m_HostAddress = Global::IP;
    int m_Port = Global::PORT.toInt();
    SqlTableModel *m_model;
    QTimer * m_pTimer;
};

#endif // SENDTASK_H
