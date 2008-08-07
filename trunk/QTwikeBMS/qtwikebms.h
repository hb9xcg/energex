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
#ifndef QTWIKEBMS_H
#define QTWIKEBMS_H

#include <QtGui/QWidget>
#include "ui_qtwikebms.h"

class ReceiverThread;

class QTwikeBMS : public QWidget
{
    Q_OBJECT

public:
    QTwikeBMS(QWidget *parent = 0);
    ~QTwikeBMS();

private:
    Ui::QTwikeBMSClass ui;
    ReceiverThread* receiver;
    
private slots:
	void on_spinVoltage_valueChanged(double floatValue);
	void on_spinCurrent_valueChanged(double floatValue);
	void on_spinCharge_valueChanged(double floatValue);
};

#endif // QTWIKEBMS_H
