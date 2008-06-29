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
