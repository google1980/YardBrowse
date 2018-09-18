#include "sqltablemodel.h"
#include <QSqlRecord>

SqlTableModel::SqlTableModel(QObject *parent, QSqlDatabase db,int type)
    : QSqlTableModel(parent,db)
    , m_type(type)
{

}

SqlTableModel::~SqlTableModel()
{

}

QVariant SqlTableModel::data(const QModelIndex &index, int role) const
{
    QVariant value  = QSqlTableModel::data(index,role);
    return value;
}

void SqlTableModel::insertRow(const QList<QPair<QVariant,QVariant>> &list){

    QSqlRecord record = QSqlTableModel::record();

    for(int i = 0 ; i<list.size(); i++ ){

        if (list.at(i).first.toString() == "ID"){

            record.setValue(list.at(i).first.toString(),list.at(i).second.toInt());

        }else{

            record.setValue(list.at(i).first.toString(),list.at(i).second.toString());

        }

    }

    //qDebug() << record.value("ID");
    //qDebug() << record.value("CTN_NO");

    QSqlTableModel::insertRecord(-1,record);
    emit rowInserted(record);
}

void SqlTableModel::updateRow(int key,const QList<QPair<QVariant,QVariant>>  &list){

    QSqlRecord newrecord = QSqlTableModel::record(key);
    QSqlRecord oldrecord = QSqlRecord(newrecord);

    for(int i = 0 ; i<list.size(); i++ ){

        newrecord.setValue(list.at(i).first.toString(),list.at(i).second.toString());

    }
    QSqlTableModel::setRecord(key,newrecord);
    emit rowUpdated(oldrecord,newrecord);
}

void SqlTableModel::deleteRow(int key){

    QSqlRecord record = QSqlTableModel::record(key);
    QSqlTableModel::removeRow(key);
    emit rowDeleted(record);
}
