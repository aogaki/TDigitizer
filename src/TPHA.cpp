#include <iostream>

#include "TPHA.hpp"

TPHA::TPHA() {}

TPHA::~TPHA() { FreeMemory(); }

void TPHA::Config()
{
  TrgConf();
  AcqConf();
  DPPConf();
  PHAConfig();
  SetProbes();
  RegisterSettings();

  BufferConf();
}

void TPHA::PHAConfig()
{
  CAEN_DGTZ_DPP_PHA_Params_t param;
  for (auto iCh = 0; iCh < fNChs; iCh++) {
    param.M[iCh] = fDigiPar.TrapPoleZero[iCh];
    param.m[iCh] = fDigiPar.TrapFlatTop[iCh];
    param.k[iCh] = fDigiPar.TrapRiseTime[iCh];
    param.ftd[iCh] = fDigiPar.PeakingTime[iCh];
    param.a[iCh] = fDigiPar.TTFSmoothing[iCh];
    param.b[iCh] = fDigiPar.SignalRiseTime[iCh];
    param.thr[iCh] = fDigiPar.TrgTh[iCh];
    param.nsbl[iCh] = fDigiPar.NSBaseLine[iCh];
    param.nspk[iCh] = fDigiPar.NSPeak[iCh];
    param.pkho[iCh] = fDigiPar.PeakHoldOff[iCh];
    param.blho[iCh] = fDigiPar.BaseLineHoldOff[iCh];
    param.trgho[iCh] = fDigiPar.TrgHoldOff[iCh];
    param.twwdt[iCh] = fDigiPar.ZeroCrossAccWindow[iCh];
    param.trgwin[iCh] = fDigiPar.TrgAccWindow[iCh];
    param.dgain[iCh] = fDigiPar.DigitalGain[iCh];
    param.enf[iCh] = fDigiPar.EneFineGain[iCh];
    param.decimation[iCh] = fDigiPar.Decimation[iCh];
  }

  auto errCode = CAEN_DGTZ_SetDPPParameters(
      fHandler, (fDigiPar.ChMask) & ((1 << fNChs) - 1), &param);
  CheckErrCode(errCode, "SetDPPParameters");
}

void TPHA::PHARegisterSettings()
{
  // Self trigger settings
}

void TPHA::AllocateMemory()
{
  CAEN_DGTZ_ErrorCode errCode;
  uint32_t size;

  errCode = CAEN_DGTZ_MallocReadoutBuffer(fHandler, &fpReadoutBuffer, &size);
  CheckErrCode(errCode, "MallocReadoutBuffer");

  fppPHAEvents = new CAEN_DGTZ_DPP_PHA_Event_t *[MAX_CHANNELS];
  errCode = CAEN_DGTZ_MallocDPPEvents(fHandler, (void **)fppPHAEvents, &size);
  CheckErrCode(errCode, "MallocDPPEvents");

  errCode =
      CAEN_DGTZ_MallocDPPWaveforms(fHandler, (void **)&fpPHAWaveform, &size);
  CheckErrCode(errCode, "MallocDPPWaveforms");
}

void TPHA::FreeMemory()
{
  CAEN_DGTZ_ErrorCode errCode;

  if (fpReadoutBuffer != nullptr) {
    errCode = CAEN_DGTZ_FreeReadoutBuffer(&fpReadoutBuffer);
    CheckErrCode(errCode, "CAEN_DGTZ_FreeReadoutBuffer");
    fpReadoutBuffer = nullptr;
  }

  if (fppPHAEvents != nullptr) {
    errCode = CAEN_DGTZ_FreeDPPEvents(fHandler, (void **)fppPHAEvents);
    CheckErrCode(errCode, "CAEN_DGTZ_FreeDPPEvents");
    fppPHAEvents = nullptr;
  }

  if (fpPHAWaveform != nullptr) {
    errCode = CAEN_DGTZ_FreeDPPWaveforms(fHandler, (void *)fpPHAWaveform);
    CheckErrCode(errCode, "CAEN_DGTZ_FreeDPPWaveforms");
    fpPHAWaveform = nullptr;
  }
}

std::vector<EveData *> *TPHA::GetEvents()
{
  for (auto &&ele : *fDataVec) delete ele;
  fDataVec->clear();

  ReadEvents();

  return fDataVec;
}

void TPHA::ReadEvents()
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
                                   (void **)(fppPHAEvents), nEvents);

  if (errCode == CAEN_DGTZ_Success) {
    for (auto iCh = 0; iCh < 8; iCh++) {
      for (auto iEve = 0; iEve < nEvents[iCh]; iEve++) {
        const uint64_t TSMask = 0x7FFFFFFF;
        uint64_t timeTag = fppPHAEvents[iCh][iEve].TimeTag;
        if (timeTag < fPreviousTime[iCh]) {
          fTimeOffset[iCh] += (TSMask + 1);
        }
        fPreviousTime[iCh] = timeTag;
        auto tdc = (timeTag + fTimeOffset[iCh]) * fTimeSample;

        auto data = new EveData(fDigiPar.RecordLength[iCh]);
        data->ModNumber = 0;
        data->ChNumber = iCh;
        data->ChargeLong = fppPHAEvents[iCh][iEve].Energy;
        data->TimeStamp = tdc;
        data->FineTS = 0;
        if (fFlagFineTS) {
          // For safety and to kill the rounding error, cleary using double
          // No additional interpolation now
          double posZC =
              uint16_t((fppPHAEvents[iCh][iEve].Extras >> 16) & 0xFFFF);
          double negZC = uint16_t(fppPHAEvents[iCh][iEve].Extras & 0xFFFF);
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
              fHandler, &(fppPHAEvents[iCh][iEve]), fpPHAWaveform);
          CheckErrCode(errCode, "DecodeDPPWaveforms");

          data->RecordLength = fpPHAWaveform->Ns;
          data->Trace1.assign(&fpPHAWaveform->Trace1[0],
                              &fpPHAWaveform->Trace1[data->RecordLength]);
          data->Trace2.assign(&fpPHAWaveform->Trace2[0],
                              &fpPHAWaveform->Trace2[data->RecordLength]);

          data->DTrace1.assign(&fpPHAWaveform->DTrace1[0],
                               &fpPHAWaveform->DTrace1[data->RecordLength]);
          data->DTrace2.assign(&fpPHAWaveform->DTrace2[0],
                               &fpPHAWaveform->DTrace2[data->RecordLength]);
        }

        fDataVec->push_back(data);
      }
    }
  }
}

void TPHA::SetProbes()
{
  CAEN_DGTZ_ErrorCode errCode = CAEN_DGTZ_Success;

  errCode = CAEN_DGTZ_SetDPP_VirtualProbe(fHandler, ANALOG_TRACE_1,
                                          CAEN_DGTZ_DPP_VIRTUALPROBE_Input);
  CheckErrCode(errCode, "Set Trace1");

  // int probes[16];
  // int nProbes = 0;
  // CAEN_DGTZ_GetDPP_SupportedVirtualProbes(fHandler, ANALOG_TRACE_2, probes,
  //                                         &nProbes);
  // for (auto i = 0; i < nProbes; i++) std::cout << probes[i] << std::endl;

  errCode = CAEN_DGTZ_SetDPP_VirtualProbe(
      fHandler, ANALOG_TRACE_2, CAEN_DGTZ_DPP_VIRTUALPROBE_TrapezoidReduced);
  CheckErrCode(errCode, "Set Trace2");
}
