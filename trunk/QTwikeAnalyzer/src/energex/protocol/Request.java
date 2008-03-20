package energex.protocol;

import java.util.HashMap;
import java.util.Map;

import com.trolltech.qt.core.QByteArray;

public class Request {
	Map<Short,String> typeToDesc = new HashMap<Short, String>();
	
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
	static final	short COMMAND2           = 0x12;
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
	
	public Request() {
		typeToDesc.put(MODEL_TYPE,       "MODEL_TYPE");
		typeToDesc.put(PROGRAM_REV,      "PROGRAM_REV");
		typeToDesc.put(PAR_TAB_REV,      "PAR_TAB_REV");
		typeToDesc.put(NENNSPNG,         "NENNSPNG");
		typeToDesc.put(NENNSTROM,        "NENNSTROM");
		typeToDesc.put(SERIE_NUMMER,     "SERIE_NUMMER");
		typeToDesc.put(REP_DATUM,        "REP_DATUM");
		typeToDesc.put(STANDZEIT,        "STANDZEIT");
		typeToDesc.put(FAHR_LADE_ZEIT,   "FAHR_LADE_ZEIT");
		
		typeToDesc.put(BUS_ADRESSE,      "BUS_ADRESSE");
		typeToDesc.put(COMMAND1,         "COMMAND1");
		typeToDesc.put(D_STATE,          "D_STATE");
		typeToDesc.put(COMMAND2,         "COMMAND2");
		typeToDesc.put(PARAM_PROT,       "PARAM_PROT");
		
		typeToDesc.put(BINFO,            "BINFO");

		typeToDesc.put(LADESTROM,        "LADESTROM");
		typeToDesc.put(FAHRSTROM,        "FAHRSTROM");
		typeToDesc.put(TOTAL_SPANNUNG,   "TOTAL_SPANNUNG");
		typeToDesc.put(SOLL_LADESPG,     "SOLL_LADESPG");
		typeToDesc.put(AH_ZAEHLER,       "AH_ZAEHLER");
		typeToDesc.put(Q,                "Q");
		typeToDesc.put(LEISTUNG,         "LEISTUNG");
		typeToDesc.put(BATTERIE_TEMP,    "BATTERIE_TEMP");
		typeToDesc.put(FINFO,            "FINFO");
				
		typeToDesc.put(TEILSPANNUNG1,    "TEILSPANNUNG1");
		typeToDesc.put(TEILSPANNUNG2,    "TEILSPANNUNG1");
		typeToDesc.put(TEILSPANNUNG3,    "TEILSPANNUNG3"); 
		typeToDesc.put(TEILSPANNUNG4,    "TEILSPANNUNG4"); 
		typeToDesc.put(TEILSPANNUNG5,    "TEILSPANNUNG5"); 
		typeToDesc.put(TEILSPANNUNG6,    "TEILSPANNUNG6"); 
		typeToDesc.put(TEILSPANNUNG7,    "TEILSPANNUNG7"); 
			
		typeToDesc.put(TEMPERATUR1,      "TEMPERATUR1"); 
		typeToDesc.put(TEMPERATUR2,      "TEMPERATUR2"); 
		typeToDesc.put(TEMPERATUR3,      "TEMPERATUR3"); 
		typeToDesc.put(TEMPERATUR4,      "TEMPERATUR4"); 
		typeToDesc.put(TEMPERATUR5,      "TEMPERATUR5"); 
		typeToDesc.put(TEMPERATUR6,      "TEMPERATUR6"); 
		typeToDesc.put(TEMPERATUR7,      "TEMPERATUR7"); 
		typeToDesc.put(TEMPERATUR8,      "TEMPERATUR8"); 
		typeToDesc.put(TEMPERATUR9,        "TEMPERATUR9");
		typeToDesc.put(TEMPERATUR10,       "TEMPERATUR10");
		typeToDesc.put(TEMPERATUR11,       "TEMPERATUR11");
		typeToDesc.put(TEMPERATUR12,       "TEMPERATUR12");
		typeToDesc.put(TEMPERATUR13,       "TEMPERATUR13");
		typeToDesc.put(TEMPERATUR14,       "TEMPERATUR14");
				
		typeToDesc.put(MAX_BAT_TEMP,       "MAX_BAT_TEMP");
		typeToDesc.put(UMGEBUNGS_TEMP,     "UMGEBUNGS_TEMP");
		typeToDesc.put(MAX_LADE_TEMP,      "MAX_LADE_TEMP");
		typeToDesc.put(MIN_LADE_TEMP,      "MIN_LADE_TEMP");
		typeToDesc.put(MAX_FAHR_TEMP,      "MAX_FAHR_TEMP");
		typeToDesc.put(MIN_FAHR_TEMP,      "MIN_FAHR_TEMP");
		typeToDesc.put(MAX_LAGER_TEMP,     "MAX_LAGER_TEMP");
		typeToDesc.put(MIN_LAGER_TEMP,     "MIN_LAGER_TEMP");

		typeToDesc.put(MAX_KAPAZITAET,     "MAX_KAPAZITAET");
		typeToDesc.put(MIN_KAPAZITAET,      "MIN_KAPAZITAET");
		typeToDesc.put(GELADENE_AH,        "GELADENE_AH");
		typeToDesc.put(ENTLADENE_AH,       "ENTLADENE_AH");
		typeToDesc.put(LADEZYKLEN,         "LADEZYKLEN");
		typeToDesc.put(TIEFENTLADE_ZYKLEN, "TIEFENTLADE_ZYKLEN");
		typeToDesc.put(MAX_ENTLADE_STROM,  "MAX_ENTLADE_STROM");
				
		typeToDesc.put(ZYKLUS_UEBER_110,   "ZYKLUS_UEBER_110");
		typeToDesc.put(ZYKLUS_UEBER_100,   "ZYKLUS_UEBER_100");
		typeToDesc.put(ZYKLUS_UEBER_90,    "ZYKLUS_UEBER_90");
		typeToDesc.put(ZYKLUS_UEBER_80,    "ZYKLUS_UEBER_80");
		typeToDesc.put(ZYKLUS_UEBER_70,    "ZYKLUS_UEBER_70");
		typeToDesc.put(ZYKLUS_UEBER_60,    "ZYKLUS_UEBER_60");
		typeToDesc.put(ZYKLUS_UEBER_50,    "ZYKLUS_UEBER_50");
		typeToDesc.put(ZYKLUS_UEBER_40,    "ZYKLUS_UEBER_40");
		typeToDesc.put(ZYKLUS_UEBER_30,    "ZYKLUS_UEBER_30");
		typeToDesc.put(ZYKLUS_UEBER_20,    "ZYKLUS_UEBER_20");
		typeToDesc.put(ZYKLUS_UEBER_10,    "ZYKLUS_UEBER_10");
		typeToDesc.put(ZYKLUS_UNTER_10,    "ZYKLUS_UNTER_10");

		typeToDesc.put(LADE_STR_UNTER_M10, "LADE_STR_UNTER_M10");
		typeToDesc.put(LADE_STR_UEBER_M10, "LADE_STR_UEBER_M10");
		typeToDesc.put(LADE_STR_UEBER_P00, "LADE_STR_UEBER_P00");
		typeToDesc.put(LADE_STR_UEBER_P10, "LADE_STR_UEBER_P10");
		typeToDesc.put(LADE_STR_UEBER_P20, "LADE_STR_UEBER_P20");
		typeToDesc.put(LADE_STR_UEBER_P30, "LADE_STR_UEBER_P30");
		typeToDesc.put(LADE_STR_UEBER_P40, "LADE_STR_UEBER_P40");
		typeToDesc.put(LADE_STR_UEBER_P45, "LADE_STR_UEBER_P45");
		typeToDesc.put(LADE_STR_UEBER_P50, "LADE_STR_UEBER_P50");
			
		typeToDesc.put(LADE_SPG_UNTER_M10, "LADE_SPG_UNTER_M10");
		typeToDesc.put(LADE_SPG_UEBER_M10, "LADE_SPG_UEBER_M10");
		typeToDesc.put(LADE_SPG_UEBER_P00, "LADE_SPG_UEBER_P00");
		typeToDesc.put(LADE_SPG_UEBER_P10, "LADE_SPG_UEBER_P10");
		typeToDesc.put(LADE_SPG_UEBER_P20, "LADE_SPG_UEBER_P20");
		typeToDesc.put(LADE_SPG_UEBER_P30, "LADE_SPG_UEBER_P30");
		typeToDesc.put(LADE_SPG_UEBER_P40, "LADE_SPG_UEBER_P40");
		typeToDesc.put(LADE_SPG_UEBER_P45, "LADE_SPG_UEBER_P45");
		typeToDesc.put(LADE_SPG_UEBER_P50, "LADE_SPG_UEBER_P50");
			
		typeToDesc.put(NOM_KAPAZITAET,     "NOM_KAPAZITAET");
		typeToDesc.put(MIN_FAHR_SPANNUNG,  "MIN_FAHR_SPANNUNG");
		typeToDesc.put(SELBST_ENTL_STROM,  "SELBST_ENTL_STROM");
		typeToDesc.put(TIEFENTLADE_SPG,    "TIEFENTLADE_SPG");
		typeToDesc.put(MAX_SPANNUNG_DIFF,  "MAX_SPANNUNG_DIFF");
		typeToDesc.put(MIN_FAHR_TEMP_B,    "MIN_FAHR_TEMP_B");
		typeToDesc.put(MAX_FAHR_TEMP_B,    "MAX_FAHR_TEMP_B");
		typeToDesc.put(MAX_FAHR_STROM,     "MAX_FAHR_STROM");
				
		typeToDesc.put(KAL_TEMP_7_6,       "KAL_TEMP_7_6");
		typeToDesc.put(KAL_TEMP_5_4,       "KAL_TEMP_5_4");
		typeToDesc.put(KAL_TEMP_3_2,       "KAL_TEMP_3_2");
		typeToDesc.put(KAL_TEMP_1_AMB,     "KAL_TEMP_1_AMB");
		typeToDesc.put(KAL_TEMP_GD_14,     "KAL_TEMP_GD_14");
		typeToDesc.put(KAL_TEMP_13_12,     "KAL_TEMP_13_12");
		typeToDesc.put(KAL_TEMP_11_10,     "KAL_TEMP_11_10");
		typeToDesc.put(KAL_TEMP_9_8,       "KAL_TEMP_9_8");
			
		typeToDesc.put(OFFS_KLEIN_STL,     "OFFS_KLEIN_STL");
		typeToDesc.put(OFFS_GROSS_STL,     "OFFS_GROSS_STL");
		typeToDesc.put(KALIB_SPG_1,        "KALIB_SPG_1");
		typeToDesc.put(KALIB_SPG_2,        "KALIB_SPG_2");
		typeToDesc.put(KALIB_SPG_3,        "KALIB_SPG_3");
		typeToDesc.put(KALIB_SPG_4,        "KALIB_SPG_4");
		typeToDesc.put(KALIB_SPG_5,        "KALIB_SPG_5");
		typeToDesc.put(KALIB_SPG_6,        "KALIB_SPG_6");
		typeToDesc.put(KALIB_SPG_9,        "KALIB_SPG_9");
			
		typeToDesc.put(DEBUG_VALUE_C,      "DEBUG_VALUE_C");
		typeToDesc.put(DEBUG_VALUE_H,      "DEBUG_VALUE_H");
		typeToDesc.put(DEBUG_VALUE_ADDR,   "DEBUG_VALUE_ADDR");
	}
	
	public String decodeRequest(QByteArray data) {
		short requestType = data.at(3);
		String description = typeToDesc.get(requestType);
		if( description == null) {
			description = "UNKNOWN";
		}
		return description;		
	}
}
