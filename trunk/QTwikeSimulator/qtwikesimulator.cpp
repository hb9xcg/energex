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

#include "qtwikesimulator.h"
#include <QTimer>
#include <QCloseEvent>

#include "load.h"
#include "powersupply.h"
#include "battery.h"
#include "balancer.h"


QTwikeSimulator::QTwikeSimulator(QWidget *parent)
    : QWidget(parent)
{
	ui.setupUi(this);
	
	load = new Load();
	load->show();
	
	battery = new Battery();
	battery->show();
	
	powerSupply = new PowerSupply();
	powerSupply->show();
	
	balancer = new Balancer(*battery, *powerSupply);
	balancer->show();
	
	timer = new QTimer(this);
	timer->setInterval(10);
	connect(timer, SIGNAL(timeout()), this, SLOT(timeout()));
	
	ui.button_stop->setEnabled(false);
	
	ui.lcdVoltage->setSegmentStyle(QLCDNumber::Flat);
	ui.lcdVoltage->setNumDigits(5);
	
	ui.lcdCurrent->setSegmentStyle(QLCDNumber::Flat);
	ui.lcdCurrent->setNumDigits(5);
	
	ui.lcdCharge->setSegmentStyle(QLCDNumber::Flat);
	ui.lcdCharge->setNumDigits(5);
	
	ui.lcdTemperature->setSegmentStyle(QLCDNumber::Flat);
	//ui.lcdTemperature->setNumDigits(3);
	
	ui.lcdSym->setSegmentStyle(QLCDNumber::Flat);
	
}

QTwikeSimulator::~QTwikeSimulator()
{
	delete timer;
	delete balancer;
	delete load;
	delete powerSupply;
	delete battery;
}

void QTwikeSimulator::on_button_start_clicked(void)
{
	if (ui.radio_charge->isChecked())
	{
		powerSupply->on();
	}
	timer->start();
	ui.button_start->setEnabled(false);
	ui.button_stop->setEnabled(true);
}

void QTwikeSimulator::on_button_stop_clicked(void)
{
	timer->stop();
	ui.button_stop->setEnabled(false);
	ui.button_start->setEnabled(true);
}

void QTwikeSimulator::on_radio_drive_toggled(bool state)
{
	if (state)
	{
		powerSupply->off();
		load->on();
		load->show();
	}
}
void QTwikeSimulator::on_radio_off_toggled(bool state)
{
	if (state)
	{
		powerSupply->off();
		load->off();
	}
}
void QTwikeSimulator::on_radio_charge_toggled(bool state)
{
	if (state)
	{
		load->off();
		powerSupply->on();
		powerSupply->show();
	}
}


void QTwikeSimulator::timeout(void)
{
	battery->sample( timer->interval() );
	balancer->sample( timer->interval() );
	
	double voltage = battery->getVoltage();
	
	double sourceCurrent = powerSupply->getCurrent(voltage);
	double drainCurrent = load->getCurrent(voltage);
	
	double totalCurrent = sourceCurrent-drainCurrent; 
	battery->setCurrent(totalCurrent);
	
	
	ui.lcdVoltage->display(voltage);
	ui.lcdCurrent->display(totalCurrent);
	ui.lcdCharge->display( battery->getCapacity());
	ui.lcdSym->display(battery->getSymVoltage());
}

void QTwikeSimulator::closeEvent(QCloseEvent *event)
{
	event->accept();
	delete balancer;
	delete load;
	delete powerSupply;
	delete battery;
}
