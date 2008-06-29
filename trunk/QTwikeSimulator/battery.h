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

#ifndef BATTERY_H
#define BATTERY_H

#include <QtGui/QWidget>
#include "ui_battery.h"
#include "cell.h"

class Battery : public QWidget
{
    Q_OBJECT

public:
    Battery(QWidget *parent = 0);
    ~Battery();
    void sample(int ms);
    void setCurrent(double current) { this->current = current; };
    double getVoltage(void) { return voltage; };
    double getCapacity(void) { return capacity; };
    int getNbrOfCells(void);
    double getCellVoltage(int address);
    void switchCellBalancer(int address, int percentage);
    double getSymVoltage(void);

protected:
	void paintEvent(QPaintEvent *event);
	
private:
    Ui::BatteryClass ui;
    QVector<Cell*> m_vecCells;
    double voltage;
    double current;
    double capacity;
    static const int yOffset;
};

#endif // BATTERY_H
