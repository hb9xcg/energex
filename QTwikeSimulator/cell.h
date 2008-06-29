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

#ifndef CELL_H
#define CELL_H

#include <QtGui/QWidget>
#include "ui_cell.h"

class Cell : public QWidget
{
    Q_OBJECT

public:
    Cell(int xPosition, int yPosition, int address, QWidget *parent = 0);
    ~Cell();
    
    void sample(int ms);
    void updateCharge(double Ah);
    double getVoltage(void) { return voltage; };
    double getCurrent(void) { return current; };
    void setCurrent(double current) { this->current = current; };
    void switchBalancer(int percentage);
    
    static int get42YPos(void);
    static int get27YPos(void);
    static int get0YPos(void);
    

private:
    Ui::CellClass ui;
    QBrush brushFill;
    
    double voltage;
    double current;
    double actualCapacity;
    double nominalCapacity;
    double Ri;
    double iBalancer;
    
    static const double nominalVoltage;
    static const double maximalVoltage;
    static const double nominalRi;
    static const double rBalancer;    // [Ohm]
    static const double lowVoltage;
    static const double RiDeviation; // [%]
    static const double capacityDeviation; // [%]
    static const int    maxHeight;
    
    int m_xPosition;
    int m_yPosition;
    int m_address;
    int toolTipRefresh;
    bool balancerOn;
    int balancerPercentage;
    int balanceCounter;
    
    void drawVoltage(void);
    
protected:
    void paintEvent(QPaintEvent *event);
};

#endif // CELL_H
