#include <QtGlobal>
#include <QTime>
#include <QRegExp>
#include "decoder.h"
#include "protocol.h"

Decoder::Decoder()
{
    time = new QTime();
    time->start();

    eState = eExpectStart;

    regSpannung   = new QRegExp("(SPG|SPANNUNG|SPNG)");
    regStrom      = new QRegExp("(STROM|STRM)");
    regTemperatur = new QRegExp("(TEMPERATUR|TEMP)");
    regCharge     = new QRegExp("(KAPAZITÄT|Q|AH)");
    regPower      = new QRegExp("(LEISTUNG)");
    regSpeed      = new QRegExp("(GESCHWINDIGKEIT)");
    regDistance   = new QRegExp("(KILOMETER)");

    regSpannung->setCaseSensitivity  (Qt::CaseInsensitive);
    regStrom->setCaseSensitivity     (Qt::CaseInsensitive);
    regTemperatur->setCaseSensitivity(Qt::CaseInsensitive);
    regCharge->setCaseSensitivity    (Qt::CaseInsensitive);
    regPower->setCaseSensitivity     (Qt::CaseInsensitive);
    regSpeed->setCaseSensitivity     (Qt::CaseInsensitive);
    regDistance->setCaseSensitivity  (Qt::CaseInsensitive);
}

Decoder::~Decoder()
{
    delete regSpannung;
    delete regStrom;
    delete regTemperatur;
    delete regCharge;
    delete regPower;
    delete regSpeed;
    delete regDistance;

    delete time;
}

void Decoder::resetTime()
{
    time->restart();
}

void Decoder::receiveByte(const char character)
{
    quint8 byte = static_cast<quint8>(character);

    switch(eState)
    {
    case eExpectStart:
        switch(byte)
        {
        case FRAME:
            eState = eExpectAddress;
            break;
        default:
            break;
        }
        break;

    case eExpectAddress:
        switch(byte)
        {
        case BATTERY_1:
        case BATTERY_2:
        case BATTERY_3:
        case BROADCAST:
            address = byte;
            eState  = eExpectCommand;
            break;
        case FRAME:
            eState = eExpectAddress;
            break;
        default:
            eState = eExpectStart;
        }
        break;

    case eExpectCommand:
        switch(byte)
        {
        case REQ_DATA:
        case REQ_GROUP:
        case TRM_DATA:
        case TRM_GROUP:
            command  = byte;
            checksum = byte;
            length   = byte & LENGTH_MASK;
            idx      = 0;
            frameDetection = true;
            eState   = eExpectData;
            break;
        case FRAME:
            eState = eExpectAddress;
            break;
        default:
            eState = eExpectStart;
        }
        break;

    case eExpectData:
        if( frameDetection && byte == FRAME)
        {
            frameDetection = false;
        }
        else
        {
            if (frameDetection == false && byte != FRAME)
            {
                // Single frame byte!
                // -> Reset state machine to start condition.
                address = byte;
                eState = eExpectCommand;
                break;
            }
            frameDetection = true;
            data[idx++] = byte;
            checksum ^= byte;
        }

        if(idx >= length)
        {
            frameDetection = true;
            eState = eExpectChecksum;
        }
        break;

    case eExpectChecksum:
        if (frameDetection && byte == FRAME)
        {
            frameDetection = false;
        }
        else
        {
            if (frameDetection == false && byte != FRAME)
            {
                // Single frame byte!
                // -> Reset state machine to start condition.
                address = byte;
                eState = eExpectCommand;
                break;
            }
            if(checksum==byte)
            {
                receivePacket();
            }
            else
            {
                receiveRubish(length, byte);
            }
            eState = eExpectStart;
        }
        break;
    }
}

void Decoder::receivePacket(void)
{
    if(address==BROADCAST)
    {
        switch(command)
        {
        case TRM_DATA:
            transmitData();
            break;
        default:
            break;
        }
    }
    else
    {
        switch(command)
        {
        case REQ_DATA:
            requestData();
            break;
        case REQ_GROUP:
            requestGroup();
            break;
        case TRM_DATA:
            transmitData();
            break;
        case TRM_GROUP:
            transmitGroup();
            break;
        }
    }
}

void Decoder::transmitData(void)
{
    qint16 value;
    qint16 parameter;

    parameter = data[1] | (data[0]<<8);
    value     = data[3] | (data[2]<<8);

    QString strTime, strRaw, strAddress, strType, strContent, strChecksum;

    strRaw += QString("%1").arg(FRAME, 2, 16, QLatin1Char('0')) + " ";
    strRaw += QString("%1").arg(address, 2, 16, QLatin1Char('0')) + " ";
    strRaw += QString("%1").arg(TRM_GROUP, 2, 16, QLatin1Char('0')) + " ";

    for(int idx=0; idx<4; idx++)
    {
        strRaw += QString("%1").arg(data[idx], 2, 16, QLatin1Char('0')) + " ";
    }

    strType = "TrmData";

    strAddress = decodeAddress(address);

    strContent = decodeValue(parameter, value);

    strTime = QString("%1").arg(time->elapsed()) + "ms";

    strChecksum = "Valid";

    emit receiveMessage(strTime, strRaw, strAddress, strType, strContent, strChecksum);
}

void Decoder::transmitGroup(void)
{
    qint16 current;
    quint16 binfo, voltage, value;

    binfo   = data[1] | (data[0]<<8);
    current = data[3] | (data[2]<<8);
    voltage = data[5] | (data[4]<<8);
    value   = data[7] | (data[6]<<8);

    QString strTime, strRaw, strAddress, strType, strContent, strChecksum;

    strRaw += QString("%1").arg(FRAME, 2, 16, QLatin1Char('0')) + " ";
    strRaw += QString("%1").arg(address, 2, 16, QLatin1Char('0')) + " ";
    strRaw += QString("%1").arg(TRM_GROUP, 2, 16, QLatin1Char('0')) + " ";

    for(int idx=0; idx<8; idx++)
    {
        strRaw += QString("%1").arg(data[idx], 2, 16, QLatin1Char('0')) + " ";
    }

    strRaw += QString("%1").arg(checksum, 2, 16, QLatin1Char('0'));

    strTime = QString("%1").arg(time->elapsed()) + "ms";

    strAddress = decodeAddress(address);

    strType = "TrmGroup";

    strContent += QString("%1").arg(float(current)/100.0) + "A, ";
    strContent += QString("%1").arg(float(voltage)/100.0) + "V, ";
    strContent += decodeValue(parameter, value);
    strContent += decodeBInfo(binfo);

    strChecksum = "Valid";
    if (parameter >= TEMPERATUR1 &&
        parameter <= TEMPERATUR14  )
    {
        switch(address)
        {
        case BATTERY_1:
            emit temperature1(time->elapsed(), TEMPERATUR14 - parameter, float(value)/100);
            break;
        case BATTERY_2:
            emit temperature2(time->elapsed(), TEMPERATUR14 - parameter, float(value)/100);
            break;
        case BATTERY_3:
            emit temperature3(time->elapsed(), TEMPERATUR14 - parameter, float(value)/100);
            break;
        default:
            break;
        }
    }

    switch(address)
    {
    case BATTERY_1:
        emit battery1(time->elapsed(), float(voltage)/100.0, float(current)/100.0);
        break;
    case BATTERY_2:
        emit battery2(time->elapsed(), float(voltage)/100.0, float(current)/100.0);
        break;
    case BATTERY_3:
        emit battery3(time->elapsed(), float(voltage)/100.0, float(current)/100.0);
        break;
    default:
        break;
    }

    emit receiveMessage(strTime, strRaw, strAddress, strType, strContent, strChecksum);
}

void Decoder::requestData(void)
{
    qint16 value;

    value = data[3] | (data[2]<<8);

    parameter = value;

    QString strTime, strRaw, strAddress, strType, strContent, strChecksum;

    strRaw += QString("%1").arg(FRAME,    2, 16, QLatin1Char('0')) + " ";
    strRaw += QString("%1").arg(address,  2, 16, QLatin1Char('0')) + " ";
    strRaw += QString("%1").arg(REQ_DATA, 2, 16, QLatin1Char('0')) + " ";

    for(int idx=0; idx<4; idx++)
    {
        strRaw += QString("%1").arg(data[idx], 2, 16, QLatin1Char('0'));
    }

    strAddress = decodeAddress(address);

    strType = "ReqData";

    strTime = QString("%1").arg(time->elapsed()) + "ms";

    strChecksum = "Valid";

    emit receiveMessage(strTime, strRaw, strAddress, strType, strContent, strChecksum);
}

void Decoder::requestGroup(void)
{
    parameter = data[1] | (data[0]<<8);

    QString strTime, strRaw, strAddress, strType, strContent, strChecksum;

    strRaw += QString("%1").arg(FRAME,     2, 16, QLatin1Char('0')) + " ";
    strRaw += QString("%1").arg(address,   2, 16, QLatin1Char('0')) + " ";
    strRaw += QString("%1").arg(REQ_GROUP, 2, 16, QLatin1Char('0')) + " ";

    for(int idx=0; idx<2; idx++)
    {
        strRaw += QString("%1").arg(data[idx], 2, 16, QLatin1Char('0')) + " ";
    }

    strRaw += QString("%1").arg(checksum, 2, 16, QLatin1Char('0'));

    strTime = QString("%1").arg(time->elapsed()) + "ms";

    strAddress = decodeAddress(address);

    strType = "ReqGroup";

    strContent += "Request " + decodeParameter(parameter);

    strChecksum = "Valid";

    emit receiveMessage(strTime, strRaw, strAddress, strType, strContent, strChecksum);
}

void Decoder::receiveRubish(quint8 lenght, quint8 checksum)
{
    QString strTime, strRaw, strAddress, strType, strContent, strChecksum;

    strRaw += QString("%1").arg(FRAME,   2, 16, QLatin1Char('0')) + " ";
    strRaw += QString("%1").arg(address, 2, 16, QLatin1Char('0')) + " ";
    strRaw += QString("%1").arg(command, 2, 16, QLatin1Char('0')) + " ";

    for(int idx=0; idx<lenght; idx++)
    {
        strRaw += QString("%1").arg(data[idx], 2, 16, QLatin1Char('0')) + " ";
    }

    strRaw += QString("%1").arg(checksum, 2, 16, QLatin1Char('0'));

    strTime = QString("%1").arg(time->elapsed()) + "ms";

    strChecksum = "Invalid";

    emit receiveMessage(strTime, strRaw, strAddress, strType, strContent, strChecksum);
}

QString Decoder::decodeAddress(quint8 address)
{
    switch(address)
    {
        case BATTERY_1: return "Battery 1";
        case BATTERY_2: return "Battery 2";
        case BATTERY_3: return "Battery 3";
        case BROADCAST: return "Broadcast";
        default:        return "Unknown";
    }
}

QString Decoder::decodeValue(quint16 parameter, quint16 unsignedValue)
{
    QString strValue(decodeParameter(parameter));
    qint16 signedValue = static_cast<qint16>(unsignedValue);

    strValue += " ";
    Decoder::EUnit eUnit = getUnit(parameter);
    if (parameter == ENTLADENE_AH)
    {
        strValue += QString("%1").arg(unsignedValue);
    }
    else if (parameter == LEISTUNG        ||
             parameter == GESCHWINDIGKEIT )
    {
        strValue += QString("%1").arg(signedValue);
    }
    else if (parameter == DRIVE_STATE)
    {
        return "State " + decodeDriveState(EDriveState(unsignedValue));
    }
    else if (parameter == SOLL_LADESPG)
    {
        strValue += QString("%1").arg(float(unsignedValue)/100);
    }
    else
    {
        strValue += QString("%1").arg(float(signedValue)/100);
    }

    strValue += decodeUnit(eUnit);

    return strValue;
}

QString Decoder::decodeParameter(quint16 parameter)
{
    switch(parameter)
    {
        case  MODEL_TYPE:         return "Model Type";
        case  PROGRAM_REV:        return "Program Rev";
        case  PAR_TAB_REV:        return "Par Tab Rev";
        case  NENNSPNG:           return "Nennspannung";
        case  NENNSTROM:          return "Nennstrom";
        case  SERIE_NUMMER:       return "Serie Nummer";
        case  REP_DATUM:          return "Rep Datum";
        case  STANDZEIT:          return "Standzeit";
        case  FAHR_LADE_ZEIT:     return "Fahr Lade Zeit";

        case  BUS_ADRESSE:        return "Bus Adresse";
        case  COMMAND1:           return "Command 1";
        case  DRIVE_STATE:        return "Drive State";
        case  COMMAND:            return "Relais State";
        case  PARAM_PROT:         return "PARAM_PROT";

        case  BINFO:              return "BINFO";

        case  LADESTROM:          return "Ladestrom";
        case  FAHRSTROM:          return "Fahrstrom";
        case  TOTAL_SPANNUNG:     return "Total Spannung";
        case  SOLL_LADESPG:       return "Soll Ladespannung";
        case  AH_ZAEHLER:         return "Ah Zähler";
        case  Q:                  return "Q";
        case  LEISTUNG:           return "Leistung";
        case  BATTERIE_TEMP:      return "Batterie Temp";
        case  FINFO:              return "FINFO";
        case  SYM_SPANNUNG:       return "Sym Spannung";

        case  TEILSPANNUNG1:      return "Teilspannung 1";
        case  TEILSPANNUNG2:      return "Teilspannung 2";
        case  TEILSPANNUNG3:      return "Teilspannung 3";
        case  TEILSPANNUNG4:      return "Teilspannung 4";
        case  TEILSPANNUNG5:      return "Teilspannung 5";
        case  TEILSPANNUNG6:      return "Teilspannung 6";
        case  TEILSPANNUNG7:      return "Teilspannung 7";

        case  TEMPERATUR1:        return "Temperatur 1";
        case  TEMPERATUR2:        return "Temperatur 2";
        case  TEMPERATUR3:        return "Temperatur 3";
        case  TEMPERATUR4:        return "Temperatur 4";
        case  TEMPERATUR5:        return "Temperatur 5";
        case  TEMPERATUR6:        return "Temperatur 6";
        case  TEMPERATUR7:        return "Temperatur 7";
        case  TEMPERATUR8:        return "Temperatur 8";
        case  TEMPERATUR9:        return "Temperatur 9";
        case  TEMPERATUR10:       return "Temperatur 10";
        case  TEMPERATUR11:       return "Temperatur 11";
        case  TEMPERATUR12:       return "Temperatur 12";
        case  TEMPERATUR13:       return "Temperatur 13";
        case  TEMPERATUR14:       return "Temperatur 14";

        case  SENSOR_MASK:        return "Sensor Mask";

        case  MAX_BAT_TEMP:       return "Max Bat Temp";
        case  UMGEBUNGS_TEMP:     return "Umgebungs Temp";
        case  MAX_LADETEMP:       return "Max Lade Temp";
        case  MIN_LADETEMP:       return "Min Lade Temp";
        case  MAX_FAHRTEMP:       return "Max Fahr Temp";
        case  MIN_FAHRTEMP:       return "Min Fahr Temp";
        case  MAX_LAGERTEMP:      return "Max Lager Temp";
        case  MIN_LAGERTEMP:      return "Min Lager Temp";

        case  MAX_KAPAZITAET:     return "Max Kapazität";
        case  MIN_KAPAZITAET:     return "Min Kapazität";
        case  GELADENE_AH:        return "Geladene Ah";
        case  ENTLADENE_AH:       return "Entladene Ah";
        case  LADEZYKLEN:         return "Ladezyklen";
        case  TIEFENTLADE_ZYKLEN: return "Tiefentladene Zyklen";
        case  MAX_ENTLADE_STROM:  return "Max Entlade Strom";

        case  ZYKLUS_UEBER_110:   return "Zyklus > 110%";
        case  ZYKLUS_UEBER_100:   return "Zyklus > 100%";
        case  ZYKLUS_UEBER_90:    return "Zyklus > 90%";
        case  ZYKLUS_UEBER_80:    return "Zyklus > 80%";
        case  ZYKLUS_UEBER_70:    return "Zyklus > 70%";
        case  ZYKLUS_UEBER_60:    return "Zyklus > 60%";
        case  ZYKLUS_UEBER_50:    return "Zyklus > 50%";
        case  ZYKLUS_UEBER_40:    return "Zyklus > 40%";
        case  ZYKLUS_UEBER_30:    return "Zyklus > 30%";
        case  ZYKLUS_UEBER_20:    return "Zyklus > 20%";
        case  ZYKLUS_UEBER_10:    return "Zyklus > 10%";
        case  ZYKLUS_UNTER_10:    return "Zyklus <= 10%";

        case  SYMMETRIER_STROM:   return "Symmetrierstrom";

        case  LADE_STR_UNTER_M10: return "Lade Strom < -10°C";
        case  LADE_STR_UEBER_M10: return "Lade Strom > -10°C";
        case  LADE_STR_UEBER_P00: return "Lade Strom > 0°C";
        case  LADE_STR_UEBER_P10: return "Lade Strom > 10°C";
        case  LADE_STR_UEBER_P20: return "Lade Strom > 20°C";
        case  LADE_STR_UEBER_P30: return "Lade Strom > 30°C";
        case  LADE_STR_UEBER_P40: return "Lade Strom > 40°C";
        case  LADE_STR_UEBER_P45: return "Lade Strom > 45°C";
        case  LADE_STR_UEBER_P50: return "Lade Strom > 50°C";

        case  LADE_SPG_UNTER_M10: return "Lade Spg < -10°C";
        case  LADE_SPG_UEBER_M10: return "Lade Spg > -10°C";
        case  LADE_SPG_UEBER_P00: return "Lade Spg > 0°C";
        case  LADE_SPG_UEBER_P10: return "Lade Spg > 10°C";
        case  LADE_SPG_UEBER_P20: return "Lade Spg > 20°C";
        case  LADE_SPG_UEBER_P30: return "Lade Spg > 30°C";
        case  LADE_SPG_UEBER_P40: return "Lade Spg > 40°C";
        case  LADE_SPG_UEBER_P45: return "Lade Spg > 45°C";
        case  LADE_SPG_UEBER_P50: return "Lade Spg > 50°C";

        case  NOM_KAPAZITAET:     return "Nom Kapazität";
        case  MIN_FAHR_SPANNUNG:  return "Min Fahr Spannung";
        case  SELBST_ENTL_STROM:  return "Selbst Entl Strom";
        case  TIEFENTLADE_SPG:    return "Tiefentlade Spg";
        case  MAX_SPANNUNG_DIFF:  return "Max Spannung Diff";
        case  MIN_FAHR_TEMP_B:    return "Min Fahr Temp B";
        case  MAX_FAHR_TEMP_B:    return "Max Fahr Temp B";
        case  MAX_FAHR_STROM:     return "Max Fahr Strom";

        case  KAL_TEMP_7_6:       return "Kal Temp 7 6";
        case  KAL_TEMP_5_4:       return "Kal Temp 5 4";
        case  KAL_TEMP_3_2:       return "Kal Temp 3 2";
        case  KAL_TEMP_1_AMB:     return "Kal Temp 1 AMB";
        case  KAL_TEMP_GD_14:     return "Kal Temp GD 14";
        case  KAL_TEMP_13_12:     return "Kal Temp 13 12";
        case  KAL_TEMP_11_10:     return "Kal Temp 11 10";
        case  KAL_TEMP_9_8:       return "Kal Temp 9 8";

        case  OFFS_KLEIN_STL:     return "Offs klein Stl";
        case  OFFS_GROSS_STL:     return "Offs gross Stl";
        case  KALIB_SPG_1:        return "Kalib Spg 1";
        case  KALIB_SPG_2:        return "Kalib Spg 2";
        case  KALIB_SPG_3:        return "Kalib Spg 3";
        case  KALIB_SPG_4:        return "Kalib Spg 4";
        case  KALIB_SPG_5:        return "Kalib Spg 5";
        case  KALIB_SPG_6:        return "Kalib Spg 6";
        case  KALIB_SPG_9:        return "Kalib Spg 9";

        case  DEBUG_VALUE_C:      return "Debug Value C";
        case  DEBUG_VALUE_H:      return "Debug Value H";
        case  DEBUG_VALUE_ADDR:   return "Debug Value Addr";

        case GESCHWINDIGKEIT:     return "Geschwindigkeit";
        case TAGESKILOMETER:      return "Tageskilometer";
        default:                  return "Unknown";
    }
}

QString Decoder::decodeDriveState(EDriveState state)
{
    switch(state)
    {
    case eConverterOff:     return "InvOff";        // Umrichter aus
    case eConverterTest:    return "InvTest";       // Test-Modus
    case eConverterProg:    return "InvProg";       // Programm-Modus
    case eConverterIdle:    return "InvIdle";       // Umschaltungszustand vor dem Fahren
    case eBreakDown:        return "BreakDown";     // Fehler des Umrichters
    case eDrive:            return "Drive";         // Fahren
    case eReadyCharge:      return "ReadyCharge";   // warten auf Netzspannung
    case ePreCharge:        return "PreCharge";     // Vorladung
    case eClosePCRelais:    return "CloseVRelais";  // Vorladerelais schliessen
    case eICharge:          return "ICharge";       // I-Ladung
    case eUCharge:          return "UCharge";       // U-Ladung
    case eOpenBatRelais:    return "OpenBRelais";   // Batterierelais öffnen
    case ePostCharge:       return "PostCharge";    // Nachladung
    case eCloseBatRelais:   return "CloseBRelais";  // Batterierelais schliessen
    case eSymCharge:        return "SymmCharge";    // Symmetrierladung
    case eTrickleCharge:    return "TrickleCharge"; // Erhaltungsladung
    case eTrickleWait:      return "TrickleWait";   // Pause
    case eUnknown17:        return "(?)";           // (möglicherweise Erhaltungsladung einzelner Blöcke?)
    case eUnknown18:        return "(?)";           // (möglicherweise Erhaltungsladung einzelner Blöcke?)
    case eUnknown19:        return "(?)";           // (möglicherweise Erhaltungsladung einzelner Blöcke?)
    case eUnknown20:        return "(?)";           //
    default:                return "Unexpected";
    }
}

QString Decoder::decodeBInfo(quint16 binfo)
{
    QString strBInfo(",");

    if ((1<<REKUPERATION_NOK) & binfo)
    {
        strBInfo += " Rekuperation gesperrt,";  // Rekuperation nicht erlaubt
    }
    if ((1<<CHARGE_NOK) & binfo)
    {
        strBInfo += " Ladung gesprerrt,";  // Ladung nicht erlaubt
    }
    if ((1<<DRIVE_NOK) & binfo)
    {
        strBInfo += " Fahren gesperrt,";  // Fahren nicht erlaubt
    }
    if ((1<<CHARGE_CUR_TO_HI) & binfo)
    {
        strBInfo += " Ladestrom zu hoch,";  // Zu hoher Ladestrom, reduziert Ladestrom
    }
    if ((1<<DRIVE_CUR_TO_HI) & binfo)
    {
        strBInfo += " Fahrstrom zu hoch,";  // Zu hoher Fahrstrom, reduziert Fahrstrom
    }
    if ((1<<VOLTAGE_TO_HI) & binfo)
    {
        strBInfo += " Spannung zu hoch,";  // Zu hohe Ladespannung, aktiviert U-Ladung
    }
    if ((1<<VOLTAGE_TO_LO) & binfo)
    {
        strBInfo += " Spannung zu tief,";  // Zu tiefe Fahrspannung
    }
    if ((1<<BAT_REL_OPEN) & binfo)
    {
        strBInfo += " Batterie Relais offen,";  // Batterie Relais offen
    }
    if ((1<<BAT_FULL) & binfo)
    {
        strBInfo += " Batterie voll,";  // Batterie voll
    }
    if ((1<<BAT_EMPTY) & binfo)
    {
        strBInfo += " Batterie leer,";  // Batterie entladen
    }
    if ((1<<CHARGE_TEMP_TO_HI) & binfo)
    {
        strBInfo += " Ladetemperatur zu hoch,";  // Zu hohe Ladetemperatur
    }
    if ((1<<CHARGE_TEMP_TO_LO) & binfo)
    {
        strBInfo += " Ladetemperatur zu tief,";  // Zu tiefe Ladetemperatur
    }
    if ((1<<DRIVE_TEMP_TO_HI) & binfo)
    {
        strBInfo += " Fahrtemperatur zu hoch,";  // Zu hohe Fahrtemperatur
    }
    if ((1<<DRIVE_TEMP_TO_LO) & binfo)
    {
        strBInfo += " Fahrtemperatur zu tief,";  // Zu tiefe Fahrtemperatur
    }
    if ((1<<VOLTAGE_NOK) & binfo)
    {
        strBInfo += " Unsymetrische Spannung,";  // Unsymmetrische Spannungen
    }
    if ((1<<BAT_ERROR) & binfo)
    {
        strBInfo += " Batterie Fehler,";  // Fehler in der Batterieüberwachung
    }

    strBInfo.chop(1);

    return strBInfo;
}

QString Decoder::decodeUnit(Decoder::EUnit eUnit)
{
    switch(eUnit)
    {
    case Decoder::eVoltage:     return "V";
    case Decoder::eCurrent:     return "A";
    case Decoder::eCharge:      return "Ah";
    case Decoder::eTemperatur:  return "°C";
    case Decoder::ePower:       return "W";
    case Decoder::eSpeed:       return "km/h";
    case Decoder::eDistance:    return "km";
    default:                    return "";
    }
}

Decoder::EUnit Decoder::getUnit(quint8 parameter)
{
    QString description = decodeParameter(parameter);
    Decoder::EUnit eUnit = Decoder::eUnknown;

    if( regSpannung->indexIn(description) != -1)
    {
        eUnit = Decoder::eVoltage;
    }
    else if( regStrom->indexIn(description) != -1)
    {
        eUnit = Decoder::eCurrent;
    }
    else if( regTemperatur->indexIn(description) != -1)
    {
        eUnit = Decoder::eTemperatur;
    }
    else if( regCharge->indexIn(description) != -1)
    {
        eUnit = Decoder::eCharge;
    }
    else if( regPower->indexIn(description) != -1)
    {
        eUnit = Decoder::ePower;
    }
    else if( regSpeed->indexIn(description) != -1)
    {
        eUnit = Decoder::eSpeed;
    }
    else if( regDistance->indexIn(description) != -1)
    {
        eUnit = Decoder::eDistance;
    }
    return eUnit;
}
