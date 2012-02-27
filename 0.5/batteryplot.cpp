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
#include "batteryplot.h"
#include <qwt/qwt_plot.h>
#include <qwt/qwt_plot_curve.h>
#include <qwt/qwt_legend.h>
#include <qwt/qwt_plot_grid.h>
#include <cstdio>

BatteryPlot::BatteryPlot(QWidget* parent, int battery) :
        QwtPlot(parent),
        batteryIndex(battery)
{
    setLineWidth(2);
    setMidLineWidth(3);

    setTitle(QString("Battery %1").arg(battery));

    setAxisTitle(yLeft, "Voltage [V]");
    setAxisScale(yLeft, 250, 450);

    //setAxisTitle(yRight, QString("Current [A] / Temperature [10°C]"));
    setAxisTitle(yRight, QString("Current[A]"));
    setAxisScale(yRight, -7, 7);
    enableAxis(yRight, true);

    setAxisTitle(xBottom, "Time [s]");
    setAxisScale(xBottom, 0, 120);

    curveVoltage = new QwtPlotCurve("u(t)");
    curveVoltage->attach(this);
    curveVoltage->setPen(QPen(Qt::blue));

    curveCurrent = new QwtPlotCurve("i(t)");
    curveCurrent->attach(this);
    curveCurrent->setPen(QPen(Qt::red));

    curveTemperature = new QwtPlotCurve("T(t)");
    curveTemperature->attach(this);
    curveTemperature->setPen(QPen(Qt::green));

    // legend
    insertLegend(new QwtLegend(), QwtPlot::LeftLegend);

    // grid
    QwtPlotGrid *grid = new QwtPlotGrid;
    grid->enableXMin(true);
    grid->setMajPen(QPen(Qt::white, 0, Qt::DotLine));
    grid->setMinPen(QPen(Qt::gray, 0 , Qt::DotLine));
    grid->attach(this);

    setAutoReplot(true);

    connect(&repaintTimer, SIGNAL(timeout()), this, SLOT(update()));
    repaintTimer.start(100);
}

void BatteryPlot::addPower(int ms, float voltage, float current)
{
    u.append(voltage);
    i.append(current*200.0/14.0+350.0); // scale to voltage range
    //time.append(double(ms)*120.0/4000.0);
    time.append(double(ms)/1000.0);  
}

void BatteryPlot::temperature(int ms, int sensor, float temperature)
{
    if (temperatureSensors.contains(sensor))
    {
        double average = 0.0;
        foreach( float value, temperatureSensors)
        {
            average += value;
        }
        average /= temperatureSensors.size();
        temperatureSensors.clear();
        temperatureSensors.insert(sensor, temperature);

        timeT.append(double(ms)/1000.0);
        T.append(average*200.0/140.0+350.0);
    }
    else
    {
        temperatureSensors.insert(sensor, temperature);
    }
}

void BatteryPlot::update()
{
    if (!time.empty())
    {
        // Copy the data to the plot
        curveVoltage->setSamples(time, u);
        curveCurrent->setSamples( time, i);
        curveTemperature->setSamples( timeT, T);

        setAxisScale(xBottom, 0, time.last());
        repaint();
    }
}

void BatteryPlot::clear()
{
    timeT.clear();;
    T.clear();
    temperatureSensors.clear();;

    time.clear();;
    u.clear();
    i.clear();
}
