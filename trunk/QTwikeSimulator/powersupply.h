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

#ifndef POWERSUPPLY_H
#define POWERSUPPLY_H

#include <QtGui/QWidget>
#include "ui_powersupply.h"

class PowerSupply : public QWidget
{
    Q_OBJECT

public:
    PowerSupply(QWidget *parent = 0);
    ~PowerSupply();
    
    double getCurrent(double voltage);
    void on(void);
    void off(void);
    void battFull(void);
    void voltageToHigh(void);
    void sample(int ms);

private:
    Ui::PowerSupplyClass ui;
    double current;
    static double prechargeCurrent;
    static double fastCurrent;
    static double normalCurrent;
    static double symCurrent;
    long long symTime;
    
    enum EState
    {
    	eOff,
    	eLadeBereit,
    	eVorladung,
    	eU_Ladung,
    	eI_SchnellLadung,
    	eI_NormalLadung,
    	eSym_Ladung,
    	eErh_Ladung,
    	eInvalid
    } eState;
    
    void switchState(EState eState);
    
private slots:
	void on_spinboxPreCharge_valueChanged(void);
	void on_spinboxNormalCharge_valueChanged(void);
	void on_spinboxQuickCharge_valueChanged(void);
	void on_spinboxSymCharge_valueChanged(void);
	
};

#endif // POWERSUPPLY_H
