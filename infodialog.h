#ifndef INFODIALOG_H
#define INFODIALOG_H

#include <QWidget>
#include <QDialog>

namespace Ui {
class InfoDialog;
}

class InfoDialog : public QDialog
{
    Q_OBJECT

public:
    explicit InfoDialog(QWidget *parent = 0);
    ~InfoDialog();

    void setText(const QString &ctnno,const QString &ctnSize,const QString &ctnType,const QString &ctnOwner,const QString &position);

private:
    Ui::InfoDialog *ui;

protected:
    void mousePressEvent(QMouseEvent *event) override;
};

#endif // INFODIALOG_H
