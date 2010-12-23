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

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>

#include "io.h"
#include "delay.h"

#define OFFSET_OVERSAMPLING_LOG   5
#define ADC_NBR_OF_CHANNELS	  3


typedef struct
{
	uint8 channel;
	int16* value;
} adc_channel_t;

static volatile int8 act_channel_idx = -1;
static adc_channel_t channels[ADC_NBR_OF_CHANNELS];
int8_t adc_sampling = 0;

/*!
 * Selektiert den Kanal
 */
static inline void adc_select_channel(uint8 channel);

/*!
 * Initialisert den AD-Umsetzer. 
 */
void adc_init(uint8_t channel)
{
	DDRA  &= (uint8_t)~(1<<channel);
	PORTA &= (uint8_t)~(1<<channel);

	ADCSRA= (1 <<ADPS2) | 
		(1 <<ADPS1) |		// prescale faktor= 128 ADC laeuft
		(1 <<ADPS0) |		// mit 16MHz/ 128 = 125kHz 
		(1 <<ADEN);		// ADC an
	
	delay(5);
	
	adc_sampling = 1;
}


/*!
 * Liesst pollend einen channel aus
 * @param channel 	Kanal - hex-Wertigkeit des Pins (0x01 fuer PA0; 0x02 fuer PA1, ..)
 */
uint16_t adc_read_polled(uint8 channel) {
	uint16 result = 0;
	uint8_t i;

	adc_select_channel( channel ); // externe 2.500V Refernzspannung AVCC, rechts Ausrichtung

	ADCSRA= (1 <<ADPS2) | 
		(1 <<ADPS1) |		// prescale faktor= 128 ADC laeuft
		(1 <<ADPS0) |		// mit 16MHz/ 128 = 125kHz 
		(1 <<ADEN);		// ADC an


	ADCSRA |= (1 << ADSC);		// Beginne mit der Konvertierung

			
	//Warten bis konvertierung beendet, nach dem ersten Einschalten dauert das 25 ADC-Zyklen...
	while ((ADCSRA&(1<<ADSC))!=0);


	ADCH;	// Spühle erste Messung

	// Echte 8-fache Messung
	for (i=0; i<(1<<OFFSET_OVERSAMPLING_LOG); i++)	
	{
		
		ADCSRA |= (1 << ADSC); // Starte erneut diesmal nur 13 Zyklen -> 104us

		while ((ADCSRA&(1<<ADSC))!=0);

		result += ADCL;
		result += (ADCH<<8);	// Ergebnis zusammenbauen
	}

	result >>= OFFSET_OVERSAMPLING_LOG;

	ADCSRA &= ~ADIF;
	
	return result;
}

inline void adc_select_channel(uint8 channel)
{
	channel &= 0xF;		// rechts Ausrichtung und Kanal waehlen, 
				// nur single ended
				//
	channel |= ((1<<REFS1) |
		    (1<<REFS0));	// internal 2.56V reference

	ADMUX = channel;
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

	if (next_channel_idx >= ADC_NBR_OF_CHANNELS) 
		return;	// es gibt nur 3 ADC-Channels

	channels[next_channel_idx].value = p_sens;
	channels[next_channel_idx++].channel = channel & 0x1F;
	if (act_channel_idx == -1)
	{
		act_channel_idx = 0;
		
		adc_select_channel(channel);

		ADCSRA = (1 <<ADPS2) | 
			 (1 <<ADPS1) |		// prescale faktor= 128 ADC laeuft
			 (1 <<ADPS0) |		// mit 16MHz/ 128 = 125kHz 
			 (1 <<ADEN)  |		// ADC an
			 (1 <<ADSC)  |		// Beginne mit der Konvertierung
			 (1 <<ADIE);		// Interrupt an
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
	if (act_channel_idx < ADC_NBR_OF_CHANNELS && channels[act_channel_idx].value != NULL)
	{
		adc_select_channel( channels[act_channel_idx].channel );

		ADCSRA= (1 <<ADPS2) | 
			(1 <<ADPS1) |		// prescale faktor= 128 ADC laeuft
			(1 <<ADPS0) |		// mit 16MHz/ 128 = 125kHz 
			(1 <<ADEN)  |		// ADC an
			(1 <<ADSC)  |		// Beginne mit der Konvertierung
			(1 <<ADIE);		// Interrupt an
	}
	else
	{
//		ADCSRA = (1 << ADEN);   // ADC eingeschaltet lassen
		act_channel_idx = -1;
	}
}

/*!
 * Gibt die laufende Nr. des Channels zurueck, der aktuell ausgewertet wird.
 * 0: erste registrierter Channel, 1: zweiter registrierter Channel usw.
 * 255: derzeit wird kein Channel ausgewertet (= Konvertierung fertig)
 */
uint8 adc_get_active_channel(void)
{
	return (uint8)act_channel_idx;	
}
