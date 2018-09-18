#ifndef FORMCONTAINERQUERY_H
#define FORMCONTAINERQUERY_H

#include <QWidget>
#include "sqltablemodel.h"

namespace Ui {
class FormContainerQuery;
}

class FormContainerQuery : public QWidget
{
    Q_OBJECT

public:
    explicit FormContainerQuery(SqlTableModel *model,QWidget *parent = 0);
    ~FormContainerQuery();

private slots:
    void OnQuery();
    void OnInsert();
    void OnDelete();
    void OnUpdate();
    void OnReset();

private:
    Ui::FormContainerQuery *ui;
    SqlTableModel *m_model;
    int m_id;
};

#endif // FORMCONTAINERQUERY_H
