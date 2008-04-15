/*
 * Energex
 * 
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software
 * Foundation; either version 2 of the License, or (at your
 * option) any later version. 
 * This program is distributed in the hope that it will be 
 * useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
 * PURPOSE. See the GNU General Public License for more details.
 * You should have received a copy of the GNU General Public 
 * License along with this program; if not, write to the Free 
 * Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307, USA.
 * 
 */
 
 /*! 
 * @file 	adc.c
 * @brief 	Implements 10bit blocking analog digital converter.
 * @author 	Markus Walser (markus.walser@gmail.com)
 * @date 	04.03.2008
  */
#include <avr/io.h>

#include "adc.h"

#define ClearBit(register, bit)       register &= ~(1<<bit)
#define SetBit(register, bit)         register |=  (1<<bit)
#define IsSet(register, bit)          register &   (1<<bit)
#define IsCleared(register, bit)    !(register &   (1<<bit))


uint16_t readADC(uint8_t Channel)
{
	uint16_t result=0;
	uint16_t low, high;
	uint8_t i;

	ADMUX = Channel;                      //Set AD-Channel

	SetBit( ADMUX, REFS0);                // Use internal reference (also 2,56 V)
	SetBit( ADMUX, REFS1);

	SetBit(ADMUX,ADLAR);                  // Adjust ad-result to the left

	SetBit(ADCSR,ADEN);                   // Enable ADC

	SetBit(ADCSR, ADPS0);                 // Set clock prescaler 128
	SetBit(ADCSR, ADPS1);
	SetBit(ADCSR, ADPS2);

	ClearBit(ADCSR, ADFR);                // Disable freerunning
	SetBit(ADCSR, ADIF);

	SetBit(ADCSR, ADSC);                  // Start conversion
	while(!(ADCSR&(1<<ADIF)));            // Wait till end of conversion
	ADCH;                                 // Flush first result
	SetBit(ADCSR, ADIF);                  // Clear interrupt flag

	for (i=0; i<8; i++)                   // Take 8 Samples
	{
		SetBit(ADCSR, ADSC);              // Start conversion
		while(!(ADCSR&(1<<ADIF)));        // Wait until end of conversion
		low = ADCL;                       // Read low bits
		high = ADCH;                      // Read high bits
		
		low >>= 6;
		high <<= 2;
		
		result+= low;                     // Combine lower and higher bits and
		result+= high;                    // integrate result
		SetBit(ADCSR, ADIF);              // clear interrupt flag
	}

	ClearBit(ADCSR,ADEN);                //Disable ADC peripheral
	return result;
}
