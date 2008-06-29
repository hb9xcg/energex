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

Balancer::Balancer(Battery& pBattery, PowerSupply& ppowerSupply, QWidget *parent)
    : QWidget(parent),
      battery(pBattery),
      powerSupply(ppowerSupply)
{
	ui.setupUi(this);
	time = 0;
	on = false;
}

Balancer::~Balancer()
{

}

void Balancer::on_radio_off_toggled(bool state)
{
	if (state)
	{
		on = false;
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
		on = true;
		time = 10000;
	}
}

void Balancer::sample(int ms)
{
	time += ms;
	if (time > 10000 && on)
	{
		time = 0;
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
			if (voltage > 4.175)
			{
				powerSupply.voltageToHigh();
			}
		}
		
		if (minVoltage>4.175)
		{
			powerSupply.battFull();
		}
		
		double gap = (maxVoltage - minVoltage) / 10.0;
		for (int addr=0; addr<nbrOfCells; addr++)
		{
			double voltage;
			int percentage = 0;
			for (voltage=minVoltage; voltage<=maxVoltage; voltage+=gap)
			{
				if (battery.getCellVoltage(addr) >= voltage && 
					battery.getCellVoltage(addr) <  voltage+gap	)
				{
					battery.switchCellBalancer(addr, percentage);
				}
				percentage += 10;
			}
		}
	}
}
