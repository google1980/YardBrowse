#include "painter.h"
#include<QSqlQuery>
#include<QSqlError>

QMap<QString,Stack *> PainterHelper::STACK_MAP;

QMap<int,QString> PainterHelper::ROW_20_MAP;
QMap<int,QString> PainterHelper::ROW_40_MAP;
QMap<int,QString> PainterHelper::TIER_MAP;
QMap<int,QString> PainterHelper::COL_MAP;
QMap<QString,QString> PainterHelper::COLOR_MAP;
QMap<int,int> PainterHelper::SELECTED_CTNS_MAP;

unsigned int PainterHelper::SELECTED = 0;

void PainterHelper::initMap()
{
    QSqlQuery sql_query;

    QString sql_text = "select TABLE_ID,LOGICAL,USER_NAME from TABLE_DISPLAY ";
    sql_query.exec(sql_text);

    if(!sql_query.exec(sql_text)){
        qDebug()<<sql_query.lastError();
    }else{
        while (sql_query.next()){
            if (sql_query.value(0).toInt() == 1)
                PainterHelper::ROW_20_MAP.insert(sql_query.value(1).toInt(),sql_query.value(2).toString());
            else if (sql_query.value(0).toInt() == 2)
                PainterHelper::ROW_40_MAP.insert(sql_query.value(1).toInt(),sql_query.value(2).toString());
            else if (sql_query.value(0).toInt() == 3)
                PainterHelper::TIER_MAP.insert(sql_query.value(1).toInt(),sql_query.value(2).toString());
            else if (sql_query.value(0).toInt() == 4)
                PainterHelper::COL_MAP.insert(sql_query.value(1).toInt(),sql_query.value(2).toString());
            else
                continue;
        }

    }

    sql_text = "select CTN_OWNER,COLOR from CODE_CTN_OWNER ";
    sql_query.exec(sql_text);
    if(!sql_query.exec(sql_text)){
        qDebug()<<sql_query.lastError();
    }else{
        while (sql_query.next()){
            PainterHelper::COLOR_MAP.insert(sql_query.value(0).toString(),sql_query.value(1).toString());
        }

    }
}

PainterScene::PainterScene(SqlTableModel *model,QObject * parent)
    : QGraphicsScene(parent)
    , m_model(model)
{

}

PainterScene::~PainterScene()
{

}

void PainterScene::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
/*
    this->addItem(m_select);
    m_select->setStartPoint(event->scenePos());
*/
    QGraphicsScene::mousePressEvent(event);

}

void PainterScene::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
/*
    m_select->setEndPoint(event->scenePos());
    update();
*/
    QGraphicsScene::mouseMoveEvent(event);

}

void PainterScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
/*
    QList<QGraphicsItem *> items = this->items();
    foreach (QGraphicsItem *item, items) {
        if ((item->type() == QGraphicsItem::UserType + 1)) {

            Stack * stack = static_cast<Stack *> (item);

            if (QRectF(stack->getStartPoint(),stack->getEndPoint()).intersects(m_select->boundingRect())){

                stack->setSelected(true);

            }
        }
    }

    this->removeItem(m_select);
    update();
*/
    QGraphicsScene::mouseReleaseEvent(event);

}

SqlTableModel * PainterScene::getModel()
{
    return m_model;
}

YardSatelliteView::YardSatelliteView(QWidget *parent)
    : QGraphicsView(parent)
    , m_zoomDelta(0.1)
    , m_scale(1.0)
{
    setRenderHint(QPainter::Antialiasing,false);
    setAttribute(Qt::WA_DeleteOnClose);
    setTransformationAnchor(QGraphicsView::AnchorViewCenter);
    setDragMode(QGraphicsView::RubberBandDrag);
}

YardSatelliteView::~YardSatelliteView()
{

}

void YardSatelliteView::setZoomDelta(qreal delta)
{
    // 建议增量范围
    Q_ASSERT_X(delta >= 0.0 && delta <= 1.0,
               "InteractiveView::setZoomDelta", "Delta should be in range [0.0, 1.0].");
    m_zoomDelta = delta;
}

void YardSatelliteView::wheelEvent(QWheelEvent *event)
{
    // 滚轮的滚动量
    //this->centerOn(event->pos());
    QPoint scrollAmount = event->angleDelta();
    // 正值表示滚轮远离使用者（放大），负值表示朝向使用者（缩小）
    scrollAmount.y() > 0 ? zoomIn() : zoomOut();
    QGraphicsView::wheelEvent(event);
}

// 放大
void YardSatelliteView::zoomIn()
{
    zoom(1 + m_zoomDelta);
}

// 缩小
void YardSatelliteView::zoomOut()
{
    zoom(1 - m_zoomDelta);
}

// 缩放 - scaleFactor：缩放的比例因子
void YardSatelliteView::zoom(float scaleFactor)
{
    // 防止过小或过大
    qreal factor = transform().scale(scaleFactor, scaleFactor).mapRect(QRectF(0, 0, 1, 1)).width();
    if (factor < 0.1 || factor > 100)
        return;
    //
    scale(scaleFactor, scaleFactor);



    m_scale *= scaleFactor;

    //this->resetMatrix();

}


YardSectionView::YardSectionView(SqlTableModel *model,QWidget *parent)
    : QGraphicsView(parent)
    , m_SLOT_MAP(new QMap<QString,Slot *>())
{
    m_scene = new PainterScene(model);
    setAttribute(Qt::WA_DeleteOnClose);
    this->setScene(m_scene);

    setDragMode(QGraphicsView::RubberBandDrag);
}

YardSectionView::~YardSectionView()
{
    delete m_scene;
    delete m_SLOT_MAP;
}

/*
bool YardSectionView::event(QEvent *event)
{
    if (event->type() == QEvent::ToolTip) {
        QHelpEvent *helpEvent = static_cast<QHelpEvent *>(event);
        QGraphicsItem * item = itemAt(helpEvent->pos());
        if ((item != Q_NULLPTR)&&(item->type() == QGraphicsItem::UserType + 2)) {

            m_info->setStartPoint(helpEvent->globalPos());
            m_info->setVisible(true);

            //QToolTip::showText(helpEvent->globalPos(), "fuck!!!");
        } else {
            //QToolTip::hideText();
            m_info->setVisible(false);
            event->ignore();
        }

        return true;
    }
    return QWidget::event(event);
}
*/

void YardSectionView::onRowUpdated(const QSqlRecord &oldrecord,const QSqlRecord &newrecord)
{

    QString newpos = newrecord.value("POSITION").toString();
    QString oldpos = oldrecord.value("POSITION").toString();

    if (oldrecord.value("CTN_SIZE").toString().left(1) == "2"){

        clearSlotContainers(oldpos);
        fillSlotContainers(newpos,QColor(PainterHelper::COLOR_MAP.value(newrecord.value("CTN_OWNER").toString())),
                           newrecord.value("CTN_SIZE").toString()+newrecord.value("CTN_TYPE").toString(),
                           newrecord.value("ID").toInt(),1,1);

    }else{

        int row40 = PainterHelper::ROW_40_MAP.key(oldpos.mid(2,2));
        QString row20 = PainterHelper::ROW_20_MAP.value(row40);
        clearSlotContainers(oldpos.left(2)+row20+oldpos.mid(4,4));
        row20 = PainterHelper::ROW_20_MAP.value(row40+1);
        clearSlotContainers(oldpos.left(2)+row20+oldpos.mid(4,4));

        row40 = PainterHelper::ROW_40_MAP.key(newpos.mid(2,2));
        row20 = PainterHelper::ROW_20_MAP.value(row40);
        fillSlotContainers(newpos.left(2)+row20+newpos.mid(4,4),QColor(PainterHelper::COLOR_MAP.value(newrecord.value("CTN_OWNER").toString())),
                           newrecord.value("CTN_SIZE").toString()+newrecord.value("CTN_TYPE").toString(),
                           newrecord.value("ID").toInt(),2,2);
        row20 = PainterHelper::ROW_20_MAP.value(row40+1);
        fillSlotContainers(newpos.left(2)+row20+newpos.mid(4,4),QColor(PainterHelper::COLOR_MAP.value(newrecord.value("CTN_OWNER").toString())),
                           newrecord.value("CTN_SIZE").toString()+newrecord.value("CTN_TYPE").toString(),
                           newrecord.value("ID").toInt(),3,2);
    }

}

void YardSectionView::fillSlotContainers(QString pos,QColor color,QString text,int key,int used,int size)
{
    Slot * slot = m_SLOT_MAP->value(pos);
    if (slot != Q_NULLPTR){

        slot->setFillColor(color);
        slot->setText(text);
        slot->setKey(key);
        slot->setUsed(used);
        slot->setSize(size);
        slot->update();

    }

}

void YardSectionView::clearSlotContainers(QString pos)
{
    Slot * slot = m_SLOT_MAP->value(pos);
    if (slot != Q_NULLPTR){
        slot->setFillColor(Qt::white);
        slot->setText("");
        slot->setKey(0);
        slot->setUsed(0);
        slot->setSize(0);
        slot->update();
    }
}

QMap<QString,Slot *> * YardSectionView::getSLOTMAP()
{
    return m_SLOT_MAP;
}

void YardSectionView::mousePressEvent(QMouseEvent *event)
{

    if (event->button() == Qt::RightButton){
        if (PainterHelper::SELECTED & FLAG_CHECK) {

            QList<QGraphicsItem *> list = this->scene()->selectedItems();

            qDebug() << "target list : " + QString::number(list.size());

            for (int i=0 ; i<list.size(); i++){
                Slot * slot = static_cast<Slot *> (list.at(i));
                if (slot->getKey() != 0){

                    continue;

                }else{

                    if ((slot->getUsed() == 0)&&(PainterHelper::SELECTED_CTNS_MAP.size() > 0)){

                        QList<QPair<QVariant,QVariant>> pairs;

                        if (PainterHelper::SELECTED_CTNS_MAP.first() == 1){

                            pairs.append(QPair<QVariant,QVariant>("POSITION", slot->getName()));


                        }else{

                            int row40 = PainterHelper::ROW_20_MAP.key(slot->getName().mid(2,2));

                            slot = m_SLOT_MAP->value(slot->getName().left(2)+ PainterHelper::ROW_20_MAP.value(row40 +1) + slot->getName().mid(4,4));
                            if ((slot != Q_NULLPTR) && slot->getUsed()==0){

                                pairs.append(QPair<QVariant,QVariant>("POSITION", slot->getName().left(2)+
                                                                     PainterHelper::ROW_40_MAP.value(row40)+slot->getName().mid(4,4)));

                            }else{

                                return;

                            }
                        }

                        QJsonArray jsonArray;

                        for(int j=0;j<pairs.size();j++){
                            QJsonObject record;
                            record.insert("id", QString::number(PainterHelper::SELECTED_CTNS_MAP.keys().first()));
                            record.insert(pairs.at(j).first.toString(), pairs.at(j).second.toString());
                            jsonArray.append(record);
                        }

                        QJsonObject json;

                        json.insert("serviceName", "ctnService");
                        json.insert("ctns", jsonArray);

                        QString str = QJsonDocument(json).toJson();

                        qDebug() << str;

                        emit put(str);

                        //slot->setSelected(false);

                        //PainterScene * scene = static_cast<PainterScene *> (this->scene());

                        //scene->getModel()->setFilter("ID = "+QString::number(PainterHelper::SELECTED_CTNS_MAP.firstKey()));

                        //if (scene->getModel()->select()) {

                        //    for (int i = 0; i < scene->getModel()->rowCount(); ++i) {
                        //        scene->getModel()->updateRow(i,pairs);
                        //    }
                        //}

                        //scene->getModel()->submitAll();
                        PainterHelper::SELECTED_CTNS_MAP.remove(PainterHelper::SELECTED_CTNS_MAP.firstKey());
                        break;
                    }

                }
            }
        }else if(PainterHelper::SELECTED & FLAG_INFO){

            QGraphicsItem * item = itemAt(event->pos());
            if ((item != Q_NULLPTR)&&(item->type() == QGraphicsItem::UserType + 2)) {

                Slot * slot = static_cast<Slot *> (item);

                if (slot->getKey()!=0){

                    emit info(event->globalPos(),slot->getKey());

                }
            }
        }
    }else{

        if (PainterHelper::SELECTED & FLAG_CHECK) {

            //PainterHelper::SELECTED_CTNS_MAP.clear();
            QList<QGraphicsItem *> list = this->scene()->selectedItems();

            qDebug() << "source list : " + QString::number(list.size());

            for (int i=0 ; i<list.size(); i++){
                Slot * slot = static_cast<Slot *> (list.at(i));
                if (slot->getKey() != 0){
                    PainterHelper::SELECTED_CTNS_MAP.insert(slot->getKey(),slot->getSize());
                }else{


                    continue;

                }
            }

            qDebug() << "selected map : " + QString::number(PainterHelper::SELECTED_CTNS_MAP.size());
        }
    }
    QGraphicsView::mousePressEvent(event);
}

void YardSectionView::mouseMoveEvent(QMouseEvent *event)
{
    QGraphicsView::mouseMoveEvent(event);
}

void YardSectionView::mouseReleaseEvent(QMouseEvent *event)
{    
    QGraphicsView::mouseReleaseEvent(event);
}

Slot::Slot(int height,int width)
    : m_height(height)
    , m_width(width)
    , m_fillColor(Qt::white)
    , m_key(0)
    , m_used(0)
    , m_size(0)
{
    m_rcBounding.setWidth(m_width);
    m_rcBounding.setHeight(m_height);
    setFlags(ItemIsSelectable);
    setAcceptHoverEvents(true);
}

Slot::~Slot(){

}

QRectF Slot::boundingRect() const
{
    return m_rcBounding;
}

void Slot::setStartPoint(const QPointF &pos)
{
    m_startPosScene = pos;
    qreal startX = m_startPosScene.x();
    qreal startY = m_startPosScene.y();
    setPos(startX,startY);
}

void Slot::setFillColor(const QColor &color)
{
    m_fillColor = color;
}

void Slot::setText(const QString &text)
{
    m_text = text;
}

void Slot::setKey(const int key)
{
    m_key = key;
}

void Slot::setName(const QString &name)
{
    m_name = name;
}

void Slot::setUsed(const int used)
{
    m_used = used;
}

int Slot::getKey()
{
    return m_key;
}

QString Slot::getName()
{
    return m_name;
}

int Slot::getUsed()
{
    return m_used;
}

int Slot::getSize()
{
    return m_size;
}

void Slot::setSize(int size)
{
    m_size = size;
}

void Slot::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(widget);

    painter->save();

    (option->state & QStyle::State_Selected) ? painter->setBrush(Qt::Dense6Pattern) : painter->setBrush(m_fillColor);

    painter->drawRect(m_rcBounding);

    if((m_used == 1) || (m_used == 2)){

        QFont font;
        font.setBold(true);
        font.setPixelSize(14);
        painter->setFont(font);

        QRect rect = option->rect;

        rect.adjust(4,4,-4,-4);

        painter->drawText(rect, Qt::TextWordWrap | Qt::AlignVCenter | Qt::AlignHCenter,m_text);

    }else if (m_used == 3){

        QFont font;
        font.setPixelSize(36);
        painter->setFont(font);

        QRect rect = option->rect;

        rect.adjust(4,4,-4,-4);

        painter->drawText(rect, Qt::TextWordWrap | Qt::AlignVCenter | Qt::AlignHCenter,"+");
    }




    painter->restore();

}

int Slot::type() const
{
    return UserType + 2;
}

Stack::Stack(int length,int width,int blockId,int rowId,int colId,QString blockName,int maxTier)
    : m_width(length)
    , m_length(width)
    , m_blockId(blockId)
    , m_rowId(rowId)
    , m_colId(colId)
    , m_closeFlag(0)
    , m_blockName(blockName)
    , m_maxTier(maxTier)
{
    m_rcBounding.setWidth(m_width);
    m_rcBounding.setHeight(m_length);
    setFlags(ItemIsSelectable);
    setAcceptHoverEvents(true);
    m_colorlist.insert(0,Qt::white);
}

Stack::~Stack()
{

}

QRectF Stack::boundingRect() const
{
    return m_rcBounding;
}

void Stack::setStartPoint(const QPointF &pos)
{
    m_startPosScene = pos;
    qreal startX = m_startPosScene.x();
    qreal startY = m_startPosScene.y();
    setPos(startX,startY);
}

QPointF Stack::getStartPoint()
{
    return m_startPosScene;
}

QPointF Stack::getEndPoint()
{
    m_endPosScene.setX(m_startPosScene.x()+m_width);
    m_endPosScene.setY(m_startPosScene.y()+m_length);
    return m_endPosScene;
}

void Stack::setFillColor(const int tier,const QColor &color)
{
    m_colorlist.insert(tier,color);
}

void Stack::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(widget);

    painter->save();

    int tier = 0;

    foreach (int key, m_colorlist.keys())
        tier<key ? tier = key : tier;

    (option->state & QStyle::State_Selected) ? painter->setBrush(QBrush(Qt::black,Qt::Dense6Pattern)) : painter->setBrush(m_colorlist.value(tier));

    if (m_closeFlag == 0){

        painter->drawRect(m_rcBounding);

    }else if (m_closeFlag == 1){

        painter->setPen(Qt::NoPen);
        painter->drawRect(m_rcBounding);
        painter->setPen(Qt::SolidLine);
        painter->drawLine(m_rcBounding.topLeft(),m_rcBounding.topRight());
        painter->drawLine(m_rcBounding.topLeft(),m_rcBounding.bottomLeft());
        painter->drawLine(m_rcBounding.bottomLeft(),m_rcBounding.bottomRight());
        painter->setPen(QPen(m_colorlist.value(tier),Qt::SolidLine));
        painter->drawLine(m_rcBounding.topRight(),m_rcBounding.bottomRight());

    }else if(m_closeFlag == 2){

        painter->setPen(Qt::NoPen);
        painter->drawRect(m_rcBounding);
        painter->setPen(Qt::SolidLine);
        painter->drawLine(m_rcBounding.topLeft(),m_rcBounding.topRight());
        painter->drawLine(m_rcBounding.topRight(),m_rcBounding.bottomRight());
        painter->drawLine(m_rcBounding.bottomLeft(),m_rcBounding.bottomRight());
        painter->setPen(QPen(m_colorlist.value(tier),Qt::SolidLine));
        painter->drawLine(m_rcBounding.topLeft(),m_rcBounding.bottomLeft());

    }else{

    }
    painter->restore();

}

int Stack::getBlockId(){
    return m_blockId;
}

int Stack::getRowId(){
    return m_rowId;
}

int Stack::getColId(){
    return m_colId;
}

int Stack::getMaxTier(){
    return m_maxTier;
}

void Stack::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    //setSelected(true);

    QGraphicsItem::mousePressEvent(event);
}

void Stack::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsItem::mouseMoveEvent(event);

}

void Stack::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsItem::mouseReleaseEvent(event);
}

int Stack::type() const
{
    return UserType + 1;
}


InfoRectangle::InfoRectangle()
    : m_rcBounding(0,0,0,0)
{

}

InfoRectangle::~InfoRectangle()
{

}

QRectF InfoRectangle::boundingRect() const
{
    return m_rcBounding;
}

void InfoRectangle::setStartPoint(const QPointF &pos)
{
    m_rcBounding.setTopLeft(pos);
    m_rcBounding.setWidth(200);
    m_rcBounding.setHeight(250);
}

void InfoRectangle::setText(const QString &text)
{
    m_text = text;
}


void InfoRectangle::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(widget);
    painter->save();

    painter->setPen(Qt::NoPen);

    QColor color = QColor(Qt::gray);
    color.setAlpha(200);

    painter->setBrush(color);
    painter->drawRect(m_rcBounding);

    QFont font;
    font.setBold(true);
    font.setPixelSize(14);
    painter->setFont(font);

    QRect rect = option->rect;

    rect.adjust(4,4,-4,-4);

    painter->setPen(Qt::SolidLine);

    painter->drawText(rect, Qt::TextWordWrap | Qt::AlignVCenter | Qt::AlignHCenter,m_text);

    painter->restore();

}
