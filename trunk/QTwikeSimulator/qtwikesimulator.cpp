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

#include "qtwikesimulator.h"
#include <QTimer>
#include <QCloseEvent>

#include "load.h"
#include "powersupply.h"
#include "battery.h"
#include "balancer.h"


QTwikeSimulator::QTwikeSimulator(QWidget *parent)
    : QWidget(parent)
{
	ui.setupUi(this);
	
	load = new Load();
	load->show();
	
	battery = new Battery();
	battery->show();
	
	powerSupply = new PowerSupply();
	powerSupply->show();
	
	balancer = new Balancer(*battery, *powerSupply);
	balancer->show();
	
	timer = new QTimer(this);
	timer->setInterval(10);
	connect(timer, SIGNAL(timeout()), this, SLOT(timeout()));
	
	ui.button_stop->setEnabled(false);
	
	ui.lcdVoltage->setSegmentStyle(QLCDNumber::Flat);
	ui.lcdVoltage->setNumDigits(5);
	
	ui.lcdCurrent->setSegmentStyle(QLCDNumber::Flat);
	ui.lcdCurrent->setNumDigits(5);
	
	ui.lcdCharge->setSegmentStyle(QLCDNumber::Flat);
	ui.lcdCharge->setNumDigits(5);
	
	ui.lcdTemperature->setSegmentStyle(QLCDNumber::Flat);
	//ui.lcdTemperature->setNumDigits(3);
	
	ui.lcdSym->setSegmentStyle(QLCDNumber::Flat);
	
}

QTwikeSimulator::~QTwikeSimulator()
{
	delete timer;
	delete balancer;
	delete load;
	delete powerSupply;
	delete battery;
}

void QTwikeSimulator::on_button_start_clicked(void)
{
	if (ui.radio_charge->isChecked())
	{
		powerSupply->on();
	}
	timer->start();
	ui.button_start->setEnabled(false);
	ui.button_stop->setEnabled(true);
}

void QTwikeSimulator::on_button_stop_clicked(void)
{
	timer->stop();
	ui.button_stop->setEnabled(false);
	ui.button_start->setEnabled(true);
}

void QTwikeSimulator::on_radio_drive_toggled(bool state)
{
	if (state)
	{
		powerSupply->off();
		load->on();
		load->show();
	}
}
void QTwikeSimulator::on_radio_off_toggled(bool state)
{
	if (state)
	{
		powerSupply->off();
		load->off();
	}
}
void QTwikeSimulator::on_radio_charge_toggled(bool state)
{
	if (state)
	{
		load->off();
		powerSupply->on();
		powerSupply->show();
	}
}


void QTwikeSimulator::timeout(void)
{
	battery->sample( timer->interval() );
	balancer->sample( timer->interval() );
	
	double voltage = battery->getVoltage();
	
	double sourceCurrent = powerSupply->getCurrent(voltage);
	double drainCurrent = load->getCurrent(voltage);
	
	double totalCurrent = sourceCurrent-drainCurrent; 
	battery->setCurrent(totalCurrent);
	
	
	ui.lcdVoltage->display(voltage);
	ui.lcdCurrent->display(totalCurrent);
	ui.lcdCharge->display( battery->getCapacity());
	ui.lcdSym->display(battery->getSymVoltage());
}

void QTwikeSimulator::closeEvent(QCloseEvent *event)
{
	event->accept();
	delete balancer;
	delete load;
	delete powerSupply;
	delete battery;
}
