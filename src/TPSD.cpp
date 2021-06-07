#include <iostream>

#include "TPSD.hpp"

TPSD::TPSD() {}

TPSD::~TPSD() { FreeMemory(); }

void TPSD::Config()
{
  TrgConf();
  AcqConf();
  DPPConf();
  PSDConfig();
  RegisterSettings();

  BufferConf();
}

void TPSD::PSDConfig()
{
  CAEN_DGTZ_DPP_PSD_Params_t param;
  for (auto iCh = 0; iCh < fNChs; iCh++) {
    param.thr[iCh] = fDigiPar.TrgTh[iCh];
    param.selft[iCh] = (fDigiPar.ChMask >> iCh) & 0b1;
    param.csens[iCh] = fDigiPar.ChargeSens[iCh];
    param.sgate[iCh] = fDigiPar.ShortGate[iCh] / fTimeSample;
    param.lgate[iCh] = fDigiPar.LongGate[iCh] / fTimeSample;
    param.pgate[iCh] = fDigiPar.PreTriggerSize[iCh] / fTimeSample;
    param.tvaw[iCh] = fDigiPar.CoincWindow[iCh] / fTimeSample;
    param.nsbl[iCh] = fDigiPar.NSBaseLine[iCh];
    param.trgc[iCh] = CAEN_DGTZ_DPP_TriggerConfig_Threshold;  // set to 1
    // param.trgc[iCh] = CAEN_DGTZ_DPP_TriggerConfig_Peak;  // set to 0
    param.discr[iCh] = fDigiPar.DiscrMode[iCh];
    param.cfdd[iCh] = fDigiPar.CFDDelay[iCh] / fTimeSample;
    param.cfdf[iCh] = fDigiPar.CFDFraction[iCh];
  }
  param.blthr = 3;
  param.bltmo = 100;
  param.trgho = fDigiPar.TrgHoldOff[0] / fTimeSample;
  param.purh = fDigiPar.PileUpRejection;
  param.purgap = fDigiPar.PURGap;

  auto errCode = CAEN_DGTZ_SetDPPParameters(
      fHandler, (fDigiPar.ChMask) & ((1 << fNChs) - 1), &param);
  CheckErrCode(errCode, "SetDPPParameters");
}

void TPSD::AllocateMemory()
{
  CAEN_DGTZ_ErrorCode errCode;
  uint32_t size;

  errCode = CAEN_DGTZ_MallocReadoutBuffer(fHandler, &fpReadoutBuffer, &size);
  CheckErrCode(errCode, "MallocReadoutBuffer");

  fppPSDEvents = new CAEN_DGTZ_DPP_PSD_Event_t *[MAX_CHANNELS];
  errCode = CAEN_DGTZ_MallocDPPEvents(fHandler, (void **)fppPSDEvents, &size);
  CheckErrCode(errCode, "MallocDPPEvents");

  errCode =
      CAEN_DGTZ_MallocDPPWaveforms(fHandler, (void **)&fpPSDWaveform, &size);
  CheckErrCode(errCode, "MallocDPPWaveforms");
}

void TPSD::FreeMemory()
{
  CAEN_DGTZ_ErrorCode errCode;

  if (fpReadoutBuffer != nullptr) {
    errCode = CAEN_DGTZ_FreeReadoutBuffer(&fpReadoutBuffer);
    CheckErrCode(errCode, "CAEN_DGTZ_FreeReadoutBuffer");
    fpReadoutBuffer = nullptr;
  }

  if (fppPSDEvents != nullptr) {
    errCode = CAEN_DGTZ_FreeDPPEvents(fHandler, (void **)fppPSDEvents);
    CheckErrCode(errCode, "CAEN_DGTZ_FreeDPPEvents");
    fppPSDEvents = nullptr;
  }

  if (fpPSDWaveform != nullptr) {
    errCode = CAEN_DGTZ_FreeDPPWaveforms(fHandler, (void *)fpPSDWaveform);
    CheckErrCode(errCode, "CAEN_DGTZ_FreeDPPWaveforms");
    fpPSDWaveform = nullptr;
  }
}

std::vector<EveData *> *TPSD::GetEvents()
{
  for (auto &&ele : *fDataVec) delete ele;
  fDataVec->clear();

  ReadEvents();

  return fDataVec;
}

void TPSD::ReadEvents()
{
  CAEN_DGTZ_ErrorCode errCode;
  uint32_t bufferSize;
  errCode =
      CAEN_DGTZ_ReadData(fHandler, CAEN_DGTZ_SLAVE_TERMINATED_READOUT_MBLT,
                         fpReadoutBuffer, &bufferSize);
  CheckErrCode(errCode, "ReadData");
  if (bufferSize == 0) return;

  uint32_t nEvents[MAX_CHANNELS];
  errCode = CAEN_DGTZ_GetDPPEvents(fHandler, fpReadoutBuffer, bufferSize,
                                   (void **)(fppPSDEvents), nEvents);

  for (auto iCh = 0; iCh < 8; iCh++) {
    for (auto iEve = 0; iEve < nEvents[iCh]; iEve++) {
      const uint64_t TSMask = 0x7FFFFFFF;
      uint64_t timeTag = fppPSDEvents[iCh][iEve].TimeTag;
      if (timeTag < fPreviousTime[iCh]) {
        fTimeOffset[iCh] += (TSMask + 1);
      }
      fPreviousTime[iCh] = timeTag;
      auto tdc = (timeTag + fTimeOffset[iCh]) * fTimeSample;

      auto data = new EveData(fDigiPar.RecordLength[iCh]);
      data->ModNumber = 0;
      data->ChNumber = iCh;
      data->ChargeLong = fppPSDEvents[iCh][iEve].ChargeLong;
      data->ChargeShort = fppPSDEvents[iCh][iEve].ChargeShort;
      data->TimeStamp = tdc;
      data->FineTS = 0;
      if (fFlagFineTS) {
        // For safety and to kill the rounding error, cleary using double
        // No additional interpolation now
        double posZC =
            uint16_t((fppPSDEvents[iCh][iEve].Extras >> 16) & 0xFFFF);
        double negZC = uint16_t(fppPSDEvents[iCh][iEve].Extras & 0xFFFF);
        double thrZC = 8192;  // (1 << 13). (1 << 14) is maximum of ADC
        if (fDigiPar.DiscrMode[iCh] == CAEN_DGTZ_DPP_DISCR_MODE_LED)
          thrZC += fDigiPar.TrgTh[iCh];

        if ((negZC <= thrZC) && (posZC >= thrZC)) {
          double dt = fTimeSample;
          data->FineTS =
              ((dt * 1000. * (thrZC - negZC) / (posZC - negZC)) + 0.5);
        }
      }

      data->RecordLength = 0;
      if (fDigiPar.AcqMode == CAEN_DGTZ_DPP_ACQ_MODE_Mixed) {
        errCode = CAEN_DGTZ_DecodeDPPWaveforms(
            fHandler, &(fppPSDEvents[iCh][iEve]), fpPSDWaveform);
        CheckErrCode(errCode, "DecodeDPPWaveforms");

        data->RecordLength = fpPSDWaveform->Ns;
        data->Trace1.assign(&fpPSDWaveform->Trace1[0],
                            &fpPSDWaveform->Trace1[data->RecordLength]);
        data->Trace2.assign(&fpPSDWaveform->Trace2[0],
                            &fpPSDWaveform->Trace2[data->RecordLength]);

        data->DTrace1.assign(&fpPSDWaveform->DTrace1[0],
                             &fpPSDWaveform->DTrace1[data->RecordLength]);
        data->DTrace2.assign(&fpPSDWaveform->DTrace2[0],
                             &fpPSDWaveform->DTrace2[data->RecordLength]);
        data->DTrace3.assign(&fpPSDWaveform->DTrace3[0],
                             &fpPSDWaveform->DTrace3[data->RecordLength]);
        data->DTrace4.assign(&fpPSDWaveform->DTrace4[0],
                             &fpPSDWaveform->DTrace4[data->RecordLength]);
      }

      fDataVec->push_back(data);
    }
  }
}

void TPSD::EnableFineTS()
{  // This is for the x725 and x730 series.
   // For other models, check the registers address.

  // When we used Extra data as the fine TS.  Digitizer returned
  // comb (rounding error effect) shaped distribution.
  // Using zero crossing information and calculting by program is better.
  // This is also the reason, extended time stamp is not used, and this
  // class calcultes the extended time stamp in ReadEvents().

  for (uint iCh = 0; iCh < fNChs; iCh++) {
    // Using extra as fine TS
    // RegisterSetBits(fHandler, 0x1084 + (iCh << 8), 8, 10, 0b010,
    //                 fWDcfg);
    // Using extra as zero crossing information
    RegisterSetBits(0x1084 + (iCh << 8), 8, 10, 0b101);
  }

  // Trace settings
  RegisterSetBits(0x8000, 11, 11, 1);
  RegisterSetBits(0x8000, 12, 13, 1);
  RegisterSetBits(0x8000, 23, 25, 0b000);
  RegisterSetBits(0x8000, 26, 28, 0b111);

  fFlagFineTS = true;
}
