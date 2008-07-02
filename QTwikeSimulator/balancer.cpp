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

#include "balancer.h"
#include "battery.h"
#include "powersupply.h"
#include <QTimer>
#include <QSettings>

const double Balancer::upperVoltage = 4.200;
const double Balancer::underVoltage = 4.175;

Balancer::Balancer(Battery& pBattery, PowerSupply& ppowerSupply, QWidget *parent)
    : QWidget(parent),
      battery(pBattery),
      powerSupply(ppowerSupply)
{
	ui.setupUi(this);
	time = 0;
	on = false;
	
	timer = new QTimer(this);
	timer->setInterval(10000);
	connect(timer, SIGNAL(timeout()), this, SLOT(timeout()));
	
	QSettings settings;
    settings.beginGroup("Balancer");
    restoreGeometry( settings.value("geometry").toByteArray() );
    settings.endGroup();
}

Balancer::~Balancer()
{
	delete timer;
	
	QSettings settings;
	settings.beginGroup("Balancer");
	settings.setValue("geometry",         saveGeometry());    
    settings.endGroup();
}

void Balancer::on_radio_off_toggled(bool state)
{
	if (state)
	{
		timer->stop();
		for (int addr=0; addr<battery.getNbrOfCells(); addr++)
		{
			battery.switchCellBalancer(addr, 0);
		}
	}
}

void Balancer::on_radio_on_toggled(bool state)
{
	if (state)
	{
		timer->start();
		timeout();
	}
}

void Balancer::timeout(void)
{
	double minVoltage=9.0, maxVoltage=0.0;
	int nbrOfCells = battery.getNbrOfCells();
	for (int addr=0; addr<nbrOfCells; addr++)
	{
		double voltage = battery.getCellVoltage(addr);
		if (voltage < minVoltage)
		{
			minVoltage = voltage;
		}
		if (voltage > maxVoltage)
		{
			maxVoltage = voltage;
		}
	}
	if (maxVoltage > upperVoltage)
	{
		powerSupply.voltageToHigh();
	}
	if (minVoltage > underVoltage)
	{
		powerSupply.battFull();
	}
	
	double gap = (maxVoltage - minVoltage) / 11.0;
	
	for (int addr=0; addr<nbrOfCells; addr++)
	{
		if (gap > 0)
		{
			double voltage;
			int percentage = 0;
			for (voltage=minVoltage; voltage<=maxVoltage; voltage+=gap)
			{
				if (battery.getCellVoltage(addr) >= voltage && 
					battery.getCellVoltage(addr) <  voltage+gap	)
				{
					battery.switchCellBalancer(addr, percentage);
					break;
				}
				percentage += 10;
			}
		}
		else
		{
			battery.switchCellBalancer(addr, 0);
		}
	}
}
