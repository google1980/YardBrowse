#include "sendtask.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QHostAddress>
#include <QByteArray>

SendTask::SendTask(SqlTableModel *model,QObject *parent) : QObject(parent)
{
    m_model = model;


}

void SendTask::connectServer()
{
    m_client_socket->connectToHost(QHostAddress(m_HostAddress), m_Port);
}

void SendTask::onSocketDisconnect()
{
    m_client_socket->disconnectFromHost();
}

QTimer * SendTask::getTimer()
{
    return m_pTimer;
}

void SendTask::onTimeout()
{

    SyncClock::instance()->cancelSync();
    qDebug("Synchronize canceled!!!");
    m_pTimer->stop();

}

void SendTask::init()
{
    m_pTimer = new QTimer();
    m_client_socket = new QTcpSocket();
    m_buffer.clear();
    connectServer();
    typedef void (QAbstractSocket::*QAbstractSocketErrorSignal)(QAbstractSocket::SocketError);

    connect(m_client_socket, static_cast<QAbstractSocketErrorSignal>(&QAbstractSocket::error),
            this, &SendTask::displayError);
    connect(m_client_socket, &QIODevice::readyRead, this, &SendTask::onReceiveMessage);

    connect(m_client_socket, &QAbstractSocket::connected, this, &SendTask::onSocketConnected);

    connect(m_pTimer, &QTimer::timeout, this, &SendTask::onTimeout);

    connect(this, &SendTask::sendReady, this, &SendTask::sendMessage, Qt::QueuedConnection);

}

void SendTask::onSocketConnected()
{
    qDebug("socket connected!!!");
    emit connected();
}

void SendTask::displayError(QAbstractSocket::SocketError socketError)
{
    QString str;

    switch (socketError) {
    case QAbstractSocket::RemoteHostClosedError:
        qDebug("Remote Host Closed Error! ");
        str = "Remote Host Closed Error! ";
        break;
    case QAbstractSocket::HostNotFoundError:
        qDebug("Host Not Found Error! ");
        str = "Host Not Found Error! ";
        break;
    case QAbstractSocket::ConnectionRefusedError:
        qDebug("Connection Refused Error! ");
        str = "Connection Refused Error! ";
        break;
    case QAbstractSocket::NetworkError:
        qDebug("New Work Error!");
        str = "New Work Error!";
        break;
    default:
        qDebug("Unknown socket error!");
        str = "Unknown socket error!";
    }
    emit socketerror(str);

}

void SendTask::onReceiveMessage()
{
    if (m_client_socket->bytesAvailable() <= 0)
    {
        return;
    }

    QByteArray buffer;
    QByteArray message;

    buffer = m_client_socket->readAll();

    m_buffer.append(buffer);

    int start = m_buffer.indexOf("///");

    while (start >= 0){

        m_buffer = m_buffer.right(m_buffer.size() - start);

        int end = m_buffer.indexOf("$$$");

        if (end >= 0){

            message = m_buffer.left(end + 1);
            m_buffer = m_buffer.right(m_buffer.size() - message.size());

            if (message.size() - 2 > 0){

                message = message.mid(3, message.size() - 4);
                QJsonObject json = QJsonDocument::fromJson(message).object();

                QString clock = json.value("data").toObject().value("clock").toString();

                qDebug() << clock;

                if (!clock.isEmpty()){

                    if (!SyncClock::instance()->isSynchronized()){

                        SyncClock::instance()->finishSync(clock);
                        m_pTimer->stop();
                        qDebug("Synchronize finished!!!");
                        updateModel(json);
                    }

                }else{

                    updateModel(json);

                }



            }

        }
        else{

            break;
        }

        start = m_buffer.indexOf("///");
    }
}

void SendTask::updateModel(const QJsonObject &json)
{
    QJsonArray data = json.value("data").toObject().value("ctns").toArray();

    if(!data.isEmpty()){

        for (int i =0 ; i < data.count(); i++) {
            QList<QPair<QVariant,QVariant>> list;
            list.append(QPair<QVariant,QVariant>("CTN_NO", data.at(i).toObject().value("ctnNo").toString()));
            list.append(QPair<QVariant,QVariant>("CTN_TYPE", data.at(i).toObject().value("ctnType").toString()));
            list.append(QPair<QVariant,QVariant>("POSITION", data.at(i).toObject().value("position").toString()));
            list.append(QPair<QVariant,QVariant>("CTN_OWNER", data.at(i).toObject().value("ctnOwner").toString()));
            list.append(QPair<QVariant,QVariant>("CTN_SIZE", data.at(i).toObject().value("ctnSize").toString()));

            m_model->setFilter("ID = "+QString::number(data.at(i).toObject().value("id").toInt()));

            if (m_model->select()) {

                if(m_model->rowCount() > 0){

                    for (int i = 0; i < m_model->rowCount(); ++i) {
                        m_model->updateRow(i,list);
                    }

                }else{

                    list.append(QPair<QVariant,QVariant>("ID", QString::number(data.at(i).toObject().value("id").toInt())));
                    m_model->insertRow(list);

                }
            }

            m_model->submitAll();
        }
    }
}


void SendTask::onPut(const QString &str)
{
    QByteArray bytes = str.toUtf8();
    bytes.insert(0, "///");
    bytes.append("$$$");
    QMutexLocker locker(&listMutex);
    m_list.append(bytes);
    emit sendReady();
}

void SendTask::onSync()
{
    QJsonObject json;

    if (SyncClock::instance()->isSynchronized()){

        qDebug("Synchronize started ...");

        json.insert("serviceName", "syncService");
        json.insert("clock", SyncClock::instance()->startSync());

        QString str = QJsonDocument(json).toJson();

        QByteArray bytes = str.toUtf8();
        bytes.insert(0, "///");
        bytes.append("$$$");
        QMutexLocker locker(&listMutex);
        m_list.append(bytes);
        m_pTimer->start(5000);
        emit sendReady();
    }
}

void SendTask::sendMessage()
{
    QMutexLocker locker(&listMutex);
    while (!m_list.isEmpty()) {
        QByteArray bytes = m_list.takeLast();
        m_client_socket->write(bytes);
    }
}

void SendTask::onSocketConnect()
{
    connectServer();
}
