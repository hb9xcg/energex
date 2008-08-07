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
#include "qtwikebms.h"
#include "ReceiverThread.h"
#include "battery.h"
#include "protocol.h"

QTwikeBMS::QTwikeBMS(QWidget *parent)
    : QWidget(parent)
{
	ui.setupUi(this);
	
	receiver = new ReceiverThread();
	
	connect(ui.radio_on,  SIGNAL(clicked()), receiver, SLOT(start()));
	connect(ui.radio_off, SIGNAL(clicked()), receiver, SLOT(stop()));
	
	connect(receiver, SIGNAL(updateLog(const QString&)), ui.textField, SLOT(insertPlainText(const QString&)));
}

QTwikeBMS::~QTwikeBMS()
{
	delete receiver;
}

void QTwikeBMS::on_spinVoltage_valueChanged(double floatValue)
{
	uint16_t fixedValue = (uint16_t)(floatValue * 100.0);
	battery1.voltage = fixedValue;
	battery2.voltage = fixedValue;
	battery3.voltage = fixedValue;
}

void QTwikeBMS::on_spinCurrent_valueChanged(double floatValue)
{
	uint16_t fixedValue = (uint16_t)(floatValue * 100.0);
	battery1.current = fixedValue;
	battery2.current = fixedValue;
	battery3.current = fixedValue;
}

void QTwikeBMS::on_spinCharge_valueChanged(double floatValue)
{
	uint16_t fixedValue = (uint16_t)(floatValue * 100.0);
	battery1.ah_counter = fixedValue;
	battery2.ah_counter = fixedValue;
	battery3.ah_counter = fixedValue;
}

