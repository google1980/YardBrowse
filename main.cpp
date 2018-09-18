#include "mainwindow.h"
#include <QApplication>
#include <QMetaType>
#include "global.h"
#include "syncclock.h"

int main(int argc, char *argv[])
{
    qRegisterMetaType<QSqlRecord>("QSqlRecord");
    QApplication a(argc, argv);

    QSettings *configfile = new QSettings("./application.ini", QSettings::IniFormat);

    Global::IP = configfile->value("/host/ip").toString();

    Global::PORT = configfile->value("/host/port").toString();

    SyncClock::instance()->finishSync(configfile->value("/host/clock").toString());

    MainWindow w;

    w.show();

    return a.exec();
}
