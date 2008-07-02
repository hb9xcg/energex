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

#ifndef BALANCER_H
#define BALANCER_H

#include <QtGui/QWidget>
#include "ui_balancer.h"

class QTimer;
class Battery;
class PowerSupply;

class Balancer : public QWidget
{
    Q_OBJECT

public:
    Balancer(Battery& battery, PowerSupply& powerSupply, QWidget *parent = 0);
    ~Balancer();

private:
    Ui::BalancerClass ui;
    
    QTimer *timer;
    Battery& battery;
    PowerSupply& powerSupply;
    int time;
    bool on;
    static const double upperVoltage;
    static const double underVoltage;
    
private slots:
    void on_radio_off_toggled(bool state);
    void on_radio_on_toggled(bool state);
    void timeout(void);
};

#endif // BALANCER_H
