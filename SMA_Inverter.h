/* MIT License

Copyright (c) 2022 Lupo135

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/


#define tokWh(value64)    (double)(value64)/1000
#define tokW(value32)     (float)(value32)/1000
#define toHour(value64)   (double)(value64)/3600
#define toAmp(value32)    (float)(value32)/1000
#define toVolt(value32)   (float)(value32)/100
#define toHz(value32)     (float)(value32)/100
#define toPercent(value32)(float)(value32)/100
#define toTemp(value32)   (float)(value32)/100

int32_t  value32 = 0;
int64_t  value64 = 0;
uint64_t totalWh = 0;
uint64_t totalWh_prev = 0;
time_t   dateTime = 0;

#define UG_USER      0x07
#define UG_INSTALLER 0x0A

enum E_RC {
    E_OK =            0,    // No error
    E_INIT =         -1,    // Unable to initialise
    E_INVPASSW =     -2,    // Invalid password
    E_RETRY =        -3,    // Retry the last action
    E_EOF =          -4,    // End of data
    E_NODATA =       -5,    // no data
    E_OVERFLOW =     -6,    // data buffer overflow
    E_BADARG =       -7,    // Invalid data type
    E_CHKSUM =       -8,    // Invalid checksum
    E_INVRESP =      -9,    // Invalid response
    E_ARCHNODATA =   -10,   // no archive data
};

#define ARCH_DAY_SIZE 288
struct InverterData {
    uint8_t BTAddress[6];
    uint8_t SUSyID;
    uint32_t Serial;
    uint8_t NetID;
    int32_t Pmax;
    int32_t Pac;
    int32_t Uac;
    int32_t Iac;
    int32_t Udc;
    int32_t Idc;
    int32_t Freq;
    int32_t Eta;
    uint64_t EToday;
    uint64_t ETotal;
  //DayData dayData[288];
    uint64_t dayWh[ARCH_DAY_SIZE];
  //int32_t dayW[ARCH_DAY_SIZE];
    time_t  DayStartTime;
    bool hasDayData;
    bool hasMonthData;
    time_t   LastTime;
    uint64_t OperationTime;
    uint64_t FeedInTime;
    E_RC     status;
};
InverterData InvData;
InverterData *pInvData = &InvData;

enum getInverterDataType {
    EnergyProduction    = 1 << 0,
    SpotDCPower         = 1 << 1,
    SpotDCVoltage       = 1 << 2,
    SpotACPower         = 1 << 3,
    SpotACVoltage       = 1 << 4,
    SpotGridFrequency   = 1 << 5,
    //MaxACPower        = 1 << 6,
    //MaxACPower2       = 1 << 7,
    SpotACTotalPower    = 1 << 8,
    TypeLabel           = 1 << 9,
    OperationTime       = 1 << 10,
    SoftwareVersion     = 1 << 11,
    DeviceStatus        = 1 << 12,
    GridRelayStatus     = 1 << 13,
    BatteryChargeStatus = 1 << 14,
    BatteryInfo         = 1 << 15,
    InverterTemp        = 1 << 16,
    MeteringGridMsTotW  = 1 << 17,
    sbftest             = 1 << 31
};

enum LriDef {
    OperationHealth                 = 0x2148,   // *08* Condition (aka INV_STATUS)
    CoolsysTmpNom                   = 0x2377,   // *40* Operating condition temperatures
    DcMsWatt                        = 0x251E,   // *40* DC power input (aka SPOT_PDC1 / SPOT_PDC2)
    MeteringTotWhOut                = 0x2601,   // *00* Total yield (aka SPOT_ETOTAL)
    MeteringDyWhOut                 = 0x2622,   // *00* Day yield (aka SPOT_ETODAY)
    GridMsTotW                      = 0x263F,   // *40* Power (aka SPOT_PACTOT)
    BatChaStt                       = 0x295A,   // *00* Current battery charge status
    OperationHealthSttOk            = 0x411E,   // *00* Nominal power in Ok Mode (deprecated INV_PACMAX1)
    OperationHealthSttWrn           = 0x411F,   // *00* Nominal power in Warning Mode (deprecated INV_PACMAX2)
    OperationHealthSttAlm           = 0x4120,   // *00* Nominal power in Fault Mode (deprecated INV_PACMAX3)
    OperationGriSwStt               = 0x4164,   // *08* Grid relay/contactor (aka INV_GRIDRELAY)
    OperationRmgTms                 = 0x4166,   // *00* Waiting time until feed-in
    DcMsVol                         = 0x451F,   // *40* DC voltage input (aka SPOT_UDC1 / SPOT_UDC2)
    DcMsAmp                         = 0x4521,   // *40* DC current input (aka SPOT_IDC1 / SPOT_IDC2)
    MeteringPvMsTotWhOut            = 0x4623,   // *00* PV generation counter reading
    MeteringGridMsTotWhOut          = 0x4624,   // *00* Grid feed-in counter reading
    MeteringGridMsTotWhIn           = 0x4625,   // *00* Grid reference counter reading
    MeteringCsmpTotWhIn             = 0x4626,   // *00* Meter reading consumption meter
    MeteringGridMsDyWhOut           = 0x4627,   // *00* ?
    MeteringGridMsDyWhIn            = 0x4628,   // *00* ?
    MeteringTotOpTms                = 0x462E,   // *00* Operating time (aka SPOT_OPERTM)
    MeteringTotFeedTms              = 0x462F,   // *00* Feed-in time (aka SPOT_FEEDTM)
    MeteringGriFailTms              = 0x4631,   // *00* Power outage
    MeteringWhIn                    = 0x463A,   // *00* Absorbed energy
    MeteringWhOut                   = 0x463B,   // *00* Released energy
    MeteringPvMsTotWOut             = 0x4635,   // *40* PV power generated
    MeteringGridMsTotWOut           = 0x4636,   // *40* Power grid feed-in
    MeteringGridMsTotWIn            = 0x4637,   // *40* Power grid reference
    MeteringCsmpTotWIn              = 0x4639,   // *40* Consumer power
    GridMsWphsA                     = 0x4640,   // *40* Power L1 (aka SPOT_PAC1)
    GridMsWphsB                     = 0x4641,   // *40* Power L2 (aka SPOT_PAC2)
    GridMsWphsC                     = 0x4642,   // *40* Power L3 (aka SPOT_PAC3)
    GridMsPhVphsA                   = 0x4648,   // *00* Grid voltage phase L1 (aka SPOT_UAC1)
    GridMsPhVphsB                   = 0x4649,   // *00* Grid voltage phase L2 (aka SPOT_UAC2)
    GridMsPhVphsC                   = 0x464A,   // *00* Grid voltage phase L3 (aka SPOT_UAC3)
    GridMsAphsA_1                   = 0x4650,   // *00* Grid current phase L1 (aka SPOT_IAC1)
    GridMsAphsB_1                   = 0x4651,   // *00* Grid current phase L2 (aka SPOT_IAC2)
    GridMsAphsC_1                   = 0x4652,   // *00* Grid current phase L3 (aka SPOT_IAC3)
    GridMsAphsA                     = 0x4653,   // *00* Grid current phase L1 (aka SPOT_IAC1_2)
    GridMsAphsB                     = 0x4654,   // *00* Grid current phase L2 (aka SPOT_IAC2_2)
    GridMsAphsC                     = 0x4655,   // *00* Grid current phase L3 (aka SPOT_IAC3_2)
    GridMsHz                        = 0x4657,   // *00* Grid frequency (aka SPOT_FREQ)
    MeteringSelfCsmpSelfCsmpWh      = 0x46AA,   // *00* Energy consumed internally
    MeteringSelfCsmpActlSelfCsmp    = 0x46AB,   // *00* Current self-consumption
    MeteringSelfCsmpSelfCsmpInc     = 0x46AC,   // *00* Current rise in self-consumption
    MeteringSelfCsmpAbsSelfCsmpInc  = 0x46AD,   // *00* Rise in self-consumption
    MeteringSelfCsmpDySelfCsmpInc   = 0x46AE,   // *00* Rise in self-consumption today
    BatDiagCapacThrpCnt             = 0x491E,   // *40* Number of battery charge throughputs
    BatDiagTotAhIn                  = 0x4926,   // *00* Amp hours counter for battery charge
    BatDiagTotAhOut                 = 0x4927,   // *00* Amp hours counter for battery discharge
    BatTmpVal                       = 0x495B,   // *40* Battery temperature
    BatVol                          = 0x495C,   // *40* Battery voltage
    BatAmp                          = 0x495D,   // *40* Battery current
    NameplateLocation               = 0x821E,   // *10* Device name (aka INV_NAME)
    NameplateMainModel              = 0x821F,   // *08* Device class (aka INV_CLASS)
    NameplateModel                  = 0x8220,   // *08* Device type (aka INV_TYPE)
    NameplateAvalGrpUsr             = 0x8221,   // *  * Unknown
    NameplatePkgRev                 = 0x8234,   // *08* Software package (aka INV_SWVER)
    InverterWLim                    = 0x832A,   // *00* Maximum active power (deprecated INV_PACMAX1_2) (SB3300/SB1200)
    GridMsPhVphsA2B6100             = 0x464B,
    GridMsPhVphsB2C6100             = 0x464C,
    GridMsPhVphsC2A6100             = 0x464D
};

// MultiPacket:
//  Packet-first: Cmd0=08 Byte[18]=0x7E
//  Packet-last : Cmd1=01=cmdcodetowait 
#pragma pack (push, 1)
typedef struct __attribute__ ((packed)) PacketHeader {
    uint8_t   SOP;                // Start Of Packet (0x7E)
    uint16_t  pkLength;
    uint8_t   pkChecksum;
    uint8_t   SourceAddr[6];      // SMA Inverter Address
    uint8_t   DestinationAddr[6]; // Local BT Address
    uint16_t  command;
} L1Hdr;

bool isValidSender(uint8_t expAddr[6], uint8_t isAddr[6]) {
  for (int i = 0; i < 6; i++)
    if ((isAddr[i] != expAddr[i]) && (expAddr[i] != 0xFF)) {
      DEBUG2_PRINTF("\nShoud-Addr: %02X %02X %02X %02X %02X %02X\n   Is-Addr: %02X %02X %02X %02X %02X %02X\n",
        expAddr[5], expAddr[4], expAddr[3], expAddr[2], expAddr[1], expAddr[5],
         isAddr[5],  isAddr[4],  isAddr[3],  isAddr[2],  isAddr[1],  isAddr[5]);
        return false;
    }
  return true;
}


// ----------------------------------------------------------------------------------------------
//unsigned int readBtPacket(int index, unsigned int cmdcodetowait) {
E_RC getPacket(uint8_t expAddr[6], int wait4Command) {
  DEBUG3_PRINTF("getPacket cmd=0x%04x\n", wait4Command);
  int index = 0;
  bool hasL2pckt = false;
  E_RC rc = E_OK; 
  L1Hdr *pL1Hdr = (L1Hdr *)&BTrdBuf[0];
  do {
    // read L1Hdr
    uint8_t rdCnt=0;
    for (rdCnt=0;rdCnt<18;rdCnt++) {
      BTrdBuf[rdCnt]= BTgetByte();
      if (ReadTimeout)  break;
    }
    DEBUG2_PRINTF("\nL1 Rec=%d bytes pkL=0x%04x=%d Cmd=0x%04x\n",
        rdCnt, pL1Hdr->pkLength, pL1Hdr->pkLength, pL1Hdr->command);

    if (rdCnt<17) {
      DEBUG3_PRINTF("L1<18=%d bytes", rdCnt);
      #if (DEBUG_SMA > 2)
      HexDump(BTrdBuf, rdCnt, 10, 'R');
      #endif
      return E_NODATA;
    }
    // Validate L1 header
    if (!((BTrdBuf[0] ^ BTrdBuf[1] ^ BTrdBuf[2]) == BTrdBuf[3])) {
      DEBUG1_PRINT("\nWrong L1 CRC!!" );
    }

    if (pL1Hdr->pkLength > sizeof(L1Hdr)) { // more bytes to read
      for (rdCnt=18; rdCnt<pL1Hdr->pkLength; rdCnt++) {
        BTrdBuf[rdCnt]= BTgetByte();
        if (ReadTimeout) break;
      }
      DEBUG3_PRINTF("L2 Rec=%d bytes", rdCnt-18);
      #if (DEBUG_SMA > 2)
      HexDump(BTrdBuf, rdCnt, 10, 'R');
      #endif

      //Check if data is coming from the right inverter
      if (isValidSender(expAddr, pL1Hdr->SourceAddr)) {
        rc = E_OK;

        DEBUG2_PRINTF("HasL2pckt: 0x7E?=0x%02X 0x656003FF?=0x%08X\n", BTrdBuf[18], get_u32(BTrdBuf+19));
        if ((hasL2pckt == 0) && (BTrdBuf[18] == 0x7E) && (get_u32(BTrdBuf+19) == 0x656003FF)) {
          hasL2pckt = true;
        }

        if (hasL2pckt) {
          //Copy BTrdBuf to pcktBuf
          bool escNext = false;

          for (int i=sizeof(L1Hdr); i<pL1Hdr->pkLength; i++) {
            pcktBuf[index] = BTrdBuf[i];
            //Keep 1st byte raw unescaped 0x7E
            if (escNext == true) {
              pcktBuf[index] ^= 0x20;
              escNext = false;
              index++;
            } else {
              if (pcktBuf[index] == 0x7D)
                escNext = true; //Throw away the 0x7d byte
              else
                index++;
            }
            if (index >= maxpcktBufsize) {
              DEBUG1_PRINTF("pcktBuf overflow! (%d)\n", index);
            }
          }
          pcktBufPos = index;
        } else {  // no L2pckt
          memcpy(pcktBuf, BTrdBuf, rdCnt);
          pcktBufPos = rdCnt;
        }
      } else { // isValidSender()
          rc = E_RETRY;
      }
    } else {  // L1 only
    #if (DEBUG_SMA > 2)
      HexDump(BTrdBuf, rdCnt, 10, 'R');
    #endif
      //Check if data is coming from the right inverter
      if (isValidSender(expAddr, pL1Hdr->SourceAddr)) {
          rc = E_OK;

          memcpy(pcktBuf, BTrdBuf, rdCnt);
          pcktBufPos = rdCnt;
      } else { // isValidSender()
          rc = E_RETRY;
      }
    }
    if (BTrdBuf[0] != '\x7e') { 
       SerialBT.flush();
       DEBUG2_PRINT("\nCommBuf[0]!=0x7e -> BT-flush");
    }
  } while (((pL1Hdr->command != wait4Command) || (rc == E_RETRY)) && (0xFF != wait4Command));

  if ((rc == E_OK) ) {
  #if (DEBUG_SMA > 1)
    DEBUG2_PRINT("<<<====Rd Content of pcktBuf =======>>>");
    HexDump(pcktBuf, pcktBufPos, 10, 'P');
    DEBUG2_PRINT("==>>>");
  #endif
  }

  if (pcktBufPos > pcktBufMax) {
    pcktBufMax = pcktBufPos;
    DEBUG2_PRINTF("pcktBufMax is now %d bytes\n", pcktBufMax);
  }

  return rc;
}

// *************************************************
void writePacketHeader(uint8_t *buf, const uint16_t control, const uint8_t *destaddress) {
    pcktBufPos = 0;

    FCSChecksum = 0xFFFF;
    buf[pcktBufPos++] = 0x7E;
    buf[pcktBufPos++] = 0;  //placeholder for len1
    buf[pcktBufPos++] = 0;  //placeholder for len2
    buf[pcktBufPos++] = 0;  //placeholder for checksum
    int i;
    for(i = 0; i < 6; i++) buf[pcktBufPos++] = EspBTAddress[i];
    for(i = 0; i < 6; i++) buf[pcktBufPos++] = destaddress[i];

    buf[pcktBufPos++] = (uint8_t)(control & 0xFF);
    buf[pcktBufPos++] = (uint8_t)(control >> 8);
}
// ***********************************************
E_RC getInverterDataCfl(uint32_t command, uint32_t first, uint32_t last) {
    pcktID++;
    writePacketHeader(pcktBuf, 0x01, sixff); //addr_unknown);
    //if (pInvData->SUSyID == SID_SB240)
    //writePacket(pcktBuf, 0x09, 0xE0, 0, pInvData->SUSyID, pInvData->Serial);
    //else
    writePacket(pcktBuf, 0x09, 0xA0, 0, pInvData->SUSyID, pInvData->Serial);
    write32(pcktBuf, command);
    write32(pcktBuf, first);
    write32(pcktBuf, last);
    writePacketTrailer(pcktBuf);
    writePacketLength(pcktBuf);

    BTsendPacket(pcktBuf);

    uint8_t pcktcount = 0;
    bool  validPcktID = false;
    do {
    do {
      pInvData->status = getPacket(pInvData->BTAddress, 0x0001);
   
      if (pInvData->status != E_OK) return pInvData->status;
      if (validateChecksum()) {
        if ((pInvData->status = (E_RC)get_u16(pcktBuf + 23)) != E_OK) {
          DEBUG2_PRINTF("Packet status: 0x%02X\n", pInvData->status);
          return pInvData->status;
        }
        // *** analyze received data ***
        pcktcount = get_u16(pcktBuf + 25);
        uint16_t rcvpcktID = get_u16(pcktBuf + 27) & 0x7FFF;
        if (pcktID == rcvpcktID) {
          if ((get_u16(pcktBuf + 15) == pInvData->SUSyID) 
            && (get_u32(pcktBuf + 17) == pInvData->Serial)) {
            validPcktID = true;
            value32 = 0;
            value64 = 0;
            uint16_t recordsize = 4 * ((uint32_t)pcktBuf[5] - 9) / (get_u32(pcktBuf + 37) - get_u32(pcktBuf + 33) + 1);
            DEBUG2_PRINTF("\npcktID=0x%04x recsize=%d BufPos=%d pcktCnt=%04x", 
                            rcvpcktID,   recordsize, pcktBufPos, pcktcount);
            for (uint16_t ii = 41; ii < pcktBufPos - 3; ii += recordsize) {
              uint8_t *recptr = pcktBuf + ii;
              uint32_t code = get_u32(recptr);
              //LriDef lri = (LriDef)(code & 0x00FFFF00);
              uint16_t lri = (code & 0x00FFFF00) >> 8;
              uint32_t cls = code & 0xFF;
              uint8_t dataType = code >> 24;
              time_t datetime = (time_t)get_u32(recptr + 4);
              DEBUG3_PRINTF("\nlri=0x%04x cls=0x%08X dataType=0x%02x",lri, cls, dataType);
       
              if (recordsize == 16) {
                value64 = get_u64(recptr + 8);
                DEBUG3_PRINTF("\nvalue64=%d=0x%08x",value64, value64);
       
                  //if (is_NaN(value64) || is_NaN((uint64_t)value64)) value64 = 0;
              } else if ((dataType != 16) && (dataType != 8)) { // ((dataType != DT_STRING) && (dataType != DT_STATUS)) {
                value32 = get_u32(recptr + 16);
                DEBUG3_PRINTF("\nvalue32=%d=0x%08x",value32, value32);
              }
              switch (lri) {
              case GridMsTotW: //SPOT_PACTOT
                  //This function gives us the time when the inverter was switched off
                  pInvData->LastTime = datetime;
                  pInvData->Pac = value32;
                  //debug_watt("SPOT_PACTOT", value32, datetime);
                  printUnixTime(timeBuf, datetime);
                  DEBUG1_PRINTF("\nPac %15.3f kW GMT:%s", tokW(value32), timeBuf);
                  break;
       
              case GridMsWphsA: //SPOT_PAC1
                  pInvData->Pmax = value32;
                  //debug_watt("SPOT_PAC1", value32, datetime);
                  DEBUG1_PRINTF("\nPmax %14.2f kW ", tokW(value32));
                  //printUnixTime(timeBuf, datetime);
                  break;
       
              case GridMsPhVphsA: //SPOT_UAC1
                  pInvData->Uac = value32;
                  //debug_volt("SPOT_UAC1", value32, datetime);
                  DEBUG1_PRINTF("\nUac %15.2f V  ", toVolt(value32));
                  //printUnixTime(timeBuf, datetime);
                  break;
       
              case GridMsAphsA_1: //SPOT_IAC1
              case GridMsAphsA:
                  pInvData->Iac = value32;
                  //debug_amp("SPOT_IAC1", value32, datetime);
                  DEBUG1_PRINTF("\nIac %15.2f A  ", toAmp(value32));
                  //printUnixTime(timeBuf, datetime);
                  break;
       
              case GridMsHz: //SPOT_FREQ
                  pInvData->Freq = value32;
                  DEBUG1_PRINTF("\nFreq %14.2f Hz ", toHz(value32));
                  //printUnixTime(timeBuf, datetime);
                  break;
       
              case DcMsWatt: //SPOT_PDC1 / SPOT_PDC2
                  DEBUG1_PRINTF("\nPDC %15.2f kW ", tokW(value32));
                  //printUnixTime(timeBuf, datetime);
                  break;
       
              case DcMsVol: //SPOT_UDC1 / SPOT_UDC2
                  pInvData->Udc = value32;
                  DEBUG1_PRINTF("\nUdc %15.2f V  ", toVolt(value32));
                  //printUnixTime(timeBuf, datetime);
                  break;
       
              case DcMsAmp: //SPOT_IDC1 / SPOT_IDC2
                  pInvData->Idc = value32;
                  DEBUG1_PRINTF("\nIdc %15.2f A  ", toAmp(value32));
                  //printUnixTime(timeBuf, datetime);
                  if ((pInvData->Udc!=0) && (pInvData->Idc != 0))
                    pInvData->Eta = ((uint64_t)pInvData->Uac * (uint64_t)pInvData->Iac * 10000) /
                                    ((uint64_t)pInvData->Udc * (uint64_t)pInvData->Idc );
                  else pInvData->Eta = 0;
                  DEBUG1_PRINTF("\nEfficiency %8.2f %%", toPercent(pInvData->Eta));
                  break;
       
              case MeteringDyWhOut: //SPOT_ETODAY
                  //This function gives us the current inverter time
                  //pInvData->InverterDatetime = datetime;
                  pInvData->EToday = value64;
                  //debug_kwh("SPOT_ETODAY", value64, datetime);
                  DEBUG1_PRINTF("\nE-Today %11.3f kWh", tokWh(value64));
                  //printUnixTime(timeBuf, datetime);
                  break;
       
              case MeteringTotWhOut: //SPOT_ETOTAL
                  //In case SPOT_ETODAY missing, this function gives us inverter time (eg: SUNNY TRIPOWER 6.0)
                  //pInvData->InverterDatetime = datetime;
                  pInvData->ETotal = value64;
                  //debug_kwh("SPOT_ETOTAL", value64, datetime);
                  DEBUG1_PRINTF("\nE-Total %11.3f kWh", tokWh(value64));
                  //printUnixTime(timeBuf, datetime);
                  break;
       
              case MeteringTotOpTms: //SPOT_OPERTM
                  pInvData->OperationTime = value64;
                  //debug_hour("SPOT_OPERTM", value64, datetime);
                  DEBUG1_PRINTF("\nOperTime  %7.3f h  ", toHour(value64));
                  //printUnixTime(timeBuf, datetime);
                  break;
       
              case MeteringTotFeedTms: //SPOT_FEEDTM
                  pInvData->FeedInTime = value64;
                  //debug_hour("SPOT_FEEDTM", value64, datetime);
                  DEBUG1_PRINTF("\nFeedTime  %7.3f h  ", toHour(value64));
                  //printUnixTime(timeBuf, datetime);
                  break;
       
              case CoolsysTmpNom:
                  //pInvData->Temperature = value32;
                  DEBUG1_PRINTF("\nTemp.     %7.3f �C ", toTemp(value32));
                  break;
       
              case MeteringGridMsTotWOut:
                  //pInvData->MeteringGridMsTotWOut = value32;
                  break;
       
              case MeteringGridMsTotWIn:
                  //pInvData->MeteringGridMsTotWIn = value32;
                  break;
              }
            } //for
          } else {
            DEBUG3_PRINTF("*** Wrong SUSyID=%04x=%04x Serial=%08x=%08x", 
                 get_u16(pcktBuf + 15), pInvData->SUSyID, get_u32(pcktBuf + 17),pInvData->Serial);
          }
        } else {  // wrong PacketID
          DEBUG3_PRINTF("PacketID mismatch: exp=0x%04X is=0x%04X\n", pcktID, rcvpcktID);
          validPcktID = false;
          pcktcount = 0;
        }
      } else { // invalid Checksum
        pInvData->status = E_CHKSUM;
        return pInvData->status;
      }
   } while (pcktcount > 0);
   } while (!validPcktID);
   return pInvData->status;
}
// ***********************************************
E_RC getInverterData(enum getInverterDataType type) {
  E_RC rc = E_OK;
  uint32_t command;
  uint32_t first;
  uint32_t last;

  switch (type) {
  case EnergyProduction:
      DEBUG2_PRINT("\n*** EnergyProduction ***");
      // SPOT_ETODAY, SPOT_ETOTAL
      command = 0x54000200;
      first = 0x00260100;
      last = 0x002622FF;
      break;

  case SpotDCPower:
      DEBUG2_PRINT("\n*** SpotDCPower ***");
      // SPOT_PDC1, SPOT_PDC2
      command = 0x53800200;
      first = 0x00251E00;
      last = 0x00251EFF;
      break;

  case SpotDCVoltage:
      DEBUG2_PRINT("\n*** SpotDCVoltage ***");
      // SPOT_UDC1, SPOT_UDC2, SPOT_IDC1, SPOT_IDC2
      command = 0x53800200;
      first = 0x00451F00;
      last = 0x004521FF;
      break;

  case SpotACPower:
      DEBUG2_PRINT("\n*** SpotACPower ***");
      // SPOT_PAC1, SPOT_PAC2, SPOT_PAC3
      command = 0x51000200;
      first = 0x00464000;
      last = 0x004642FF;
      break;

  case SpotACVoltage:
      DEBUG2_PRINT("\n*** SpotACVoltage ***");
      // SPOT_UAC1, SPOT_UAC2, SPOT_UAC3, SPOT_IAC1, SPOT_IAC2, SPOT_IAC3
      command = 0x51000200;
      first = 0x00464800;
      last = 0x004655FF;
      break;

  case SpotGridFrequency:
      DEBUG2_PRINT("\n*** SpotGridFrequency ***");
      // SPOT_FREQ
      command = 0x51000200;
      first = 0x00465700;
      last = 0x004657FF;
      break;

  case SpotACTotalPower:
      DEBUG2_PRINT("\n*** SpotACTotalPower ***");
      // SPOT_PACTOT
      command = 0x51000200;
      first = 0x00263F00;
      last = 0x00263FFF;
      break;

  case TypeLabel:
      DEBUG2_PRINT("\n*** TypeLabel ***");
      // INV_NAME, INV_TYPE, INV_CLASS
      command = 0x58000200;
      first = 0x00821E00;
      last = 0x008220FF;
      break;

  case SoftwareVersion:
      DEBUG2_PRINT("\n*** SoftwareVersion ***");
      // INV_SWVERSION
      command = 0x58000200;
      first = 0x00823400;
      last = 0x008234FF;
      break;

  case DeviceStatus:
      DEBUG2_PRINT("\n*** DeviceStatus ***");
      // INV_STATUS
      command = 0x51800200;
      first = 0x00214800;
      last = 0x002148FF;
      break;

  case GridRelayStatus:
      DEBUG2_PRINT("\n*** GridRelayStatus ***");
      // INV_GRIDRELAY
      command = 0x51800200;
      first = 0x00416400;
      last = 0x004164FF;
      break;

  case OperationTime:
      DEBUG2_PRINT("\n*** OperationTime ***");
      // SPOT_OPERTM, SPOT_FEEDTM
      command = 0x54000200;
      first = 0x00462E00;
      last = 0x00462FFF;
      break;

  case InverterTemp:
      DEBUG2_PRINT("\n*** InverterTemp ***");
      command = 0x52000200;
      first = 0x00237700;
      last = 0x002377FF;
      break;

  case MeteringGridMsTotW:
      DEBUG2_PRINT("\n*** MeteringGridMsTotW ***");
      command = 0x51000200;
      first = 0x00463600;
      last = 0x004637FF;
      break;

  default:
      DEBUG1_PRINT("\nInvalid getInverterDataType!!");
      return E_BADARG;
  };

  // Request data from inverter
  for (uint8_t retries=1;; retries++) {
    rc = getInverterDataCfl(command, first, last);
    if (rc != E_OK) {
      if (retries>1) {
         return rc;
      }
      DEBUG2_PRINTF("\nRetrying.%d",retries);
    } else {
      break;
    } 
  }

  return rc;
}

//-------------------------------------------------------------------------
bool getBT_SignalStrength() {
  DEBUG2_PRINT("\n\n*** SignalStrength ***");
  writePacketHeader(pcktBuf, 0x03, pInvData->BTAddress);
  writeByte(pcktBuf,0x05);
  writeByte(pcktBuf,0x00);
  writePacketLength(pcktBuf);
  BTsendPacket(pcktBuf);

  getPacket(pInvData->BTAddress, 4);
  DEBUG1_PRINTF("BT-Signal %9.1f %%", ((float)BTrdBuf[22] * 100.0f / 255.0f));
  return true;
}
//-------------------------------------------------------------------------
E_RC initialiseSMAConnection() {
  DEBUG2_PRINTLN(" -> Initialize");
  getPacket(pInvData->BTAddress, 2); // 1. Receive
  pInvData->NetID = pcktBuf[22];
  DEBUG2_PRINTF("SMA netID=%02X\n", pInvData->NetID);
  writePacketHeader(pcktBuf, 0x02, pInvData->BTAddress);
  write32(pcktBuf, 0x00700400);
  writeByte(pcktBuf, pInvData->NetID);
  write32(pcktBuf, 0);
  write32(pcktBuf, 1);
  writePacketLength(pcktBuf);

  BTsendPacket(pcktBuf);             // 1. Reply
  getPacket(pInvData->BTAddress, 5); // 2. Receive

  // Extract ESP32 BT address
  memcpy(EspBTAddress, pcktBuf+26,6); 
  DEBUG3_PRINTF("ESP32 BT address: %02X:%02X:%02X:%02X:%02X:%02X\n",
             EspBTAddress[5], EspBTAddress[4], EspBTAddress[3],
             EspBTAddress[2], EspBTAddress[1], EspBTAddress[0]);

  pcktID++;
  writePacketHeader(pcktBuf, 0x01, sixff); //addr_unknown);
  writePacket(pcktBuf, 0x09, 0xA0, 0, 0xFFFF, 0xFFFFFFFF); // anySUSyID, anySerial);
  write32(pcktBuf, 0x00000200);
  write32(pcktBuf, 0);
  write32(pcktBuf, 0);
  writePacketTrailer(pcktBuf);
  writePacketLength(pcktBuf);

  BTsendPacket(pcktBuf);             // 2. Reply
  if (getPacket(pInvData->BTAddress, 1) != E_OK) // 3. Receive
    return E_INIT;

  if (!validateChecksum())
    return E_CHKSUM;

  pInvData->Serial = get_u32(pcktBuf + 57);
  DEBUG1_PRINTF("Serial Nr: %lu\n", pInvData->Serial);
  return E_OK;
}
// **** Logon SMA **********
E_RC logonSMAInverter(const char *password, const uint8_t user) {
  #define MAX_PWLENGTH 12
  uint8_t pw[MAX_PWLENGTH];
  E_RC rc = E_OK;

  // Encode password
  uint8_t encChar = (user == USERGROUP)? 0x88:0xBB;
  uint8_t idx;
  for (idx = 0; (password[idx] != 0) && (idx < sizeof(pw)); idx++)
    pw[idx] = password[idx] + encChar;
  for (; idx < MAX_PWLENGTH; idx++) pw[idx] = encChar;

    bool validPcktID = false;
    time_t now;
    pcktID++;
    now = time(NULL);
    writePacketHeader(pcktBuf, 0x01, sixff);
    writePacket(pcktBuf, 0x0E, 0xA0, 0x0100, 0xFFFF, 0xFFFFFFFF); // anySUSyID, anySerial);
    write32(pcktBuf, 0xFFFD040C);
    write32(pcktBuf, user); //userGroup);    // User / Installer
    write32(pcktBuf, 0x00000384); // Timeout = 900sec ?
    write32(pcktBuf, now);
    write32(pcktBuf, 0);
    writeArray(pcktBuf, pw, sizeof(pw));
    writePacketTrailer(pcktBuf);
    writePacketLength(pcktBuf);

    BTsendPacket(pcktBuf);
    if ((rc = getPacket(sixff, 1)) != E_OK) return rc;

    if (!validateChecksum()) return E_CHKSUM;
    uint8_t rcvpcktID = get_u16(pcktBuf+27) & 0x7FFF;
    if ((pcktID == rcvpcktID) && (get_u32(pcktBuf + 41) == now)) {
      pInvData->SUSyID = get_u16(pcktBuf + 15);
      pInvData->Serial = get_u32(pcktBuf + 17);
      DEBUG3_PRINTF("Set:->SUSyID=0x%02X ->Serial=0x%02X ", pInvData->SUSyID, pInvData->Serial);
      validPcktID = true;
      uint8_t retcode = get_u16(pcktBuf + 23);
      // switch (retcode) {
      //     case 0: rc = E_OK; break;
      //     case 0x0100: rc = E_INVPASSW; break;
      //     default: rc = (E_RC)retcode; break;
      // }
    } else { 
      DEBUG1_PRINTF("Unexpected response  %02X:%02X:%02X:%02X:%02X:%02X pcktID=0x%04X rcvpcktID=0x%04X now=0x%04X", 
                   BTrdBuf[9], BTrdBuf[8], BTrdBuf[7], BTrdBuf[6], BTrdBuf[5], BTrdBuf[4],
                   pcktID, rcvpcktID, now);
      rc = E_INVRESP;
    }
    return rc;
}

// ******* Archive Day Data **********
E_RC ArchiveDayData(time_t startTime) {
  DEBUG2_PRINT("\n*** ArchiveDayData ***");
  printUnixTime(timeBuf, startTime); DEBUG2_PRINTF("\nStartTime0 GMT:%s", timeBuf);
  // set time to begin of day
  uint8_t minutes = (startTime/60) % 60;
  uint8_t hours = (startTime/(60*60)) % 24;
  startTime -= minutes*60 + hours*60*60;
  printUnixTime(timeBuf, startTime); DEBUG2_PRINTF("\nStartTime2 GMT:%s", timeBuf);

  E_RC rc = E_OK;

  for (unsigned int i = 0; i<ARCH_DAY_SIZE; i++) {
     pInvData->dayWh[i] = 0;
  }
  pInvData->hasDayData = false;

  int packetcount = 0;
  bool validPcktID = false;

  E_RC hasData = E_ARCHNODATA;
  pcktID++;
  writePacketHeader(pcktBuf, 0x01, pInvData->BTAddress);
  writePacket(pcktBuf, 0x09, 0xE0, 0, pInvData->SUSyID, pInvData->Serial);
  write32(pcktBuf, 0x70000200);
  write32(pcktBuf, startTime - 300);
  write32(pcktBuf, startTime + 86100);
  writePacketTrailer(pcktBuf);
  writePacketLength(pcktBuf);

  BTsendPacket(pcktBuf);

  do {
    totalWh = 0;
    totalWh_prev = 0;
    dateTime = 0;

    do {
      rc = getPacket(pInvData->BTAddress, 1);

      if (rc != E_OK) {
         DEBUG3_PRINTF("\ngetPacket error=%d", rc);
         return rc;
      }
      // packetcount=nr of packets left on multi packet transfer n..0
      packetcount = pcktBuf[25];
      DEBUG2_PRINTF("packetcount=%d\n", packetcount);

      //TODO: Move checksum validation to getPacket
      if (!validateChecksum())
        return E_CHKSUM;
      else {
        unsigned short rcvpcktID = get_u16(pcktBuf + 27) & 0x7FFF;
        if (validPcktID || (pcktID == rcvpcktID)) {
          validPcktID = true;
          for (int x = 41; x < (pcktBufPos - 3); x += 12) {
            dateTime = (time_t)get_u32(pcktBuf + x);
            uint16_t idx =((dateTime/3600)%24 * 12)+((dateTime/60)%60/5); //h*12+min/5

            totalWh = get_u64(pcktBuf + x + 4);
            if ((totalWh > 0) && (!pInvData->hasDayData)) { 
              pInvData->DayStartTime = dateTime;
              pInvData->hasDayData = true;
              hasData = E_OK; 
              printUnixTime(timeBuf, dateTime); 
              DEBUG1_PRINTF("\nArchiveDayData %s", timeBuf);
            }
            if (idx < ARCH_DAY_SIZE) {
              pInvData->dayWh[idx] = totalWh;
              value64 = (totalWh - totalWh_prev) * 60 / 5; // assume 5 min. interval
              DEBUG3_PRINTF("[%03u] %6llu Wh %6llu W\n", idx, totalWh, value64);
            }
            totalWh_prev = totalWh;
          } //for
        } else {
            DEBUG1_PRINTF("Packet ID mismatch. Exp. %d, rec. %d\n", pcktID, rcvpcktID);
            validPcktID = true;
            packetcount = 0;
        }
      }
    } while (packetcount > 0);
  } while (!validPcktID);

  /* print values
  time_t startT = pInvData->DayStartTime;
  printUnixTime(timeBuf, startT);
  DEBUG2_PRINTF("Day History: %s\n", timeBuf);
  totalWh_prev = 0;

  for (uint16_t i = 0; i<ARCH_DAY_SIZE; i++) {
    totalWh = pInvData->dayWh[i];
    value32=0;
    if ((totalWh>0) && (totalWh_prev>0)) {
      value32 = (uint32_t)((totalWh - totalWh_prev)*60/5); 
    }
    if (totalWh>0) {
      printUnixTime(timeBuf, startT+3600+i*60*5); // GMT+1 + 5 min. interval
      DEBUG2_PRINTF("[%03d] %11.3f kWh  %7.3f kW %s\n", i, tokWh(totalWh), tokW(value32), timeBuf);
    }
    totalWh_prev = totalWh;
  }
  */
  return hasData;
}

// ******* read SMA current data **********
E_RC ReadCurrentData() {
  if (!btConnected) {
    charLen += snprintf(charBuf+charLen, CHAR_BUF_MAX-charLen, "Bluetooth offline!\n");
    return E_NODATA;
  }
  if ((getInverterData(SpotACTotalPower)) != E_OK)  {
    charLen += snprintf(charBuf+charLen, CHAR_BUF_MAX-charLen, "SpotACTotalPower error!\n" ); // Pac
    return E_NODATA;
  }
  if ((getInverterData(SpotDCVoltage)) != E_OK)     {
    charLen += snprintf(charBuf+charLen, CHAR_BUF_MAX-charLen, "getSpotDCVoltage error!\n" ); // Udc + Idc
    return E_NODATA;
  }
  if ((getInverterData(SpotACVoltage)) != E_OK)     {
    charLen += snprintf(charBuf+charLen, CHAR_BUF_MAX-charLen, "getSpotACVoltage error!\n" ); // Uac + Iac
    return E_NODATA;
  }
  if ((getInverterData(EnergyProduction)) != E_OK)  {
    charLen += snprintf(charBuf+charLen, CHAR_BUF_MAX-charLen, "EnergyProduction error!\n" ); // E-Total + E-Today
    return E_NODATA;
  }
  if ((getInverterData(SpotGridFrequency)) != E_OK) {
    charLen += snprintf(charBuf+charLen, CHAR_BUF_MAX-charLen, "SpotGridFrequency error!\n");
    return E_NODATA;
  }

//case 5: if ((getInverterData(SpotDCPower)) != E_OK)   DEBUG1_PRINTLN("getSpotDCPower error!"); //pcktBuf[23]=15 error!
//case 6: if ((getInverterData(SpotACPower)) != E_OK)   DEBUG1_PRINTLN("SpotACPower error!"   ); //pcktBuf[23]=15 error!
//case 7: if ((getInverterData(InverterTemp)) != E_OK)  DEBUG1_PRINTLN("InverterTemp error!"  ); //pcktBuf[23]=15 error!
//case 8: if ((getInverterData(OperationTime)) != E_OK) DEBUG1_PRINTLN("OperationTime error!" ); // OperTime + OperTime
  return E_OK;
} 

