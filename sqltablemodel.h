#ifndef SQLTABLEMODEL_H
#define SQLTABLEMODEL_H

#include <QVariant>
#include <QSqlDatabase>
#include <QSqlTableModel>

class SqlTableModel : public QSqlTableModel
{
    Q_OBJECT
public:
    SqlTableModel(QObject *parent = Q_NULLPTR, QSqlDatabase db = QSqlDatabase(), int type = 0);
    ~SqlTableModel();

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    void insertRow(const QList<QPair<QVariant,QVariant>>  &list);
    void updateRow(int key,const QList<QPair<QVariant,QVariant>>  &list);
    void deleteRow(int key);
signals:
    void rowInserted(const QSqlRecord &record);
    void rowUpdated(const QSqlRecord &oldRecord,const QSqlRecord &newRecord);
    void rowDeleted(const QSqlRecord &record);
private:

    int m_type;

};

#endif // SQLTABLEMODEL_H
