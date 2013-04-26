/*
 * Energex
 * 
 * Copyright (C) 2005-2007 by Benjamin Benz
 * bbe@heise.de
 *
 * Copyright (C) 2008-2010 by Markus Walser
 * markus.walser@gmail.com
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

#define OFFSET_OVERSAMPLING_LOG   6
#define ADC_NBR_OF_CHANNELS	  3

#define CH_DISCHARGE_CALIB   0x8
#define CH_CHARGE_CALIB      0xC


#define IS_DIFFERENTIAL(x)   ((x) > 7)

typedef struct
{
	uint8 channel;
	int16* value;
} adc_channel_t;

static volatile int8 act_channel_idx = -1;
static adc_channel_t channels[ADC_NBR_OF_CHANNELS];
static int16_t adc_offset_ch27 = 0; // Current channel 1x gain
static int16_t adc_offset_ch13 = 0; // Current channel 10x gain
static int16_t adc_offset_ch15 = 0; // Current channel 200x gain
static int16_t adc_offset_ch16 = 0; // Voltage channel 1x gain

/*!
 * Selektiert den Kanal
 */
static inline void adc_select_channel(uint8 channel);
static void adc_convert_differential(int16_t* value);

/*!
 * Initialisert den AD-Umsetzer. 
 */
void adc_init()
{
	DDRA  &= ~ 0xf;
	PORTA &= ~ 0xf;

//	DIDR0 |= 0xf;

	ADCSRA= (1 <<ADPS2) | 
		(1 <<ADPS1) |		// prescale faktor= 128 ADC laeuft
		(1 <<ADPS0) |		// mit 16MHz/ 128 = 125kHz 
		(1 <<ADEN);		// ADC an
}



/*!
 * Misst interne Offsetspannung
 */
void adc_calibrate_offset(int8_t channel)
{
	int16_t result = 0;
	int8_t i;

	adc_select_channel(channel);

	
	ADCSRA |= (1 << ADSC);		// Start first dummy measurement
			
	while ((ADCSRA&(1<<ADSC))!=0);	// Wait for dummy measurement
 
	ADCH;	// Flush dummy measurement, now internal offset calibration should be finished

	// Real measurement 64 times oversampled
	for (i=0; i<(1<<OFFSET_OVERSAMPLING_LOG); i++)
	{
		int16_t adc;	

		ADCSRA |= (1 << ADSC); // Restart ADC

		while ((ADCSRA&(1<<ADSC))!=0); // Wait for result
		
		adc  = ADCL;
		adc += (ADCH<<8);
		
		adc_convert_differential(&adc);
		result += adc;
	}

	result >>= OFFSET_OVERSAMPLING_LOG;

	switch(channel)
	{
	case CH_CURRENT_1:
	case CH_CURRENT_1_CALIB:
		adc_offset_ch27 = result;
		break;

	case CH_CURRENT_10:
	case CH_CURRENT_10_CALIB:
		adc_offset_ch13 = result;
		break;

	case CH_CURRENT_200:
	case CH_CURRENT_200_CALIB:
		adc_offset_ch15 = result;
		break;

	case CH_VOLTAGE:
		adc_offset_ch16 = result;
		break;
	case CH_VOLTAGE_CALIB:
		adc_offset_ch16 = result;
		break;

	}
	
	ADCSRA &= ~ADIF;
}

void adc_convert_differential(int16_t* value)
{
	if (*value & 0x200)
	{
		int16_t result = (int16_t)*value - 1024;
		*value = result;
	}
}

/*!
 * Liefert gemessene interne Offsetspannung
 */
int8_t adc_get_offset(int8_t channel)
{
	switch (channel)
	{
	case CH_CURRENT_1:	return adc_offset_ch27;
	case CH_CURRENT_10:	return adc_offset_ch13;
	case CH_CURRENT_200:	return adc_offset_ch15;
	case CH_VOLTAGE:	return adc_offset_ch16;
	default:		return 0;
	}
}

/*!
 * Liefert x mV/LSB
 */
int8_t adc_get_resolution(int8_t channel)
{
	switch(channel)
	{
	case CH_CURRENT_1:	return 200;
	case CH_CURRENT_10:	return 10;
	case CH_CURRENT_200:	return 1;

	case CH_DISCHARGE_10:	return -1;  // 2.5V / 512  / 10
	case CH_CHARGE_1:	return 5;   // 2.5V / 1024
	case CH_DISCHARGE_1:	return -5;  // 2.5V / 1024
	default:		return 5;
	}
}

/*!
 * Liesst pollend einen channel aus
 * @param channel 	Kanal - hex-Wertigkeit des Pins (0x01 fuer PA0; 0x02 fuer PA1, ..)
 */
int16_t adc_read_polled(uint8 channel) {
	int16_t result = 0;
	uint8_t i;

	adc_select_channel( channel ); // externe 2.500V Refernzspannung AVCC, rechts Ausrichtung
//	ADMUX = 0x17; // 10111 CH7+ CH1-
//	ADMUX = 0x1A; // 11010 CH2+ CH2-
//	ADMUX = 0x1E; // 11110 1.1V Bandgap
//	ADMUX = 0x09; // 01001 CH1+ CH0- 10x

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
		int16_t adc;	
		ADCSRA |= (1 << ADSC); // Starte erneut diesmal nur 13 Zyklen -> 104us

		while ((ADCSRA&(1<<ADSC))!=0);

		adc  = ADCL;
		adc += (ADCH<<8);	// Ergebnis zusammenbauen

		adc_convert_differential(&adc);
		result += adc;
	}

	result >>= OFFSET_OVERSAMPLING_LOG;

	ADCSRA &= ~ADIF;
	
	return result;
}

inline void adc_select_channel(uint8 channel)
{
	ADMUX = (channel & 0x1F);	// externe 2.5V Referenz, rechts Ausrichtung
					// und Kanal waehlen, nur single ended
}

/*!
 * @brief	Fuegt einen analogen Kanal in die ADC-Konvertierungsliste ein und wertet ihn per Interrupt aus
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
	channels[next_channel_idx].channel = channel & 0x1F;
	next_channel_idx++;
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
SIGNAL (ADC_vect)
{
	/* Daten speichern und Pointer im Puffer loeschen */
	*channels[act_channel_idx].value = ADCL | (ADCH << 8);
	if (IS_DIFFERENTIAL(channels[act_channel_idx].channel))
	{
		adc_convert_differential(channels[act_channel_idx].value);
	}
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
