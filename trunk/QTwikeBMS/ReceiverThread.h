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
#ifndef RECEIVER_H_
#define RECEIVER_H_

#include <QThread>


class ReceiverThread : public QThread
{
	Q_OBJECT

	bool running;
	
	void receivePacket(void);
	void receiveData(void);
	void transmitData(void);
	void transmitGroup(void);
	
	void logTransmit(uint8_t packet[], uint8_t legth);
	void logReceive();
	
public:
	ReceiverThread();
	virtual ~ReceiverThread();	
	
protected:
	virtual void run();
	void os_thread_sleep(uint32_t sleep);
	
public slots:
	void stop();
	
signals:
	void updateLog(const QString& log);
};

#endif /*RECEIVER_H_*/
