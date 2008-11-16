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
 *  @file 	adc.c
 * @brief 	Routinen zum Einlesen der AnalogeingÄnge
 * @author 	Benjamin Benz (bbe@heise.de)
 * @date 	26.12.05
 */
#include "adc.h"

#ifdef MCU

#include <avr/io.h>
#include <avr/interrupt.h>
//#include <avr/signal.h>
#include <stdio.h>


#define OFFSET_OVERSAMPLING_LOG		3


typedef struct{
	uint8 channel;
	int16* value;
} adc_channel_t;

static volatile int8 act_channel_idx = -1;
static adc_channel_t channels[8];
static int16 offset = 0;

/*!
 * Initialisert den AD-Umsetzer. 
 * @param channel Für jeden Kanal, den man nutzen möchte, 
 * muss das entsprechende Bit in channel gesetzt sein
 * Bit0 = Kanal 0 usw.
 */
void adc_init(uint8 channel){
	DDRA  &= ~ channel;	// Pin als input
	PORTA &= ~ channel;	// Alle Pullups aus.
}



/*!
 * Misst interne Offsetspannung
 */
void adc_calibrate_offset(void){
	uint16 result = 0;
	uint8_t i;

	DDRA  &= ~ 0x4;	// ADC2 als input
	PORTA &= ~ 0x4;	// ADC2 Pullup aus.

	ADMUX = 0; // externe 2.500V Refernzspannung AVCC, rechts Ausrichtung

	ADMUX |= 0x1A;	// Differential meassurement: ADC2+ ADC2- (connected to AGND)
	
	ADCSRA |= (1 << ADSC);		// Beginne mit der Konvertierung
			
	//Warten bis konvertierung beendet, das sollte 25 ADC-Zyklen dauern!
	while ( (ADCSRA & (1<<ADSC)) != 0) { 
		asm volatile("nop");
	} 
	ADCH;	// Spühle erste Messung

	// Echte 8-fache Messung
	for (i=0; i<(1<<OFFSET_OVERSAMPLING_LOG); i++)	{
		ADCSRA |= (1 << ADSC); // Starte erneut
		while ( (ADCSRA & (1<<ADSC)) != 0) {
			asm volatile("nop");
		} //Warten bis konvertierung beendet

		result += ADCL;
		result += (ADCH<<8);	// Ergebnis zusammenbauen
	}

	result >>= OFFSET_OVERSAMPLING_LOG;

	if (result & 0x200)	{
		offset = (int16_t)result - 1024;
	} else {
		offset = result;
	}
	ADCSRA &= ~ADIF;
}

/*!
 * Liefert gemessene interne Offsetspannung
 */
int16_t adc_get_offset(void)
{
	return offset;
}

/*!
 * Liesst pollend einen channel aus
 * @param channel 	Kanal - hex-Wertigkeit des Pins (0x01 fuer PA0; 0x02 fuer PA1, ..)
 */
uint16_t adc_read_polled(uint8 channel) {
	uint16 result = 0;
	uint8_t i;

	ADMUX = channel & 0x7; // externe 2.500V Refernzspannung AVCC, rechts Ausrichtung
	
	ADCSRA |= (1 << ADSC);		// Beginne mit der Konvertierung
			
	//Warten bis konvertierung beendet, das sollte 25 ADC-Zyklen dauern!
	while ( (ADCSRA & (1<<ADSC)) != 0) { 
		asm volatile("nop");
	} 
	ADCH;	// Spühle erste Messung

	// Echte 8-fache Messung
	for (i=0; i<(1<<OFFSET_OVERSAMPLING_LOG); i++)	{
		ADCSRA |= (1 << ADSC); // Starte erneut diesmal nur 13 Zyklen -> 104us
		while ( (ADCSRA & (1<<ADSC)) != 0) {
			asm volatile("nop");
		} //Warten bis konvertierung beendet

		result += ADCL;
		result += (ADCH<<8);	// Ergebnis zusammenbauen
	}

	result >>= OFFSET_OVERSAMPLING_LOG;

	ADCSRA &= ~ADIF;
	
	return result;
}

/*!
 * @brief			Fuegt einen analogen Kanal in die ADC-Konvertierungsliste ein und wertet ihn per Interrupt aus
 * @param channel 	Kanal - hex-Wertigkeit des Pins (0x01 fuer PA0; 0x02 fuer PA1, ..)
 * @param p_sens	Zeiger auf den Sensorwert, der das Ergebnis enthalten soll
 */
void adc_read_int(uint8 channel, int16* p_sens)
{
	static uint8 next_channel_idx = 0;

	if (act_channel_idx == -1)
		next_channel_idx = 0;

	if (next_channel_idx >= 8) 
		return;	// es gibt nur 8 ADC-Channels

	channels[next_channel_idx].value = p_sens;
	channels[next_channel_idx++].channel = channel & 0x7;
	if (act_channel_idx == -1)
	{
		act_channel_idx = 0;
		
		ADMUX  = 0;// externe 2.5V Referenz, rechts Ausrichtung		
		ADMUX |= (channel & 0x07);	// Und jetzt Kanal waehlen, nur single ended

		ADCSRA= (1 <<ADPS2) | 
				(1 <<ADPS1) |		// prescale faktor= 128 ADC laeuft
				(1 <<ADPS0) |		// mit 16MHz/ 128 = 125kHz 
				(1 <<ADEN)  |		// ADC an
				(1 <<ADSC)  |		// Beginne mit der Konvertierung
				(1 <<ADIE);			// Interrupt an
	}
}

/*!
 * Interrupt-Handler fuer den ADC. Speichert das Ergebnis des aktuellen Channels und 
 * schaltet in der Liste der auszuwertenden Sensoren eins weiter.
 */
SIGNAL (SIG_ADC)
{
	/* Daten speichern und Pointer im Puffer loeschen */
	*channels[act_channel_idx].value = ADCL | (ADCH << 8);
	channels[act_channel_idx].value = NULL;
	/* zum naechsten Sensor weiterschalten */
	act_channel_idx++;	
	if (act_channel_idx < 8 && channels[act_channel_idx].value != NULL)
	{
		ADMUX  = 0;// externe 2.5V Referenz, rechts Ausrichtung		
		ADMUX |= channels[act_channel_idx].channel;

		ADCSRA= (1 <<ADPS2) | 
				(1 <<ADPS1) |		// prescale faktor= 128 ADC laeuft
				(1 <<ADPS0) |		// mit 16MHz/ 128 = 125kHz 
				(1 <<ADEN)  |		// ADC an
				(1 <<ADSC)	|		// Beginne mit der Konvertierung
				(1 <<ADIE);		// Interrupt an
	} else{
		ADCSRA = (1 << ADEN);   // ADC eingeschaltet lassen
		act_channel_idx = -1;
	}
}

/*!
 * Gibt die laufende Nr. des Channels zurueck, der aktuell ausgewertet wird.
 * 0: erste registrierter Channel, 1: zweiter registrierter Channel usw.
 * 255: derzeit wird kein Channel ausgewertet (= Konvertierung fertig)
 */
uint8 adc_get_active_channel(void){
	return (uint8)act_channel_idx;	
}

#endif
