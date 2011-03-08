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
 * @file 	adc.h
 * @brief 	Routinen zum Einlesen der Analogeingaenge
 * @author 	Benjamin Benz (bbe@heise.de)
 * @date 	26.12.05
 */
#ifndef ADC_H_
#define ADC_H_

#include "global.h"


#define CH_CHARGE_1       0x3
#define CH_DISCHARGE_1    0x1
#define CH_CHARGE_10      0xD
#define CH_DISCHARGE_10   0x9
#define CH_VOLTAGE        0x7
#define CH_TEMPERATURE    0x4

extern int8_t adc_sampling;
/*!
 * Misst interne Offsetspannung
 */
void adc_calibrate_offset(int8_t channel);

/*!
 * Liefert gemessene interne Offsetspannung
 */
int8_t adc_get_offset(int8_t channel);

/*!
 * Liefert x mV/LSB
 */
int8_t adc_get_resolution(int8_t channel);

/*!
 * Liesst pollend einen channel aus
 * @param channel 	Kanal - hex-Wertigkeit des Pins (0x01 fuer PA0; 0x02 fuer PA1, ..)
 */
uint16_t adc_read_polled(uint8 channel);

/*!
 * @brief			Fuegt einen analogen Kanal in die ADC-Konvertierungsliste ein und wertet ihn per Interrupt aus
 * @param channel 	Kanal - hex-Wertigkeit des Pins (0x01 fuer PA0; 0x02 fuer PA1, ..)
 * @param p_sens	Zeiger auf den Sensorwert, der das Ergebnis enthalten soll
 */
void adc_read_int(uint8_t channel, int16_t* p_sens);

/*!
 * Gibt die laufende Nr. des Channels zurueck, der aktuell ausgewertet wird.
 * 0: erste registrierter Channel, 1: zweiter registrierter Channel usw.
 * 255: derzeit wird kein Channel ausgewertet (= Konvertierung fertig)
 */
uint8 adc_get_active_channel(void);

/*!
 * Initialisert den AD-Umsetzer. 
 * @param channel fuer jeden Kanal, den man nutzen moechte, 
 * muss das entsprechende Bit in channel gesetzt sein.
 * Bit0 = Kanal 0 usw.
 */
void adc_init(uint8_t channel);


#endif
