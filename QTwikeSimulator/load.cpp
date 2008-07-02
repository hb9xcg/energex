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

#include "load.h"
#include <QSettings>

Load::Load(QWidget *parent)
    : QWidget(parent)
{
	ui.setupUi(this);
	
	eState = eInvalid;

	ui.lcdVoltage->setSegmentStyle(QLCDNumber::Flat);
	ui.lcdCurrent->setSegmentStyle(QLCDNumber::Flat);
	ui.lcdPower->setSegmentStyle(QLCDNumber::Flat);
	
	switchState(eOff);
	
	QSettings settings;
    settings.beginGroup("Load");
    restoreGeometry( settings.value("geometry").toByteArray() );
    settings.endGroup();
}

Load::~Load()
{
	QSettings settings;
	settings.beginGroup("Load");
	settings.setValue("geometry", saveGeometry());    
    settings.endGroup();
}

void Load::switchState(EState eState)
{
	if (this->eState == eState)
	{
		return;
	}
	
	this->eState = eState;
	switch(eState)
	{
	case eOff:
		ui.label_state->setText("Off");
		break;
	case eOn:
		ui.label_state->setText("On");
		break;
	case eEmpty:
		ui.label_state->setText("Leer");
		break;
	case eInvalid:
		ui.label_state->setText("Invalid");
		break;
	}
}

double Load::getCurrent(double voltage)
{
	double current;
	if (eState == eOn )
	{
		current = 12.0;
	}
	else
	{
		current = 0.0;
	}
	
	ui.lcdCurrent->display(current);
	ui.lcdVoltage->display(voltage);
	ui.lcdPower->display(current*voltage);
	
	return current;
}

void Load::on(void)
{
	switchState(eOn);
}

void Load::off(void)
{
	switchState(eOff);
}

void Load::battEmpty(void)
{
	switchState(eEmpty);
}

void Load::sample(int ms)
{
	
}
