#include "formcontainerquery.h"
#include "ui_formcontainerquery.h"
#include <QSqlRecord>
#include <QDebug>

FormContainerQuery::FormContainerQuery(SqlTableModel *model,QWidget *parent) :
    QWidget(parent),
    m_model(model),
    m_id(-1),
    ui(new Ui::FormContainerQuery)
{
    ui->setupUi(this);

    connect(ui->queryButton, SIGNAL(clicked()), this, SLOT(OnQuery()));
    connect(ui->insertButton, SIGNAL(clicked()), this, SLOT(OnInsert()));
    connect(ui->deleteButton, SIGNAL(clicked()), this, SLOT(OnDelete()));
    connect(ui->updateButton, SIGNAL(clicked()), this, SLOT(OnUpdate()));
    connect(ui->clearButton, SIGNAL(clicked()), this, SLOT(OnReset()));

    setAttribute(Qt::WA_DeleteOnClose);
}

FormContainerQuery::~FormContainerQuery()
{
    delete ui;
}

void FormContainerQuery::OnQuery()
{
    if (!ui->LE_CTNNO->text().isEmpty()){

        qDebug() << ui->LE_CTNNO->text();

        m_model->setFilter("CTN_NO = '"+ui->LE_CTNNO->text()+"'");

        if (m_model->select()) {

            for (int i = 0; i < m_model->rowCount(); ++i) {


                QSqlRecord record = m_model->record(i);

                ui->LE_CTNNO->setText(record.value("CTN_NO").toString());
                ui->LE_CTNTYPE->setText(record.value("CTN_TYPE").toString());
                ui->LE_CTNPOS->setText(record.value("POSITION").toString());
                ui->LE_CTNOWNER->setText(record.value("CTN_OWNER").toString());
                ui->LE_CTNSIZE->setText(record.value("CTN_SIZE").toString());
                m_id = record.value("ID").toInt();
            }
        }

    }

}

void FormContainerQuery::OnInsert()
{
    QList<QPair<QVariant,QVariant>> list;
    list.append(QPair<QVariant,QVariant>("CTN_NO", ui->LE_CTNNO->text()));
    list.append(QPair<QVariant,QVariant>("CTN_TYPE", ui->LE_CTNTYPE->text()));
    list.append(QPair<QVariant,QVariant>("POSITION", ui->LE_CTNPOS->text()));
    list.append(QPair<QVariant,QVariant>("CTN_OWNER", ui->LE_CTNOWNER->text()));
    list.append(QPair<QVariant,QVariant>("CTN_SIZE", ui->LE_CTNSIZE->text()));

    m_model->insertRow(list);
    m_model->submitAll();

}

void FormContainerQuery::OnUpdate()
{
    if (m_id >=0){

        QList<QPair<QVariant,QVariant>> list;
        list.append(QPair<QVariant,QVariant>("CTN_NO", ui->LE_CTNNO->text()));
        list.append(QPair<QVariant,QVariant>("CTN_TYPE", ui->LE_CTNTYPE->text()));
        list.append(QPair<QVariant,QVariant>("POSITION", ui->LE_CTNPOS->text()));
        list.append(QPair<QVariant,QVariant>("CTN_OWNER", ui->LE_CTNOWNER->text()));
        list.append(QPair<QVariant,QVariant>("CTN_SIZE", ui->LE_CTNSIZE->text()));
        m_model->setFilter("ID = "+QString::number(m_id));

        if (m_model->select()) {

            for (int i = 0; i < m_model->rowCount(); ++i) {
                m_model->updateRow(i,list);
            }
        }
        m_model->submitAll();
    }
}

void FormContainerQuery::OnDelete()
{
    if (m_id >=0){

        m_model->setFilter("ID = "+QString::number(m_id));

        if (m_model->select()) {

            for (int i = 0; i < m_model->rowCount(); ++i) {
                m_model->deleteRow(i);
            }
        }
        m_model->submitAll();
    }
}

void FormContainerQuery::OnReset(){
    ui->LE_CTNNO->text().clear();
    ui->LE_CTNTYPE->text().clear();
    ui->LE_CTNPOS->text().clear();
    ui->LE_CTNOWNER->text().clear();
    ui->LE_CTNSIZE->text().clear();
    m_id = -1;
}
