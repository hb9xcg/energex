/***************************************************************************
 *   Copyright (C) 2008-2010 by Markus Walser                              *
 *   markus.walser@gmail.com                                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
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
