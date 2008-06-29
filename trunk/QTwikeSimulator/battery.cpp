/****************************************************************************
**
** Trolltech hereby grants a license to use the Qt/Eclipse Integration
** plug-in (the software contained herein), in binary form, solely for the
** purpose of creating code to be used with Trolltech's Qt software.
**
** Qt Designer is licensed under the terms of the GNU General Public
** License versions 2.0 and 3.0 ("GPL License"). Trolltech offers users the
** right to use certain no GPL licensed software under the terms of its GPL
** Exception version 1.2 (http://trolltech.com/products/qt/gplexception).
**
** THIS SOFTWARE IS PROVIDED BY TROLLTECH AND ITS CONTRIBUTORS (IF ANY) "AS
** IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
** TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
** PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER
** OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
** EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
** PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
** PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
** LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
** NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
** SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** Since we now have the GPL exception I think that the "special exception
** is no longer needed. The license text proposed above (other than the
** special exception portion of it) is the BSD license and we have added
** the BSD license as a permissible license under the exception.
**
****************************************************************************/

#include <QtGui>
#include "battery.h"

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
}

Battery::~Battery()
{
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
	return m_vecCells[address]->getVoltage();
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
