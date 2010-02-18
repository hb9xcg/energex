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

import java.util.HashMap;
import java.util.Map;

import com.trolltech.qt.core.QByteArray;
import com.trolltech.qt.core.QRegExp;
import com.trolltech.qt.core.Qt;

import energex.protocol.DataType.EUnit;

public class Request {
	static Map<Short,String> typeToDesc = new HashMap<Short, String>();
	
	static final	short MODEL_TYPE         = 0x01;
	static final	short PROGRAM_REV        = 0x02;
	static final	short PAR_TAB_REV        = 0x03;
	static final	short NENNSPNG           = 0x04;
	static final	short NENNSTROM          = 0x05;
	static final	short SERIE_NUMMER       = 0x06;
	static final	short REP_DATUM          = 0x07; 
	static final	short STANDZEIT          = 0x08;
	static final	short FAHR_LADE_ZEIT     = 0x09;
			
	static final	short BUS_ADRESSE        = 0x0b;
	static final	short COMMAND1           = 0x10; // hex?
	static final	short D_STATE            = 0x11;
	static final	short CMD2               = 0x12;
	static final	short PARAM_PROT         = 0x13;
			
	static final	short BINFO              = 0x21;
			
	static final	short LADESTROM          = 0x23;
	static final	short FAHRSTROM          = 0x24;
	static final	short TOTAL_SPANNUNG     = 0x25;
	static final	short SOLL_LADESPG       = 0x26;
	static final	short AH_ZAEHLER         = 0x27;
	static final	short Q                  = 0x28;
	static final	short LEISTUNG           = 0x29;
	static final	short BATTERIE_TEMP      = 0x2a;
	static final	short FINFO              = 0x2b;
	static final   short SYM_SPANNUNG       = 0x31;
			
	static final	short TEILSPANNUNG1      = 0x32;
	static final	short TEILSPANNUNG2      = 0x33;
	static final	short TEILSPANNUNG3      = 0x34;
	static final	short TEILSPANNUNG4      = 0x35;
	static final	short TEILSPANNUNG5      = 0x36;
	static final	short TEILSPANNUNG6      = 0x37;
	static final	short TEILSPANNUNG7      = 0x38;
		
	static final	short TEMPERATUR1        = 0x41;
	static final	short TEMPERATUR2        = 0x42;
	static final	short TEMPERATUR3        = 0x43;
	static final	short TEMPERATUR4        = 0x44;
	static final	short TEMPERATUR5        = 0x45;
	static final	short TEMPERATUR6        = 0x46;
	static final	short TEMPERATUR7        = 0x47;
	static final	short TEMPERATUR8        = 0x51;
	static final	short TEMPERATUR9        = 0x52;
	static final	short TEMPERATUR10       = 0x53;
	static final	short TEMPERATUR11       = 0x54;
	static final	short TEMPERATUR12       = 0x55;
	static final	short TEMPERATUR13       = 0x56;
	static final	short TEMPERATUR14       = 0x57;
			
	static final	short MAX_BAT_TEMP       = 0x61;
	static final	short UMGEBUNGS_TEMP     = 0x62;
	static final	short MAX_LADE_TEMP      = 0x63;
	static final	short MIN_LADE_TEMP      = 0x64;
	static final	short MAX_FAHR_TEMP      = 0x65;
	static final	short MIN_FAHR_TEMP      = 0x66;
	static final	short MAX_LAGER_TEMP     = 0x67;
	static final	short MIN_LAGER_TEMP     = 0x68;

	static final	short MAX_KAPAZITAET     = 0x71;
	static final	short MIN_KAPAZITAET     = 0x72;
	static final	short GELADENE_AH        = 0x73;
	static final	short ENTLADENE_AH       = 0x74;
	static final	short LADEZYKLEN         = 0x75;
	static final	short TIEFENTLADE_ZYKLEN = 0x76;
	static final	short MAX_ENTLADE_STROM  = 0x77;
			
	static final	short ZYKLUS_UEBER_110   = 0x81; 	
	static final	short ZYKLUS_UEBER_100   = 0x82;
	static final	short ZYKLUS_UEBER_90    = 0x83;
	static final	short ZYKLUS_UEBER_80    = 0x84;
	static final	short ZYKLUS_UEBER_70    = 0x85;
	static final	short ZYKLUS_UEBER_60    = 0x86;
	static final	short ZYKLUS_UEBER_50    = 0x87;
	static final	short ZYKLUS_UEBER_40    = 0x88;
	static final	short ZYKLUS_UEBER_30    = 0x89;
	static final	short ZYKLUS_UEBER_20    = 0x8a;
	static final	short ZYKLUS_UEBER_10    = 0x8b;
	static final	short ZYKLUS_UNTER_10    = 0x8c;
	
	static final	short SYMMETRIER_STROM   = 0x99;

	static final	short LADE_STR_UNTER_M10 = 0xa1;
	static final	short LADE_STR_UEBER_M10 = 0xa2;
	static final	short LADE_STR_UEBER_P00 = 0xa3;
	static final	short LADE_STR_UEBER_P10 = 0xa4;
	static final	short LADE_STR_UEBER_P20 = 0xa5;
	static final	short LADE_STR_UEBER_P30 = 0xa6;
	static final	short LADE_STR_UEBER_P40 = 0xa7;
	static final	short LADE_STR_UEBER_P45 = 0xa8;
	static final	short LADE_STR_UEBER_P50 = 0xa9;
		
	static final	short LADE_SPG_UNTER_M10 = 0xb1;
	static final	short LADE_SPG_UEBER_M10 = 0xb2;
	static final	short LADE_SPG_UEBER_P00 = 0xb3;
	static final	short LADE_SPG_UEBER_P10 = 0xb4;
	static final	short LADE_SPG_UEBER_P20 = 0xb5;
	static final	short LADE_SPG_UEBER_P30 = 0xb6;
	static final	short LADE_SPG_UEBER_P40 = 0xb7;
	static final	short LADE_SPG_UEBER_P45 = 0xb8;
	static final	short LADE_SPG_UEBER_P50 = 0xb9;
		
	static final	short NOM_KAPAZITAET     = 0xc1;
	static final	short MIN_FAHR_SPANNUNG  = 0xc2; 
	static final	short SELBST_ENTL_STROM  = 0xc3;
	static final	short TIEFENTLADE_SPG    = 0xc4;
	static final	short MAX_SPANNUNG_DIFF  = 0xc5;
	static final	short MIN_FAHR_TEMP_B    = 0xc6;
	static final	short MAX_FAHR_TEMP_B    = 0xc7;
	static final	short MAX_FAHR_STROM     = 0xc8;
			
	static final	short KAL_TEMP_7_6       = 0xd1;
	static final	short KAL_TEMP_5_4       = 0xd2;
	static final	short KAL_TEMP_3_2       = 0xd3;
	static final	short KAL_TEMP_1_AMB     = 0xd4;
	static final	short KAL_TEMP_GD_14     = 0xd5;
	static final	short KAL_TEMP_13_12     = 0xd6;
	static final	short KAL_TEMP_11_10     = 0xd7;
	static final	short KAL_TEMP_9_8       = 0xd8;
	
	static final	short SENSOR_MASK        = 0xd9;
		
	static final	short OFFS_KLEIN_STL     = 0xe1;
	static final	short OFFS_GROSS_STL     = 0xe2;
	static final	short KALIB_SPG_1        = 0xe3;
	static final	short KALIB_SPG_2        = 0xe4;
	static final	short KALIB_SPG_3        = 0xe5;
	static final	short KALIB_SPG_4        = 0xe6;
	static final	short KALIB_SPG_5        = 0xe7;
	static final	short KALIB_SPG_6        = 0xe8;
	static final	short KALIB_SPG_9        = 0xe9;
		
	static final	short DEBUG_VALUE_C      = 0xf1;
	static final	short DEBUG_VALUE_H      = 0xf2;
	static final	short DEBUG_VALUE_ADDR   = 0xf3;
	
	static final   int   CMD1_INDEX = 3;
	static final   int   CMD2_INDEX = 4;
	static final   int   TYPE_INDEX = 4;
	
	static final QRegExp regSpannung   = new QRegExp("(SPG|SPANNUNG|SPNG)");
	static final QRegExp regStrom      = new QRegExp("(STROM|STRM)");
	static final QRegExp regTemperatur = new QRegExp("(TEMPERATUR|TEMP)");
	static final QRegExp regCharge     = new QRegExp("(KAPAZITÄT|Q|AH)");
	static final QRegExp regPower      = new QRegExp("(LEISTUNG)");
	
	private static void initMap() {
		regSpannung.setCaseSensitivity  (Qt.CaseSensitivity.CaseInsensitive);
		regStrom.setCaseSensitivity     (Qt.CaseSensitivity.CaseInsensitive);
		regTemperatur.setCaseSensitivity(Qt.CaseSensitivity.CaseInsensitive);
		regCharge.setCaseSensitivity    (Qt.CaseSensitivity.CaseInsensitive);
		regPower.setCaseSensitivity     (Qt.CaseSensitivity.CaseInsensitive);
		
		typeToDesc.put(MODEL_TYPE,       "Model Type");
		typeToDesc.put(PROGRAM_REV,      "Program Rev");
		typeToDesc.put(PAR_TAB_REV,      "Par Tab Rev");
		typeToDesc.put(NENNSPNG,         "Nennspannung");
		typeToDesc.put(NENNSTROM,        "Nennstrom");
		typeToDesc.put(SERIE_NUMMER,     "Serie Nummer");
		typeToDesc.put(REP_DATUM,        "Rep Datum");
		typeToDesc.put(STANDZEIT,        "Standzeit");
		typeToDesc.put(FAHR_LADE_ZEIT,   "Fahr Lade Zeit");
		
		typeToDesc.put(BUS_ADRESSE,      "Bus Adresse");
		typeToDesc.put(COMMAND1,         "Command 1");
		typeToDesc.put(D_STATE,          "Drive State");
		typeToDesc.put(CMD2,             "Relais State");
		typeToDesc.put(PARAM_PROT,       "PARAM_PROT");
		
		typeToDesc.put(BINFO,            "BINFO");

		typeToDesc.put(LADESTROM,        "Ladestrom");
		typeToDesc.put(FAHRSTROM,        "Fahrstrom");
		typeToDesc.put(TOTAL_SPANNUNG,   "Total Spannung");
		typeToDesc.put(SOLL_LADESPG,     "Soll Ladespannung");
		typeToDesc.put(AH_ZAEHLER,       "Ah Zähler");
		typeToDesc.put(Q,                "Q");
		typeToDesc.put(LEISTUNG,         "Leistung");
		typeToDesc.put(BATTERIE_TEMP,    "Batterie Temp");
		typeToDesc.put(FINFO,            "FINFO");
		typeToDesc.put(SYM_SPANNUNG,     "Sym Spannung");		
				
		typeToDesc.put(TEILSPANNUNG1,    "Teilspannung 1");
		typeToDesc.put(TEILSPANNUNG2,    "Teilspannung 2");
		typeToDesc.put(TEILSPANNUNG3,    "Teilspannung 3"); 
		typeToDesc.put(TEILSPANNUNG4,    "Teilspannung 4"); 
		typeToDesc.put(TEILSPANNUNG5,    "Teilspannung 5"); 
		typeToDesc.put(TEILSPANNUNG6,    "Teilspannung 6"); 
		typeToDesc.put(TEILSPANNUNG7,    "Teilspannung 7"); 
			
		typeToDesc.put(TEMPERATUR1,      "Temperatur 1"); 
		typeToDesc.put(TEMPERATUR2,      "Temperatur 2"); 
		typeToDesc.put(TEMPERATUR3,      "Temperatur 3"); 
		typeToDesc.put(TEMPERATUR4,      "Temperatur 4"); 
		typeToDesc.put(TEMPERATUR5,      "Temperatur 5"); 
		typeToDesc.put(TEMPERATUR6,      "Temperatur 6"); 
		typeToDesc.put(TEMPERATUR7,      "Temperatur 7"); 
		typeToDesc.put(TEMPERATUR8,      "Temperatur 8"); 
		typeToDesc.put(TEMPERATUR9,        "Temperatur 9");
		typeToDesc.put(TEMPERATUR10,       "Temperatur 10");
		typeToDesc.put(TEMPERATUR11,       "Temperatur 11");
		typeToDesc.put(TEMPERATUR12,       "Temperatur 12");
		typeToDesc.put(TEMPERATUR13,       "Temperatur 13");
		typeToDesc.put(TEMPERATUR14,       "Temperatur 14");
		
		typeToDesc.put(SENSOR_MASK,       "Sensor Mask");		
				
		typeToDesc.put(MAX_BAT_TEMP,       "Max Bat Temp");
		typeToDesc.put(UMGEBUNGS_TEMP,     "Umgebungs Temp");
		typeToDesc.put(MAX_LADE_TEMP,      "Max Lade Temp");
		typeToDesc.put(MIN_LADE_TEMP,      "Min Lade Temp");
		typeToDesc.put(MAX_FAHR_TEMP,      "Max Fahr Temp");
		typeToDesc.put(MIN_FAHR_TEMP,      "Min Fahr Temp");
		typeToDesc.put(MAX_LAGER_TEMP,     "Max Lager Temp");
		typeToDesc.put(MIN_LAGER_TEMP,     "Min Lager Temp");

		typeToDesc.put(MAX_KAPAZITAET,     "Max Kapazität");
		typeToDesc.put(MIN_KAPAZITAET,     "Min Kapazität");
		typeToDesc.put(GELADENE_AH,        "Geladene Ah");
		typeToDesc.put(ENTLADENE_AH,       "Entladene Ah");
		typeToDesc.put(LADEZYKLEN,         "Ladezyklen");
		typeToDesc.put(TIEFENTLADE_ZYKLEN, "Tiefentladene Zyklen");
		typeToDesc.put(MAX_ENTLADE_STROM,  "Max Entlade Strom");
				
		typeToDesc.put(ZYKLUS_UEBER_110,   "Zyklus > 110%");
		typeToDesc.put(ZYKLUS_UEBER_100,   "Zyklus > 100%");
		typeToDesc.put(ZYKLUS_UEBER_90,    "Zyklus > 90%");
		typeToDesc.put(ZYKLUS_UEBER_80,    "Zyklus > 80%");
		typeToDesc.put(ZYKLUS_UEBER_70,    "Zyklus > 70%");
		typeToDesc.put(ZYKLUS_UEBER_60,    "Zyklus > 60%");
		typeToDesc.put(ZYKLUS_UEBER_50,    "Zyklus > 50%");
		typeToDesc.put(ZYKLUS_UEBER_40,    "Zyklus > 40%");
		typeToDesc.put(ZYKLUS_UEBER_30,    "Zyklus > 30%");
		typeToDesc.put(ZYKLUS_UEBER_20,    "Zyklus > 20%");
		typeToDesc.put(ZYKLUS_UEBER_10,    "Zyklus > 10%");
		typeToDesc.put(ZYKLUS_UNTER_10,    "Zyklus <= 10%");
		
		typeToDesc.put(SYMMETRIER_STROM,   "Symmetrierstrom");

		typeToDesc.put(LADE_STR_UNTER_M10, "Lade Strom < -10°C");
		typeToDesc.put(LADE_STR_UEBER_M10, "Lade Strom > -10°C");
		typeToDesc.put(LADE_STR_UEBER_P00, "Lade Strom > 0°C");
		typeToDesc.put(LADE_STR_UEBER_P10, "Lade Strom > 10°C");
		typeToDesc.put(LADE_STR_UEBER_P20, "Lade Strom > 20°C");
		typeToDesc.put(LADE_STR_UEBER_P30, "Lade Strom > 30°C");
		typeToDesc.put(LADE_STR_UEBER_P40, "Lade Strom > 40°C");
		typeToDesc.put(LADE_STR_UEBER_P45, "Lade Strom > 45°C");
		typeToDesc.put(LADE_STR_UEBER_P50, "Lade Strom > 50°C");
			
		typeToDesc.put(LADE_SPG_UNTER_M10, "Lade Spg < -10°C");
		typeToDesc.put(LADE_SPG_UEBER_M10, "Lade Spg > -10°C");
		typeToDesc.put(LADE_SPG_UEBER_P00, "Lade Spg > 0°C");
		typeToDesc.put(LADE_SPG_UEBER_P10, "Lade Spg > 10°C");
		typeToDesc.put(LADE_SPG_UEBER_P20, "Lade Spg > 20°C");
		typeToDesc.put(LADE_SPG_UEBER_P30, "Lade Spg > 30°C");
		typeToDesc.put(LADE_SPG_UEBER_P40, "Lade Spg > 40°C");
		typeToDesc.put(LADE_SPG_UEBER_P45, "Lade Spg > 45°C");
		typeToDesc.put(LADE_SPG_UEBER_P50, "Lade Spg > 50°C");
			
		typeToDesc.put(NOM_KAPAZITAET,     "Nom Kapazität");
		typeToDesc.put(MIN_FAHR_SPANNUNG,  "Min Fahr Spannung");
		typeToDesc.put(SELBST_ENTL_STROM,  "Selbst Entl Strom");
		typeToDesc.put(TIEFENTLADE_SPG,    "Tiefentlade Spg");
		typeToDesc.put(MAX_SPANNUNG_DIFF,  "Max Spannung Diff");
		typeToDesc.put(MIN_FAHR_TEMP_B,    "Min Fahr Temp B");
		typeToDesc.put(MAX_FAHR_TEMP_B,    "Max Fahr Temp B");
		typeToDesc.put(MAX_FAHR_STROM,     "Max Fahr Strom");
				
		typeToDesc.put(KAL_TEMP_7_6,       "Kal Temp 7 6");
		typeToDesc.put(KAL_TEMP_5_4,       "Kal Temp 5 4");
		typeToDesc.put(KAL_TEMP_3_2,       "Kal Temp 3 2");
		typeToDesc.put(KAL_TEMP_1_AMB,     "Kal Temp 1 AMB");
		typeToDesc.put(KAL_TEMP_GD_14,     "Kal Temp GD 14");
		typeToDesc.put(KAL_TEMP_13_12,     "Kal Temp 13 12");
		typeToDesc.put(KAL_TEMP_11_10,     "Kal Temp 11 10");
		typeToDesc.put(KAL_TEMP_9_8,       "Kal Temp 9 8");
			
		typeToDesc.put(OFFS_KLEIN_STL,     "Offs klein Stl");
		typeToDesc.put(OFFS_GROSS_STL,     "Offs gross Stl");
		typeToDesc.put(KALIB_SPG_1,        "Kalib Spg 1");
		typeToDesc.put(KALIB_SPG_2,        "Kalib Spg 2");
		typeToDesc.put(KALIB_SPG_3,        "Kalib Spg 3");
		typeToDesc.put(KALIB_SPG_4,        "Kalib Spg 4");
		typeToDesc.put(KALIB_SPG_5,        "Kalib Spg 5");
		typeToDesc.put(KALIB_SPG_6,        "Kalib Spg 6");
		typeToDesc.put(KALIB_SPG_9,        "Kalib Spg 9");
			
		typeToDesc.put(DEBUG_VALUE_C,      "Debug Value C");
		typeToDesc.put(DEBUG_VALUE_H,      "Debug Value H");
		typeToDesc.put(DEBUG_VALUE_ADDR,   "Debug Value Addr");
	}
	
	
	public Request() {
		if(typeToDesc.isEmpty()) {
			initMap();
		}
	}
	
	public String decodeRequest(QByteArray data) {
		Short type = requestType(data);
		
		String description = typeToDesc.get(type);
		if( description == null) {
			description = "UNKNOWN";
		}
		return description;		
	}

	public Short requestType(QByteArray data) {
		Short type = new Short( data.at(TYPE_INDEX) );
		type &= 0xFF;
		return type;
	}
	
	public static EUnit getUnit(short type) {
		String description = typeToDesc.get(type);
		
		EUnit eUnit = EUnit.eUnknown;
		
		if( description != null) {

			if( regSpannung.indexIn(description) != -1) {
				eUnit = EUnit.eVoltage;
			} 
			else if( regStrom.indexIn(description) != -1) {
				eUnit = EUnit.eCurrent;
			}
			else if( regTemperatur.indexIn(description) != -1) {
				eUnit = EUnit.eTemperatur;
			}
			else if( regCharge.indexIn(description) != -1) {
				eUnit = EUnit.eCharge;
			}
			else if( regPower.indexIn(description) != -1) {
				eUnit = EUnit.ePower;
			}
		}		
		return eUnit;
	}
}
