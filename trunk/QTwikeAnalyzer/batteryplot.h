#ifndef BATTERYPLOT_H
#define BATTERYPLOT_H

#include <QObject>
#include <QMap>
#include <QVector>
#include <QTimer>
#include <qwt/qwt_plot.h>

class QwtPlotCurve;

class BatteryPlot : public QwtPlot
{
    Q_OBJECT

    QVector<double> timeT;
    QVector<double> T;
    QMap<int, float> temperatureSensors;

    QVector<double> time;
    QVector<double> u;
    QVector<double> i;

    QwtPlotCurve *curveVoltage;
    QwtPlotCurve *curveCurrent;
    QwtPlotCurve *curveTemperature;

    QTimer repaintTimer;

    int batteryIndex;

public:
    BatteryPlot(QWidget* parent, int battery);
    void clear();

public slots:
    void addPower(int ms, float voltage, float current);
    void temperature(int ms, int sensor, float temperature);
    void update ();
};

#endif // BATTERYPLOT_H
