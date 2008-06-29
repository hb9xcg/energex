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

#include "powersupply.h"

const double PowerSupply::prechargeCurrent = 0.7;
const double PowerSupply::fastCurrent      = 5.0;
const double PowerSupply::normalCurrent    = 2.0;
const double PowerSupply::symCurrent       = 0.6;

PowerSupply::PowerSupply(QWidget *parent)
    : QWidget(parent)
{
	ui.setupUi(this);
	ticks = 0;
	eState = eInvalid;
	
	ui.lcdVoltage->setSegmentStyle(QLCDNumber::Flat);
	ui.lcdCurrent->setSegmentStyle(QLCDNumber::Flat);
	ui.lcdPower->setSegmentStyle(QLCDNumber::Flat);
	
	switchState(eOff);
}

PowerSupply::~PowerSupply()
{

}

void PowerSupply::on(void)
{
	switchState(eLadeBereit);
}

void PowerSupply::off(void)
{
	switchState(eOff);
}

void PowerSupply::battFull(void)
{
	switchState(eSym_Ladung);
	startSym = ticks;
}

void PowerSupply::voltageToHigh(void)
{
	current -= 0.1;
}

void PowerSupply::switchState(EState eState)
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
	case eLadeBereit:
		ui.label_state->setText("Ladebereit");
		break;
	case eVorladung:
		ui.label_state->setText("Vorladen");
		break;
	case eI_SchnellLadung:
		ui.label_state->setText("I-Ladung Schnell");
		break;
	case eI_NormalLadung:
		ui.label_state->setText("I-Ladung Normal");
		break;
	case eU_Ladung:			
		ui.label_state->setText("U-Ladung");
		break;
	case eSym_Ladung:
		ui.label_state->setText("Sym-Ladung");
		break;
	case eErh_Ladung:
		ui.label_state->setText("Erhaltungs-Ladung");
		break;
	case eInvalid:
		ui.label_state->setText("Invalid");
		break;
	}
}

double PowerSupply::getCurrent(double voltage)
{
	switch(eState)
	{
	case eOff:
		current = 0.0;
		break;
	case eLadeBereit:
		current = 0.0;
		if (voltage<336.0)
		{
			switchState(eVorladung);
		}
		else if (voltage < 410.0)
		{
			switchState(eI_SchnellLadung);
		}
		else if (voltage < 435.0)
		{
			switchState(eI_NormalLadung);
		}
		else
		{
			switchState(eU_Ladung);
		}
		break;
	case eVorladung:
		current = prechargeCurrent;
		if (voltage>336.0)
		{
			switchState(eI_SchnellLadung);
		}
		break;
	case eI_SchnellLadung:
		current = fastCurrent;
		if (voltage>410.0)
		{
			switchState(eI_NormalLadung);
		}
		break;
	case eI_NormalLadung:
		current = normalCurrent;
		if (voltage>435.0)
		{
			switchState(eU_Ladung);
		}
		break;
	case eU_Ladung:
		if (voltage>435.0)
		{
			current -= 0.1;
		}
		else
		{
			current += 0.1;
		}
		break;
	case eSym_Ladung:
		current = symCurrent;
		if (ticks-startSym > 30*60*1000)
		{
			switchState(eErh_Ladung);
		}
		break;
	case eErh_Ladung:
		break;
	case eInvalid:
		break;
	}
	
	ui.lcdCurrent->display(current);
	ui.lcdVoltage->display(voltage);
	ui.lcdPower->display(current*voltage);
	
	return current;
}

void PowerSupply::sample(int ms)
{
	ticks += ms;
}

