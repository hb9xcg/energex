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
