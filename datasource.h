#ifndef DATASOURCE_H
#define DATASOURCE_H

#include <QObject>
#include <QtSql>

QSqlError initDb()
{
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("yard.db");

    if (!db.open())
        return db.lastError();

    return QSqlError();
}

#endif // DATASOURCE_H
