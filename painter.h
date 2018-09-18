#ifndef PAINTER_H
#define PAINTER_H

#ifdef WIN32
#pragma execution_character_set("utf-8")
#endif

#include <QtWidgets>
#include <QVariant>
#include <QSqlRecord>
#include "sqltablemodel.h"

#define FLAG_POINTER 0x01
#define FLAG_CHECK 0x02
#define FLAG_INFO 0x04

class Slot : public QGraphicsItem
{
public:
    Slot(int height,int width);
    ~Slot();
    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *item, QWidget *widget) override;
    void setStartPoint(const QPointF &pos);
    QPointF getStartPoint();
    QPointF getEndPoint();
    void setFillColor(const QColor &color);
    void setText(const QString &text);
    void setKey(const int key);
    void setName(const QString &name);
    void setUsed(const int used);
    void setSize(const int size);
    int getKey();
    QString getName();
    int getUsed();
    int getSize();
protected:

    QRectF m_rcBounding;
    int type() const;

private:
    int m_width;
    int m_height;
    QString m_text;
    int m_key;
    int m_size; //0: 无箱; 1:key是小箱;  2:key是大箱
    QColor m_fillColor;
    QPointF m_startPosScene;
    QPointF m_endPosScene;
    QString m_name;
    int m_used; // 0: 空位; 1: 小箱占用; 2 大箱前占用; 3 大箱后占用
};

class Stack : public QGraphicsItem
{
public:
    Stack(int length,int width,int blockId,int rowId,int colId,QString blockName,int maxTier);
    ~Stack();
    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *item, QWidget *widget) override;
    void setStartPoint(const QPointF &pos);
    QPointF getStartPoint();
    QPointF getEndPoint();
    void setFillColor(const int tier,const QColor &color);
    int getBlockId();
    int getRowId();
    int getColId();
    int getMaxTier();
    int m_closeFlag; //0 放小箱的，两端封闭；1 放大箱的，前端封闭，后端开放；2 放大箱的，后端封闭，前端开放
    QHash<int,QColor> m_colorlist;
    QString m_blockName;
protected:

    QRectF m_rcBounding;
    int type() const;
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

private:
    int m_width;
    int m_length;
    int m_blockId;
    int m_rowId;
    int m_colId;  
    int m_maxTier;
    QPointF m_startPosScene;
    QPointF m_endPosScene;

};

class InfoRectangle : public QGraphicsItem
{
public:
    InfoRectangle();
    ~InfoRectangle();

    QRectF boundingRect() const override;

    void setStartPoint(const QPointF &pos);

    void setText(const QString &text);

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

protected:

    QRectF m_rcBounding;
    QString m_text;

};

class YardSatelliteView : public QGraphicsView
{
    Q_OBJECT

public:
    YardSatelliteView(QWidget *parent = 0);
    ~YardSatelliteView();
    void setZoomDelta(qreal delta);
    qreal zoomDelta() const;

protected:

    void wheelEvent(QWheelEvent *event) override;



public slots:

    void zoomIn();  // 放大
    void zoomOut();  // 缩小
    void zoom(float scaleFactor);

private:

    qreal m_zoomDelta;  // 缩放的增量
    qreal m_scale;  // 缩放值

};



class PainterScene : public QGraphicsScene
{
    Q_OBJECT
public:
    PainterScene(SqlTableModel *model,QObject * parent = 0);
    ~PainterScene();
    SqlTableModel * getModel();
public slots:


protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

private:
    SqlTableModel *m_model;

};

class PainterHelper
{
public:

    static unsigned int SELECTED;
    static QMap<QString,Stack *> STACK_MAP;
    static QMap<int,QString> ROW_20_MAP;
    static QMap<int,QString> ROW_40_MAP;
    static QMap<int,QString> COL_MAP;
    static QMap<int,QString> TIER_MAP;
    static QMap<QString,QString> COLOR_MAP;
    static QMap<int,int> SELECTED_CTNS_MAP;

    static void initMap();

};

class YardSectionView : public QGraphicsView
{
    Q_OBJECT

public:
    YardSectionView(SqlTableModel *model,QWidget *parent = 0);
    ~YardSectionView();
    void fillSlotContainers(QString pos,QColor color,QString text,int key,int used,int size);
    void clearSlotContainers(QString pos);
    QMap<QString,Slot *> * getSLOTMAP();

signals:
    void put(const QString &str);
    void info(const QPoint &pos,const int &key);

protected:

    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

public slots:

    //void onRowInserted(const QSqlRecord &record);
    void onRowUpdated(const QSqlRecord &oldrecord,const QSqlRecord &newrecord);
    //void onRowDeleted(const QSqlRecord &record);

private:

    PainterScene *m_scene;
    QMap<QString,Slot *> *m_SLOT_MAP;

};


#endif // PAINTER_H
