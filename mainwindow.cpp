#include "mainwindow.h"
#include "dataSource.h"
#include "formcontainerquery.h"
#include <QDebug>

MainWindow::MainWindow():
    statusBar(new QStatusBar),
    menuBar(new QMenuBar),
    toolBar(new QToolBar),
    mdiArea(new QMdiArea),
    m_info(new InfoDialog)
{
    setCentralWidget(mdiArea);

    setWindowTitle(QString::fromUtf8("堆场"));

    createStatusBar();

    setStatusBar(statusBar);

    setMenuBar(menuBar);

    addToolBar(toolBar);

    createActions();

    QSqlError err = initDb();

    if (err.type() != QSqlError::NoError) {

        statusBar->showMessage("load db file error ", 10000);
    }

    model = new SqlTableModel(this);
    model->setTable("CONTAINER");
    model->setEditStrategy(QSqlTableModel::OnManualSubmit);

    scene = new PainterScene(model);

    m_info->setParent(this);

    m_info->setWindowFlags(Qt::Dialog|Qt::FramelessWindowHint);
    m_info->setWindowOpacity(0.9);

    PainterHelper::initMap();

    initBlockStacks();

    connect(model,&SqlTableModel::rowInserted,this,&MainWindow::onRowInserted);
    connect(model,&SqlTableModel::rowUpdated,this,&MainWindow::onRowUpdated);
    connect(model,&SqlTableModel::rowDeleted,this,&MainWindow::onRowDeleted);

    m_send_task = new SendTask(model);
    QThread *thread = new QThread();
    m_send_task->moveToThread(thread);

    connect(thread, &QThread::started, m_send_task, &SendTask::init);
    thread->start();

    connect(m_send_task, &SendTask::socketerror, this, &MainWindow::onSocketError);
    connect(m_send_task, &SendTask::connected, this, &MainWindow::onServerConnected);
    connect(this, &MainWindow::socketconnet, m_send_task, &SendTask::onSocketConnect);
    connect(this, &MainWindow::socketdisconnect, m_send_task, &SendTask::onSocketDisconnect);
    connect(this, &MainWindow::put, m_send_task, &SendTask::onPut);
    connect(this, &MainWindow::sync, m_send_task, &SendTask::onSync);

}

MainWindow::~MainWindow()
{
    //emit socketdisconnect();
    if ( m_nTimerId != 0 )
    killTimer(m_nTimerId);
    QSqlDatabase::database().close();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    Q_UNUSED(event);
    QSettings *configfile = new QSettings("./application.ini", QSettings::IniFormat);
    configfile->setValue("/host/clock",SyncClock::instance()->getClock());
}


void MainWindow::timerEvent( QTimerEvent *event )
{
    Q_UNUSED(event);
    //emit sync();
}

int MainWindow::onServerConnected(){

    statusBar->showMessage("connect to success! ", 10000);
    connectAct->setEnabled(false);

    m_nTimerId = startTimer(10000);

    //emit sync();

    return 0;
}

int MainWindow::onSocketError(const QString & str)
{
    statusBar->showMessage(str, 10000);

    if ( m_nTimerId != 0 )
    killTimer(m_nTimerId);

    return 0;
}

int MainWindow::onConnectServer(){

    emit socketconnet();
    return 0;

}

int MainWindow::onInfo(const QPoint &pos,const int &key){

    m_info->setGeometry(pos.x(),pos.y(),m_info->width(),m_info->height());
    QSqlQuery sql_query;
    QString sql_ctn = "select CTN_NO,CTN_SIZE,CTN_TYPE,CTN_OWNER,POSITION from CONTAINER where ID =" + QString::number(key);
    qDebug()<< sql_ctn;


    if(!sql_query.exec(sql_ctn)){
        qDebug()<<sql_query.lastError();
    }else{
        while (sql_query.next()){

            m_info->setText(sql_query.value(0).toString(),sql_query.value(1).toString(),
                            sql_query.value(2).toString(),sql_query.value(3).toString(),
                            sql_query.value(4).toString());
            break;

        }

    }


    m_info->show();
    return 0;
}

void MainWindow::initBlockStacks(){

    QSqlQuery sql_query;

    QString select_sql = "select ID,PIXEL_LENGTH,PIXEL_WIDTH,X_OFFSET,Y_OFFSET,ANGLE_ROTATION,NAME,MAX_TIER from BLOCK ";
    if(!sql_query.exec(select_sql)){
        qDebug()<<sql_query.lastError();
    }else{
        while (sql_query.next()){
            drawOneBlock(scene,sql_query.value(0).toInt(),sql_query.value(1).toInt(),sql_query.value(2).toInt(),
                         sql_query.value(3).toInt(),sql_query.value(4).toInt(),sql_query.value(5).toInt(),
                         sql_query.value(6).toString(),sql_query.value(7).toInt());
        }

    }

}

void MainWindow::onRowInserted(const QSqlRecord &record){

    QString newpos = record.value("POSITION").toString();
    int type;

    if (record.value("CTN_SIZE").toString().left(1) == "2")
        type = 2;
    else
        type = 1;

    fillStackContainers(newpos,QColor(PainterHelper::COLOR_MAP.value(record.value("CTN_OWNER").toString())),type);

}

void MainWindow::onRowUpdated(const QSqlRecord &oldrecord,const QSqlRecord &newrecord){

    QString newpos = newrecord.value("POSITION").toString();
    QString oldpos = oldrecord.value("POSITION").toString();

    int type;

    if (oldrecord.value("CTN_SIZE").toString().left(1) == "2")
        type = 2;
    else
        type = 1;

    clearStackContainers(oldpos,type);
    fillStackContainers(newpos,QColor(PainterHelper::COLOR_MAP.value(newrecord.value("CTN_OWNER").toString())),type);

}

void MainWindow::onRowDeleted(const QSqlRecord &record){
    qDebug()<<record.value("CTN_NO");
}

void MainWindow::createActions()
{

    QMenu *fileMenu = this->menuBar->addMenu(QString::fromUtf8("堆场"));

    const QIcon satelliteIcon = QIcon::fromTheme("yardSatelliteView", QIcon(":/images/satellite.png"));
    yardSatelliteViewAct = new QAction(satelliteIcon, QString::fromUtf8("卫星图"), this);
    yardSatelliteViewAct->setShortcuts(QKeySequence::New);
    yardSatelliteViewAct->setStatusTip(tr("Yard Satellite View"));
    connect(yardSatelliteViewAct, &QAction::triggered, this, &MainWindow::yardSatelliteView);
    fileMenu->addAction(yardSatelliteViewAct);
    toolBar->addAction(yardSatelliteViewAct);

    const QIcon cubeIcon = QIcon::fromTheme("yardSectionView", QIcon(":/images/cube.png"));
    yardSectionViewAct = new QAction(cubeIcon, QString::fromUtf8("截面图"), this);
    yardSectionViewAct->setShortcuts(QKeySequence::New);
    yardSectionViewAct->setStatusTip(tr("Yard Section View"));
    connect(yardSectionViewAct, &QAction::triggered, this, &MainWindow::yardSectionView);

    fileMenu->addAction(yardSectionViewAct);
    toolBar->addAction(yardSectionViewAct);

    const QIcon containerIcon = QIcon::fromTheme("containerView", QIcon(":/images/container.png"));
    containerViewAct = new QAction(containerIcon, QString::fromUtf8("集装箱配置"), this);
    containerViewAct->setStatusTip(tr("Query Container"));
    connect(containerViewAct, &QAction::triggered, this, &MainWindow::queryContainer);
    fileMenu->addAction(containerViewAct);
    toolBar->addAction(containerViewAct);

    const QIcon pointerIcon = QIcon::fromTheme("pointer", QIcon(":/images/pointer.png"));
    pointerAct = new QAction(pointerIcon, QString::fromUtf8("指针"), this);
    pointerAct->setStatusTip(tr("Pointer"));
    connect(pointerAct, &QAction::triggered, this, &MainWindow::pointerContainer);
    toolBar->addAction(pointerAct);
    pointerAct->setDisabled(true);
    PainterHelper::SELECTED = FLAG_POINTER;

    const QIcon correctIcon = QIcon::fromTheme("correct", QIcon(":/images/correct.png"));
    correctAct = new QAction(correctIcon, QString::fromUtf8("修正"), this);
    correctAct->setStatusTip(tr("Query Container"));
    connect(correctAct, &QAction::triggered, this, &MainWindow::correctContainer);
    toolBar->addAction(correctAct);

    const QIcon connectIcon = QIcon::fromTheme("connect", QIcon(":/images/connector.png"));
    connectAct = new QAction(connectIcon, QString::fromUtf8("连接"), this);
    connectAct->setStatusTip(tr("Connect server"));
    connect(connectAct, &QAction::triggered, this, &MainWindow::onConnectServer);
    toolBar->addAction(connectAct);

    const QIcon infoIcon = QIcon::fromTheme("info", QIcon(":/images/info.png"));
    infoAct = new QAction(infoIcon, QString::fromUtf8("信息"), this);
    infoAct->setStatusTip(tr("info"));
    connect(infoAct, &QAction::triggered, this, &MainWindow::infoContainer);
    toolBar->addAction(infoAct);


}

void MainWindow::createStatusBar()
{
    statusBar->showMessage(tr("Ready"));
}

void MainWindow::correctContainer()
{
    correctAct->setEnabled(false);
    pointerAct->setEnabled(true);
    infoAct->setEnabled(true);
    PainterHelper::SELECTED = FLAG_CHECK;
}

void MainWindow::pointerContainer()
{
    correctAct->setEnabled(true);
    pointerAct->setEnabled(false);
    infoAct->setEnabled(true);
    PainterHelper::SELECTED = FLAG_POINTER;
}

void MainWindow::infoContainer()
{
    correctAct->setEnabled(true);
    pointerAct->setEnabled(true);
    infoAct->setEnabled(false);
    PainterHelper::SELECTED = FLAG_INFO;
}

void MainWindow::queryContainer()
{
    FormContainerQuery *child = new FormContainerQuery(model);

    QMdiSubWindow * subWindow = mdiArea->addSubWindow(child);

    subWindow->resize(QSize(480,250));

    subWindow->setWindowTitle(QString::fromUtf8("集装箱"));

    subWindow->show();
}

void MainWindow::yardSatelliteView()
{
    YardSatelliteView *view = new YardSatelliteView;

    view->setScene(scene);

    view->setWindowTitle(QString::fromUtf8("卫星图"));

    view->setSceneRect(0,0,2400,2000);

    QSqlQuery sql_query;

    QString select_sql = "select NAME from BLOCK ";
    if(!sql_query.exec(select_sql)){
        qDebug()<<sql_query.lastError();
    }else{
        while (sql_query.next()){

            fillBlockContainers(sql_query.value(0).toString());
        }

    }

    QMdiSubWindow * subWindow = mdiArea->addSubWindow(view);

    subWindow->resize(QSize(1500,800));

    subWindow->setAttribute(Qt::WA_DeleteOnClose);

    subWindow->show();

}

void MainWindow::drawOneBlock(PainterScene *scene,int id,int length,int width,int xOffset,int yOffset,int angleRotation,QString name,int maxTier)
{
    int x = 0;
    int y = 0;

    if (angleRotation == 0){
        x = xOffset;
        y = yOffset;
    }

    QGraphicsItemGroup *itemGroup = new QGraphicsItemGroup();
    itemGroup->setHandlesChildEvents(false);

    QSqlQuery sql_query;

    QString sql_stack = "select ROW_ID,FIRST_COLUMN,LAST_COLUMN from BLOCK_DISPLAY where BLOCK_ID = " + QString::number(id);

    int maxColumn = 0;

    if(!sql_query.exec(sql_stack)){
        qDebug()<<sql_query.lastError();
    }else{
        while (sql_query.next()){

            int rowId = sql_query.value(0).toInt();
            int firstColumn = sql_query.value(1).toInt();
            int lastColumn = sql_query.value(2).toInt();


            maxColumn = lastColumn > maxColumn ? lastColumn : maxColumn;



            QString ROW = PainterHelper::ROW_20_MAP.value(rowId);

            for(int i = firstColumn;i<=lastColumn; i++ ){

                Stack * stack = new Stack(length,width,id,rowId,i,name,maxTier);
                stack->setStartPoint(QPointF(x + (rowId-1)*length,y + (i-1)*width));

                if (i == firstColumn){

                    QGraphicsTextItem *textItem = new QGraphicsTextItem(ROW);
                    textItem->setPos(x + (rowId-1)*length,y-18);
                    itemGroup->addToGroup(textItem);

                }

                QString COL = PainterHelper::COL_MAP.value(i);

                itemGroup->addToGroup(stack);

                PainterHelper::STACK_MAP.insert(name+ROW+COL,stack);
            }


        }
    }

    QGraphicsTextItem *textItem = new QGraphicsTextItem(name);
    QFont font;
    font.setBold(true);
    font.setPixelSize(20);
    textItem->setFont(font);
    QColor color = Qt::black;
    color.setAlpha(150);
    textItem->setDefaultTextColor(color);
    textItem->setPos(x,y + maxColumn * width);
    itemGroup->addToGroup(textItem);

    if (angleRotation != 0){

        QTransform  transform;
        transform.translate(xOffset,yOffset);
        transform.rotate(angleRotation, Qt::ZAxis);
        itemGroup->setTransform(transform);
    }

    scene->addItem(itemGroup);
    scene->destroyItemGroup(itemGroup);
}

void MainWindow::fillBlockContainers(QString name){

    QSqlQuery sql_query;

    QString sql_ctn = "select POSITION,CTN_OWNER,CTN_SIZE from CONTAINER where POSITION like '" + name +"%'";

    if(!sql_query.exec(sql_ctn)){
        qDebug()<<sql_query.lastError();
    }else{
        while (sql_query.next()){

            int type;

            if (sql_query.value(2).toString().left(1) == "2")
                type = 2;
            else
                type = 1;

            fillStackContainers(sql_query.value(0).toString(),QColor(PainterHelper::COLOR_MAP.value(sql_query.value(1).toString())),type);

        }
    }

}

void MainWindow::fillStackContainers(QString pos,QColor color,int type){

    Stack * stack;

    if (type == 2){

        stack = PainterHelper::STACK_MAP.value(pos.left(6));
        stack->setFillColor(pos.right(1).toInt(),color);
        stack->update();

    }else{

        int row40 = PainterHelper::ROW_40_MAP.key(pos.mid(2,2));
        QString row20 = PainterHelper::ROW_20_MAP.value(row40);
        stack = PainterHelper::STACK_MAP.value(pos.left(2)+row20+pos.mid(4,2));
        stack->m_closeFlag=1;
        stack->setFillColor(pos.right(1).toInt(),color);
        stack->update();
        row20 = PainterHelper::ROW_20_MAP.value(row40+1);
        stack = PainterHelper::STACK_MAP.value(pos.left(2)+row20+pos.mid(4,2));
        stack->m_closeFlag=2;
        stack->setFillColor(pos.right(1).toInt(),color);
        stack->update();
    }

}

void MainWindow::clearStackContainers(QString pos,int type){

    Stack * stack;

    if (type == 2){

        stack = PainterHelper::STACK_MAP.value(pos.left(6));
        stack->m_colorlist.remove(pos.right(1).toInt());
        stack->m_closeFlag = 0;
        stack->update();

    }else{

        int row40 = PainterHelper::ROW_40_MAP.key(pos.mid(2,2));
        QString row20 = PainterHelper::ROW_20_MAP.value(row40);
        stack = PainterHelper::STACK_MAP.value(pos.left(2)+row20+pos.mid(4,2));
        stack->m_colorlist.remove(pos.right(1).toInt());
        stack->m_closeFlag = 0;
        stack->update();

        row20 = PainterHelper::ROW_20_MAP.value(row40+1);
        stack = PainterHelper::STACK_MAP.value(pos.left(2)+row20+pos.mid(4,2));
        stack->m_colorlist.remove(pos.right(1).toInt());
        stack->m_closeFlag = 0;
        stack->update();
    }


}

void MainWindow::yardSectionView(){

    int sceneRectWidth = 1500;

    int max = 4;

    YardSectionView *view = new YardSectionView(model);

    QString title;

    connect(model,&SqlTableModel::rowUpdated,view,&YardSectionView::onRowUpdated);
    connect(view, &YardSectionView::put, m_send_task, &SendTask::onPut);
    connect(view,&YardSectionView::info,this,&MainWindow::onInfo);

    QList<Quintet> sections;

    QList<QGraphicsItem *> items = this->scene->selectedItems();
    foreach (QGraphicsItem *item, items) {
        if ((item->type() == QGraphicsItem::UserType + 1)) {

            Stack * stack = static_cast<Stack *> (item);
            Quintet quintet;
            quintet._first = stack->getBlockId();
            quintet._second = stack->getRowId();
            quintet._name = stack->m_blockName;
            quintet._max_tier = stack->getMaxTier();
            quintet._row = PainterHelper::ROW_20_MAP.value(stack->getRowId());

            if (!sections.contains(quintet)){
                sections.append(quintet);
            }
        }
    }

    int width = 0;

    for(int i= 0 ; i< sections.size(); i++){

        width = drawOneSection(view->scene(),sections.at(i)._first,sections.at(i)._second,50,50,sections.at(i)._max_tier,50+width,25,sections.at(i)._name,view);

    }

    for(int i= 0 ; i< sections.size(); i++){

        fillSections(sections.at(i)._name,sections.at(i)._second,view);
        title = title + sections.at(i)._name + sections.at(i)._row +  ": ";

        if (max < sections.at(i)._max_tier){
            max = sections.at(i)._max_tier;
        }

    }

    view->setWindowTitle(QString::fromUtf8("截面图 ")+title);

    if ((50 + width) > sceneRectWidth){
        sceneRectWidth = 50 + width;
    }

    view->setSceneRect(0,0,sceneRectWidth,(max+1) *50);

    QMdiSubWindow * subWindow = mdiArea->addSubWindow(view);

    subWindow->resize(QSize(1200,(max+1)*50 + 80));

    subWindow->setAttribute(Qt::WA_DeleteOnClose);

    subWindow->show();

}

int MainWindow::drawOneSection(QGraphicsScene *scene,int blockId,int rowId,int height,int width,int tier,int xOffset,int yOffset,QString name,YardSectionView * view){

    int firstColumn = 0;
    int lastColumn = 0;

    QString ROW = PainterHelper::ROW_20_MAP.value(rowId);
    QSqlQuery sql_query;

    QString sql_section = "select FIRST_COLUMN,LAST_COLUMN from BLOCK_DISPLAY where BLOCK_ID = " + QString::number(blockId) +" and ROW_ID = " +QString::number(rowId);

    if(!sql_query.exec(sql_section)){
        qDebug()<<sql_query.lastError();
    }else{
        while (sql_query.next()){

            firstColumn = sql_query.value(0).toInt();
            lastColumn = sql_query.value(1).toInt();

            for(int i = firstColumn;i<=lastColumn; i++ ){

                QString COL = PainterHelper::COL_MAP.value(i);

                for (int j=0; j<tier; j++){

                    QString TIER = PainterHelper::TIER_MAP.value(tier-j);
                    Slot * slot = new Slot(height,width);
                    slot->setStartPoint(QPointF(xOffset + (i-1)*width,yOffset + j*height));
                    slot->setName(name+ROW+COL+TIER);
                    scene->addItem(slot);
                    view->getSLOTMAP()->insert(name+ROW+COL+TIER,slot);


                }
            }

            QGraphicsTextItem *textItem = new QGraphicsTextItem(name+ROW);
            QFont font;
            font.setBold(true);
            font.setPixelSize(14);
            textItem->setFont(font);
            textItem->setPos(xOffset + 3*width-15, yOffset + tier*height);
            scene->addItem(textItem);
        }
    }

    return xOffset + lastColumn * width;
}

void MainWindow::fillSlotContainers(QString pos,QColor color, QString text,int id,YardSectionView * view,int used,int size){

    Slot * slot = view->getSLOTMAP()->value(pos);
    slot->setFillColor(color);
    slot->setText(text);
    slot->setKey(id);
    slot->setUsed(used);
    slot->setSize(size);
    slot->update();
}

void MainWindow::fillSections(QString blockName,int rowId,YardSectionView * view){

    QString ROW = PainterHelper::ROW_20_MAP.value(rowId);

    QSqlQuery sql_query;

    QString sql_ctn = "select POSITION,CTN_OWNER,CTN_TYPE,CTN_SIZE,ID from CONTAINER where POSITION like '" + blockName + ROW +"%'";

    if(!sql_query.exec(sql_ctn)){
        qDebug()<<sql_query.lastError();
    }else{
        while (sql_query.next()){

            fillSlotContainers(sql_query.value(0).toString(),QColor(PainterHelper::COLOR_MAP.value(sql_query.value(1).toString())),
                               sql_query.value(3).toString()+sql_query.value(2).toString(),sql_query.value(4).toInt(),view,1,1);

        }
    }


    QString ROW40 = PainterHelper::ROW_40_MAP.value(rowId);

    sql_ctn = "select POSITION,CTN_OWNER,CTN_TYPE,CTN_SIZE,ID from CONTAINER where POSITION like '" + blockName + ROW40 +"%'";

    if(!sql_query.exec(sql_ctn)){
        qDebug()<<sql_query.lastError();
    }else{
        while (sql_query.next()){

            QString pos = sql_query.value(0).toString();

            fillSlotContainers(pos.left(2)+ROW+pos.mid(4,4),QColor(PainterHelper::COLOR_MAP.value(sql_query.value(1).toString())),
                               sql_query.value(3).toString()+sql_query.value(2).toString(),sql_query.value(4).toInt(),view,2,2);

        }
    }

    if (rowId-1 > 0 ){

        ROW40 = PainterHelper::ROW_40_MAP.value(rowId-1);

        sql_ctn = "select POSITION,CTN_OWNER,CTN_TYPE,CTN_SIZE,ID from CONTAINER where POSITION like '" + blockName + ROW40 +"%'";

        if(!sql_query.exec(sql_ctn)){
            qDebug()<<sql_query.lastError();
        }else{
            while (sql_query.next()){

                QString pos = sql_query.value(0).toString();
                fillSlotContainers(pos.left(2)+ROW+pos.mid(4,4),QColor(PainterHelper::COLOR_MAP.value(sql_query.value(1).toString())),
                                   sql_query.value(3).toString()+sql_query.value(2).toString(),sql_query.value(4).toInt(),view,3,2);

            }
        }
    }

}
