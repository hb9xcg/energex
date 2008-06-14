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

public class DriveState {
	private static final byte  CONVERTER_OFF   = 0; // InvOff           Umrichter aus
	private static final byte  CONVERTER_TEST  = 1; // InvTest          Test-Modus
	private static final byte  CONVERTER_PROG  = 2; // InvProg          Programm-Modus
	private static final byte  CONVERTER_IDLE  = 3; // InvIdle          Umschaltungszustand vor dem Fahren
	private static final byte  BREAK_DOWN      = 4; // BreakDown        Fehler des Umrichters
	private static final byte  DRIVE           = 5; // Drive            Fahren
	private static final byte  READY_CHARGE    = 6; // ReadyCharge      warten auf Netzspannung
	private static final byte  PRE_CHARGE      = 7; // PreCharge        Vorladung
	private static final byte  CLOSE_PC_RELAIS = 8; // CloseVRelais     Vorladerelais schliessen
	private static final byte  I_CHARGE        = 9; // ICharge          I-Ladung
	private static final byte  U_CHARGE        =10; // UCharge          U-Ladung
	private static final byte  OPEN_B_RELAIS   =11; // OpenBRelais      Batterierelais öffnen
	private static final byte  POST_CHARGE     =12; // PostCharge       Nachladung
	private static final byte  CLOSE_B_RELAIS  =13; // CloseBRelais     Batterierelais schliessen
	private static final byte  SYMM_CHARGE     =14; // SymmCharge       Symmetrierladung
	private static final byte  TRICKLE_CHARGE  =15; // TrickleCharge    Erhaltungsladung
	private static final byte  TRICKLE_WAIT    =16; // TrickleWait      Pause
	private static final byte  UNKNOWN_17      =17; // (?)            (möglicherweise Erhaltungsladung einzelner Blöcke?)
	private static final byte  UNKNOWN_18      =18; // (?)            (möglicherweise Erhaltungsladung einzelner Blöcke?)
	private static final byte  UNKNOWN_19      =19; // (?)            (möglicherweise Erhaltungsladung einzelner Blöcke?)
	private static final byte  UNKNOWN_20      =20; // (?)            (möglicherweise Erhaltungsladung einzelner Blöcke?)

	static public String decodeDriveState(int driveState) {
		String description = null;
		
		switch(driveState) {
		case CONVERTER_OFF:
			description = "Umrichter aus";
			break;
		case CONVERTER_TEST:  
			description = "Test-Modus";
			break;
		case CONVERTER_PROG:  
			description = "Programm-Modus";
			break;
		case CONVERTER_IDLE:  
			description = "Umrichter idle";
			break;
		case BREAK_DOWN:  
			description = "Fehler des Umrichters";
			break;
		case DRIVE:  
			description = "Fahren";
			break;
		case READY_CHARGE:  
			description = "warten auf Netzspannung";
			break;
		case PRE_CHARGE:  
			description = "Vorladung";
			break;
		case CLOSE_PC_RELAIS:  
			description = "Vorladerelais schliessen";
			break;
		case I_CHARGE:  
			description = "I-Ladung";
			break;
		case U_CHARGE:  
			description = "U-Ladung";
			break;
		case OPEN_B_RELAIS:  
			description = "Batterierelais öffnen";
			break;
		case POST_CHARGE:  
			description = "Nachladung";
			break;
		case CLOSE_B_RELAIS:  
			description = "Batterierelais schliessen";
			break;
		case SYMM_CHARGE:  
			description = "Symmetrierladung";
			break;
		case TRICKLE_CHARGE:  
			description = "Erhaltungsladung";
			break;
		case TRICKLE_WAIT:  
			description = "Pause";
			break;
		case UNKNOWN_17:  
			description = "(möglicherweise Erhaltungsladung einzelner Blöcke?)";
			break;
		case UNKNOWN_18:  
			description = "(möglicherweise Erhaltungsladung einzelner Blöcke?)";
			break;
		case UNKNOWN_19:  
			description = "(möglicherweise Erhaltungsladung einzelner Blöcke?)";
			break;
		case UNKNOWN_20:
			description = "(möglicherweise Erhaltungsladung einzelner Blöcke?)";
			break;
		default:
			description = "Unexpected value";
		}
		return description;
	}
}
