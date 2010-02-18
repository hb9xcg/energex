#include "voltageitem.h"
#include <QPainter>
#include <QRgb>

VoltageItem::VoltageItem(QRectF boundings) :
        boundings(boundings)
{

}

void VoltageItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    painter->setPen(Qt::blue);

    QList<int> timeKeys = samples.keys();

    int endTime = 0;

    foreach(int sample, timeKeys)
    {
        endTime = qMax(endTime, sample);
    }

    float dT = boundings.width()  / endTime;
    float dU = boundings.height() / 435.0;

    QPainterPath path;

    qSort(timeKeys.begin(), timeKeys.end());

    int x = boundings.left() + 0 * dT;
    float y = boundings.bottom() - samples[0] * dU;

    path.moveTo(x, y);


    foreach(int ms, timeKeys)
    {
        x = boundings.left() + ms * dT;
        y = boundings.bottom() - samples[ms] * dU;
        path.lineTo(x, y);
    }

    painter->drawPath(path);

}
void VoltageItem::addSample(int ms, float voltage)
{
    samples[ms] = voltage;
}

QRectF VoltageItem::boundingRect() const
{
    return boundings;
}
