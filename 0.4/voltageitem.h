#ifndef VOLTAGEITEM_H
#define VOLTAGEITEM_H

#include <QGraphicsItem>
#include <QMap>
#include <QRectF>

class Color;

class VoltageItem : public QGraphicsItem
{
    QMap<int,float> samples;
    QRectF boundings;

public:
    VoltageItem(QRectF boundings);

    virtual QRectF boundingRect() const;

    virtual void paint(QPainter *painter,
                       const QStyleOptionGraphicsItem *option,
                       QWidget *widget = 0);

    void addSample(int ms, float voltage);
};

#endif // VOLTAGEITEM_H
