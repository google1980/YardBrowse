#include "infodialog.h"
#include "ui_infodialog.h"

InfoDialog::InfoDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::InfoDialog)
{
    ui->setupUi(this);


}

InfoDialog::~InfoDialog()
{
    delete ui;
}

void InfoDialog::mousePressEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
    this->close();
}

void InfoDialog::setText(const QString &ctnno,const QString &ctnSize,const QString &ctnType,const QString &ctnOwner,const QString &position)
{
    ui->label_ctnno->setText(ctnno);
    ui->label_ctnSize->setText(ctnSize);
    ui->label_ctnType->setText(ctnType);
    ui->label_ctnOwner->setText(ctnOwner);
    ui->label_position->setText(position);

    if (ctnSize.left(1) == '2'){

        QPixmap img;
        img.load(":/images/CTN20.png");
        ui->label_img->clear();
        ui->label_img->setPixmap(img);

    }else{

        QPixmap img;
        img.load(":/images/CTN40.png");
        ui->label_img->clear();
        ui->label_img->setPixmap(img);

    }

}
