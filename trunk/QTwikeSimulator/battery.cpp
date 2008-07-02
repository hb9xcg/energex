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

#include <QtGui>
#include "battery.h"
#include <QSettings>

const int Battery::yOffset = 25;

Battery::Battery(QWidget *parent)
    : QWidget(parent)
{
	ui.setupUi(this);
	voltage = 0.0;
	current = 0.0;
	capacity = 0.0;
	setFixedHeight(250);
	setFixedWidth(700);
	
	for (int i=0; i<100; i++)
	{
		m_vecCells.push_back( new Cell(50+i*6, yOffset, i, this) );
		voltage += m_vecCells[i]->getVoltage();
	}
	
	QSettings settings;
    settings.beginGroup("Battery");
    restoreGeometry( settings.value("geometry").toByteArray() );
    settings.endGroup();
}

Battery::~Battery()
{
	QSettings settings;
	settings.beginGroup("Battery");
	settings.setValue("geometry",         saveGeometry());    
    settings.endGroup();
    
	for (int i=0; i<100; i++)
	{
		delete m_vecCells[i];
	}
}

int Battery::getNbrOfCells(void)
{
	return m_vecCells.size();
}

double Battery::getCellVoltage(int address)
{
	double doubleVoltage = m_vecCells[address]->getVoltage();
	doubleVoltage *= 1000.0;

	// Simulate 10bit ADC
	for(int mV=0; mV<4300; mV+=5)
	{
		if (mV >= doubleVoltage)
		{
			doubleVoltage = mV;
			break;
		}
	}
	
	return doubleVoltage/1000.0;
}

void Battery::switchCellBalancer(int address, int percentage)
{
	m_vecCells[address]->switchBalancer(percentage);
}

double Battery::getSymVoltage(void)
{
	double minVoltage=9.0, maxVoltage=0.0;
	
	for (int i=0; i<m_vecCells.size(); i++)
	{
		double voltage = m_vecCells[i]->getVoltage();
		if (voltage < minVoltage)
		{
			minVoltage = voltage;
		}
		if (voltage > maxVoltage)
		{
			maxVoltage = voltage;
		}
	}
	return maxVoltage-minVoltage;
}

void Battery::paintEvent(QPaintEvent *event)
{
	(void)event;
	QPainter painter;
	painter.begin(this);
	painter.setRenderHint(QPainter::Antialiasing);
//	painter.setCompositionMode(QPainter::CompositionMode_DestinationOver);
	
	int yPos42 = yOffset + Cell::get42YPos();
	int yPos27 = yOffset + Cell::get27YPos();
	int yPos0 = yOffset + Cell::get0YPos();
	
	QLineF line42Volt(45.0, yPos42, 655.0, yPos42);
	QLineF line27Volt(45.0, yPos27, 655.0, yPos27);
	QLineF line0Volt(45.0,  yPos0,  655.0, yPos0);
	
	painter.setPen(Qt::black);
	painter.drawLine(line42Volt);
	painter.drawLine(line27Volt);
	painter.drawLine(line0Volt);
	
	painter.drawText(5, yPos42+4, QString("4.2V"));
	painter.drawText(5, yPos27+4, QString("2.7V"));
	painter.drawText(15, yPos0+4, QString("0V"));
	
	painter.end();
}

void Battery::sample(int ms)
{
	voltage = 0.0;
	for (int i=0; i<100; i++)
	{
		m_vecCells[i]->setCurrent(current);
		m_vecCells[i]->sample(ms);
		voltage += m_vecCells[i]->getVoltage();
	}
	
	capacity += current / 3600.0 * ms / 1000.0;
}
