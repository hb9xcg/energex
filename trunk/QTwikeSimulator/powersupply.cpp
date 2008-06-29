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

#include "powersupply.h"

const double PowerSupply::prechargeCurrent = 0.7;
const double PowerSupply::fastCurrent      = 5.0;
const double PowerSupply::normalCurrent    = 2.0;
const double PowerSupply::symCurrent       = 0.6;

PowerSupply::PowerSupply(QWidget *parent)
    : QWidget(parent)
{
	ui.setupUi(this);
	ticks = 0;
	eState = eInvalid;
	
	ui.lcdVoltage->setSegmentStyle(QLCDNumber::Flat);
	ui.lcdCurrent->setSegmentStyle(QLCDNumber::Flat);
	ui.lcdPower->setSegmentStyle(QLCDNumber::Flat);
	
	switchState(eOff);
}

PowerSupply::~PowerSupply()
{

}

void PowerSupply::on(void)
{
	switchState(eLadeBereit);
}

void PowerSupply::off(void)
{
	switchState(eOff);
}

void PowerSupply::battFull(void)
{
	switchState(eSym_Ladung);
	startSym = ticks;
}

void PowerSupply::voltageToHigh(void)
{
	current -= 0.1;
}

void PowerSupply::switchState(EState eState)
{
	if (this->eState == eState)
	{
		return;
	}
	
	this->eState = eState;
	switch(eState)
	{
	case eOff:
		ui.label_state->setText("Off");
		break;
	case eLadeBereit:
		ui.label_state->setText("Ladebereit");
		break;
	case eVorladung:
		ui.label_state->setText("Vorladen");
		break;
	case eI_SchnellLadung:
		ui.label_state->setText("I-Ladung Schnell");
		break;
	case eI_NormalLadung:
		ui.label_state->setText("I-Ladung Normal");
		break;
	case eU_Ladung:			
		ui.label_state->setText("U-Ladung");
		break;
	case eSym_Ladung:
		ui.label_state->setText("Sym-Ladung");
		break;
	case eErh_Ladung:
		ui.label_state->setText("Erhaltungs-Ladung");
		break;
	case eInvalid:
		ui.label_state->setText("Invalid");
		break;
	}
}

double PowerSupply::getCurrent(double voltage)
{
	switch(eState)
	{
	case eOff:
		current = 0.0;
		break;
	case eLadeBereit:
		current = 0.0;
		if (voltage<336.0)
		{
			switchState(eVorladung);
		}
		else if (voltage < 410.0)
		{
			switchState(eI_SchnellLadung);
		}
		else if (voltage < 435.0)
		{
			switchState(eI_NormalLadung);
		}
		else
		{
			switchState(eU_Ladung);
		}
		break;
	case eVorladung:
		current = prechargeCurrent;
		if (voltage>336.0)
		{
			switchState(eI_SchnellLadung);
		}
		break;
	case eI_SchnellLadung:
		current = fastCurrent;
		if (voltage>410.0)
		{
			switchState(eI_NormalLadung);
		}
		break;
	case eI_NormalLadung:
		current = normalCurrent;
		if (voltage>435.0)
		{
			switchState(eU_Ladung);
		}
		break;
	case eU_Ladung:
		if (voltage>435.0)
		{
			current -= 0.1;
		}
		else
		{
			current += 0.1;
		}
		break;
	case eSym_Ladung:
		current = symCurrent;
		if (ticks-startSym > 30*60*1000)
		{
			switchState(eErh_Ladung);
		}
		break;
	case eErh_Ladung:
		break;
	case eInvalid:
		break;
	}
	
	ui.lcdCurrent->display(current);
	ui.lcdVoltage->display(voltage);
	ui.lcdPower->display(current*voltage);
	
	return current;
}

void PowerSupply::sample(int ms)
{
	ticks += ms;
}

