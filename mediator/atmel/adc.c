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

#define CH_DISCHARGE_CALIB   0x8
#define CH_CHARGE_CALIB      0xC


#define IS_DIFFERENTIAL(x)   ((x) & 0x8)

typedef struct
{
	uint8 channel;
	int16* value;
} adc_channel_t;

static volatile int8 act_channel_idx = -1;
static adc_channel_t channels[ADC_NBR_OF_CHANNELS];
static int16_t adc_offset_ch1 = 0; // Charge channel 10x gain
static int16_t adc_offset_ch3 = 0; // Dischange channel 10x gain
int8_t adc_sampling = 0;

/*!
 * Selektiert den Kanal
 */
static inline void adc_select_channel(uint8 channel);

/*!
 * Initialisert den AD-Umsetzer. 
 * @param channel Für jeden Kanal, den man nutzen möchte, 
 * muss das entsprechende Bit in channel gesetzt sein
 * Bit0 = Kanal 0 usw.
 */
void adc_init(uint8 channel)
{
	DDRA  &= ~ (1<<channel);	// Pin als input
	PORTA &= ~ (1<<channel);	// Alle Pullups aus.

	ADCSRA= (1 <<ADPS2) | 
		(1 <<ADPS1) |		// prescale faktor= 128 ADC laeuft
		(1 <<ADPS0) |		// mit 16MHz/ 128 = 125kHz 
		(1 <<ADEN);		// ADC an
	
	delay(5);
	
	adc_calibrate_offset(CH_CHARGE_CALIB);
	adc_calibrate_offset(CH_DISCHARGE_CALIB);

	adc_sampling = 1;
}



/*!
 * Misst interne Offsetspannung
 */
void adc_calibrate_offset(int8_t channel)
{
	uint16 result = 0;
	int8_t i;

	adc_select_channel(channel);

	
	ADCSRA |= (1 << ADSC);		// Start first dummy measurement
			
	while ((ADCSRA&(1<<ADSC))!=0);	// Wait for dummy measurement
 
	ADCH;	// Flush dummy measurement, now internal offset calibration should be finished

	// Real measurement 64 times oversampled
	for (i=0; i<(1<<OFFSET_OVERSAMPLING_LOG); i++)
	{
		ADCSRA |= (1 << ADSC); // Restart ADC

		while ((ADCSRA&(1<<ADSC))!=0); // Wait for result

		result += ADCL;
		result += (ADCH<<8);
	}

	result >>= OFFSET_OVERSAMPLING_LOG;

	if (result & 0x200)
	{
		if (channel==CH_CHARGE_CALIB)
		{
			adc_offset_ch1 = (int16_t)result - 1024;
		}
		if (channel==CH_DISCHARGE_CALIB)
		{
			adc_offset_ch3 = (int16_t)result - 1024;
		}
	}
	else 
	{
		if (channel==CH_CHARGE_CALIB)
		{
			adc_offset_ch1 = result;
		}
		if (channel==CH_DISCHARGE_CALIB)
		{
			adc_offset_ch3 = result;
		}
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
	if (channel==CH_CHARGE_10)
	{
		return adc_offset_ch1;
	}
	else if (channel==CH_DISCHARGE_10)
	{
		return adc_offset_ch3;
	}
	else
	{
		return 0;
	}
}

/*!
 * Liefert x mV/LSB
 */
int8_t adc_get_resolution(int8_t channel)
{
	switch(channel)
	{
	case CH_CHARGE_10:	return 1;   // 2.5V / 512  / 10
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
uint16_t adc_read_polled(uint8 channel) {
	uint16 result = 0;
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
	ADMUX = (channel & 0x0F);	// externe 2.5V Referenz, rechts Ausrichtung
					// und Kanal waehlen, nur single ended
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
	channels[next_channel_idx++].channel = channel & 0xF;
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
