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

#include <QSettings>

double PowerSupply::prechargeCurrent = 0.7;
double PowerSupply::fastCurrent      = 5.0;
double PowerSupply::normalCurrent    = 2.0;
double PowerSupply::symCurrent       = 0.6;

PowerSupply::PowerSupply(QWidget *parent)
    : QWidget(parent)
{
	ui.setupUi(this);

	eState = eInvalid;
	
	ui.lcdVoltage->setSegmentStyle(QLCDNumber::Flat);
	ui.lcdCurrent->setSegmentStyle(QLCDNumber::Flat);
	ui.lcdPower->setSegmentStyle(QLCDNumber::Flat);
	
	QSettings settings;

    settings.beginGroup("PowerSupply");
    restoreGeometry( settings.value("geometry").toByteArray() );
    
    prechargeCurrent = settings.value("prechargeCurrent", 0.7).toDouble();    
    normalCurrent    = settings.value("normalCurrent",    5.0).toDouble();
    fastCurrent      = settings.value("fastCurrent",     10.0).toDouble();
    symCurrent       = settings.value("symCurrent",       2.0).toDouble();
	
    settings.endGroup();
	
	ui.spinboxPreCharge->setMinimum(0.0);
	ui.spinboxPreCharge->setMaximum(2.0);
	ui.spinboxPreCharge->setValue(prechargeCurrent);
	
	ui.spinboxNormalCharge->setMinimum(0.0);
	ui.spinboxNormalCharge->setMaximum(10.0);
	ui.spinboxNormalCharge->setValue(normalCurrent);
	
	ui.spinboxQuickCharge->setMinimum(0.0);
	ui.spinboxQuickCharge->setMaximum(16.0);
	ui.spinboxQuickCharge->setValue(fastCurrent);
	
	ui.spinboxSymCharge->setMinimum(0.0);
	ui.spinboxSymCharge->setMaximum(2.0);
	ui.spinboxSymCharge->setValue(symCurrent);
	
	switchState(eOff);
}

PowerSupply::~PowerSupply()
{
	QSettings settings;

	settings.beginGroup("PowerSupply");
    
	settings.setValue("prechargeCurrent", prechargeCurrent);
	settings.setValue("normalCurrent",    normalCurrent);
	settings.setValue("fastCurrent",      fastCurrent);
	settings.setValue("symCurrent",       symCurrent);
	settings.setValue("geometry",         saveGeometry());
    
    settings.endGroup();
}

void PowerSupply::on_spinboxPreCharge_valueChanged(void)
{
	prechargeCurrent = ui.spinboxPreCharge->value();
}

void PowerSupply::on_spinboxNormalCharge_valueChanged(void)
{
	normalCurrent = ui.spinboxNormalCharge->value();
}

void PowerSupply::on_spinboxQuickCharge_valueChanged(void)
{
	fastCurrent = ui.spinboxQuickCharge->value();
}

void PowerSupply::on_spinboxSymCharge_valueChanged(void)
{
	symCurrent = ui.spinboxSymCharge->value();
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
	symTime = 30*60*1000;
}

void PowerSupply::voltageToHigh(void)
{
	if (current > 0)
	{
		current -= 0.1;
		switchState(eU_Ladung);
	}
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
			//current += 0.1;
		}
		break;
	case eSym_Ladung:
		current = symCurrent;
		if (symTime < 0)
		{
			switchState(eErh_Ladung);
		}
		break;
	case eErh_Ladung:
		current = 0.0;
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
	if (eState == eSym_Ladung)
	{
		symTime -= ms;
	}
}

