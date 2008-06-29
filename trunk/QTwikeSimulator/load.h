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

#ifndef LOAD_H
#define LOAD_H

#include <QtGui/QWidget>
#include "ui_load.h"

class Load : public QWidget
{
    Q_OBJECT

public:
    Load(QWidget *parent = 0);
    ~Load();
    
    double getCurrent(double voltage);
    void on(void);
    void off(void);
    void battEmpty(void);
    void sample(int ms);

private:
    Ui::LoadClass ui;
    
    enum EState
    {
    	eOff,
    	eOn,
    	eEmpty,
    	eInvalid
    } eState;
    
    void switchState(EState eState);
};

#endif // LOAD_H
