#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#ifdef WIN32
#pragma execution_character_set("utf-8")
#endif

#include <QMainWindow>
#include <QMenu>
#include <QMdiArea>
#include <QAction>
#include <QStatusBar>
#include <QMdiSubWindow>
#include <QMenuBar>
#include <QToolBar>
#include <QCloseEvent>
#include <QDialog>
#include "painter.h"
#include "sqltablemodel.h"
#include "sendtask.h"
#include "syncclock.h"
#include "infodialog.h"

struct Quintet{

    int _first;
    int _second;
    QString _name;
    QString _row;
    int _max_tier;

    bool operator == (const Quintet &other){

        if((_first==other._first)&&(_second==other._second))
            return true;
        else
            return false;
    }

};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow();
    ~MainWindow();
    void closeEvent(QCloseEvent *event);

signals:
    void socketconnet();
    void put(const QString &str);
    void sync();
    void socketdisconnect();

private slots:

    void yardSatelliteView();
    void yardSectionView();
    void queryContainer();
    void correctContainer();
    void pointerContainer();
    void infoContainer();
    void onRowInserted(const QSqlRecord &record);
    void onRowUpdated(const QSqlRecord &oldrecord,const QSqlRecord &newrecord);
    void onRowDeleted(const QSqlRecord &record);

    int onSocketError(const QString &str);
    int onConnectServer();
    int onServerConnected();

    int onInfo(const QPoint &pos,const int &key);

protected:
    void timerEvent( QTimerEvent *event );

private:
    void createActions();
    void createStatusBar();
    void drawOneBlock(PainterScene *scene,int id,int length,int width,int xOffset,int yOffset,int angleRotation,QString name,int maxTier);
    int drawOneSection(QGraphicsScene *scene,int blockId,int rowId,int height,int width,int tier,int xOffset,int yOffset,QString name,YardSectionView * view);
    void fillBlockContainers(QString name);
    void fillSections(QString name,int rowId,YardSectionView * view);
    void fillStackContainers(QString pos,QColor color, int type);
    void fillSlotContainers(QString pos,QColor color, QString text,int id,YardSectionView * view,int used,int size);
    void clearStackContainers(QString pos,int type);
    void initBlockStacks();


    QStatusBar *statusBar;
    QMenuBar *menuBar;
    QToolBar *toolBar;
    QMdiArea *mdiArea;

    QAction *yardSatelliteViewAct;
    QAction *yardSectionViewAct;
    QAction *containerViewAct;
    QAction *correctAct;
    QAction *pointerAct;
    QAction *connectAct;
    QAction *infoAct;

    PainterScene *scene;
    SqlTableModel *model;
    SendTask * m_send_task;
    int m_nTimerId;
    InfoDialog *m_info;

};

#endif // MAINWINDOW_H
