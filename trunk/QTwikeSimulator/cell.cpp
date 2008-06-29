/***************************************************************************
 *   Copyright (C) 2008 by Markus Walser                                   *
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

#include "cell.h"
#include <QtGui>

const double Cell::maximalVoltage    = 4.3;   // [Volt]
const double Cell::nominalVoltage    = 4.2;   // [Volt]
const double Cell::lowVoltage        = 2.7;   // [Volt]
const double Cell::nominalRi         = 0.005; // [Ohm]
const double Cell::rBalancer         = 7.5;   // [Ohm]
const double Cell::RiDeviation       = 10.0;  // [%]
const double Cell::capacityDeviation = 10.0;  // [%]
const int    Cell::maxHeight         = 200;   // [pixel]


Cell::Cell(int xPosition, int yPosition, int address, QWidget *parent)
    : QWidget(parent),
      m_xPosition(xPosition),
      m_yPosition(yPosition),
      m_address(address)
{
	ui.setupUi(this);
	brushFill = QBrush(QColor(0, 255, 0));
	
	nominalCapacity    = 11.0;
	actualCapacity     = 0.0;
	current            = 0.0;
	iBalancer          = 0.0;
	toolTipRefresh     = 0;
	balancerPercentage = 0;
	balanceCounter     = 0;
	balancerOn         = false;
	
	// Add a deviation specific for each cell.
	double maxCapacityDevAbs = capacityDeviation*nominalCapacity/100.0;
	double capacityDevAbs = (maxCapacityDevAbs*qrand()/(RAND_MAX));
	capacityDevAbs -= maxCapacityDevAbs/2;
	nominalCapacity += capacityDevAbs;
	
	double maxRiDeviationAbs = RiDeviation * nominalRi / 100.0;
	double RiDeviationAbs = (maxRiDeviationAbs*qrand()/(RAND_MAX));
	RiDeviationAbs -= maxRiDeviationAbs / 2;
	Ri = nominalRi + RiDeviationAbs;
	
	updateCharge(11.0*0.6);
}

Cell::~Cell()
{

}

void Cell::drawVoltage(void)
{
	int red;
	int green;
	
	if (voltage < lowVoltage)
	{
		red   = 255;
		green = 0;
	}
	else if (voltage < 3.4)
	{
		red = 255;
		green = 255;
	}
	else if (voltage < 4.15)
	{
		red = 0;
		green = 255;
	}
	else if (voltage < nominalVoltage)
	{
		red = 255;
		green = 255;
	}
	else
	{
		red   = 255;
		green = 0;
	}
	
	brushFill = QBrush(QColor(red, green, 0));
		
	int barHeight = voltage / maximalVoltage * double(maxHeight);
	move(m_xPosition, m_yPosition+maxHeight-barHeight);
	setFixedHeight(barHeight-1);
	
	if (--toolTipRefresh<0)
	{
		toolTipRefresh = 10;
		
		// Update Tooltip
		QString info( QString("Address:  %1\nVoltage:  %2V\nCurrent:  %3A\nCapacity: %4Ah")
		        .arg(m_address).arg(voltage).arg(current-iBalancer).arg(actualCapacity));
		setToolTip(info);
	}
}

int Cell::get42YPos(void)
{
	return maxHeight - nominalVoltage / maximalVoltage * maxHeight;
}

int Cell::get27YPos(void)
{
	return maxHeight - lowVoltage / maximalVoltage * maxHeight;
}

int Cell::get0YPos(void)
{
	return maxHeight;
}

void Cell::paintEvent(QPaintEvent *event)
{
	QPainter painter;
	painter.begin(this);
	painter.setRenderHint(QPainter::Antialiasing);

	painter.fillRect(event->rect(), brushFill);
	if (balancerOn)
	{
		painter.setPen(Qt::blue);
		painter.drawEllipse(1, height()-5, 2, 2);
	}
	painter.end();
} 

void Cell::sample(int ms)
{
	iBalancer = 0.0;
	
	if( balancerOn != (balanceCounter < balancerPercentage))
	{
		balancerOn = (balanceCounter <= balancerPercentage);
		update();
	}
	if (balancerOn)
	{
		iBalancer = voltage / rBalancer;
	}
	
	balanceCounter += 1;
	if (balanceCounter > 100)
	{
		balanceCounter = 0;
	}
	
	double charge = (current-iBalancer) /3600.0 * ms / 1000.0;
	updateCharge(charge);
}

void Cell::updateCharge(double Ah)
{
	if (actualCapacity + Ah < nominalCapacity && 
		actualCapacity + Ah > 0)
	{
		actualCapacity += Ah;
	}
	
	voltage = 2.7 + (4.2 - 2.7) * actualCapacity/nominalCapacity;
	voltage += (current-iBalancer) * Ri;

	drawVoltage();
}

void Cell::switchBalancer(int percentage)
{
	balancerPercentage = percentage;
}
