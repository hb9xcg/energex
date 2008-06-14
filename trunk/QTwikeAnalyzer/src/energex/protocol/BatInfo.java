/***************************************************************************
 *   Copyright (C) 2008 by Markus Walser                                   *
 *   markus.walser@gmail.com                                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
package energex.protocol;

public class BatInfo {
	 private static int REKUPERATION_NOK  =  0; // Rekuperation nicht erlaubt
	 private static int CHARGE_NOK        =  1; // Ladung nicht erlaubt
	 private static int DRIVE_NOK         =  2; // Fahren nicht erlaubt
	 private static int CHARGE_CUR_TO_HI  =  3; // Zu hoher Ladestrom
	 private static int DRIVE_CUR_TO_HI   =  4; // Zu hoher Fahrstrom
	 private static int VOLTAGE_TO_HI     =  5; // Zu hohe Ladespannung
	 private static int VOLTAGE_TO_LO     =  6; // Zu tiefe Fahrspannung
	 private static int BAT_REL_OPEN      =  7; // Batterie Relais offen
	 private static int BAT_FULL          =  8; // Batterie voll
	 private static int BAT_EMPTY         =  9; // Batterie entladen
	 private static int CHARGE_TEMP_TO_HI = 10; // Zu hohe Ladetemperatur
	 private static int CHARGE_TEMP_TO_LO = 11; // Zu tiefe Ladetemperatur
	 private static int DRIVE_TEMP_TO_HI  = 12; // Zu hohe Fahrtemperatur
	 private static int DRIVE_TEMP_TO_LO  = 13; // Zu tiefe Fahrtemperatur
	 private static int VOLTAGE_NOK       = 14; // Unsymmetrische Spannungen
	 private static int BAT_ERROR         = 15; // Fehler in der Batterieüberwachung
	 
	 static public String decodeBatInfo(int batInfo) {
		 String description = new String();
		 
		 if ((batInfo & 1<<REKUPERATION_NOK) != 0) {
			 description = "Rekuperation nicht erlaubt";
		 }
		 if ((batInfo & 1<<CHARGE_NOK) != 0) {
			 description += ", REKUPERATION_NOK";
		 }
		 if ((batInfo & 1<<DRIVE_NOK) != 0) {
			 description += ", Fahren nicht erlaubt";
		 }
		 if ((batInfo & 1<<CHARGE_CUR_TO_HI) != 0) {
			 description += ", Zu hoher Ladestrom";
		 }
		 if ((batInfo & 1<<DRIVE_CUR_TO_HI) != 0) {
			 description += ", Zu hoher Fahrstrom";
		 }
		 if ((batInfo & 1<<VOLTAGE_TO_HI) != 0) {
			 description += ", Zu hohe Ladespannung";
		 }
		 if ((batInfo & 1<<VOLTAGE_TO_LO) != 0) {
			 description += ", Zu tiefe Fahrspannung";
		 }
		 if ((batInfo & 1<<BAT_REL_OPEN) != 0) {
			 description += ", Batterie Relais offen";
		 }
		 if ((batInfo & 1<<BAT_FULL) != 0) {
			 description += ", Batterie voll";
		 }
		 if ((batInfo & 1<<BAT_EMPTY) != 0) {
			 description += ", Batterie entladen";
		 }
		 if ((batInfo & 1<<CHARGE_TEMP_TO_HI) != 0) {
			 description += ", Zu hohe Ladetemperatur";
		 }
		 if ((batInfo & 1<<CHARGE_TEMP_TO_LO) != 0) {
			 description += ", Zu tiefe Ladetemperatur";
		 }
		 if ((batInfo & 1<<DRIVE_TEMP_TO_HI) != 0) {
			 description += ", Zu hohe Fahrtemperatur";
		 }
		 if ((batInfo & 1<<DRIVE_TEMP_TO_LO) != 0) {
			 description += ", Zu tiefe Fahrtemperatur";
		 }
		 if ((batInfo & 1<<VOLTAGE_NOK) != 0) {
			 description += ", Unsymmetrische Spannungen";
		 }
		 if ((batInfo & 1<<BAT_ERROR) != 0) {
			 description += ", Fehler in der Batterieüberwachung";
		 }
			 
		 return description;		 
	 }
}
