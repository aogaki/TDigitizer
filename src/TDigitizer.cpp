#include <iostream>

#include "ErrorCodeMap.hpp"
#include "TDigitizer.hpp"

TDigitizer::TDigitizer()
{
  fDataVec = new std::vector<EveData *>;
  fDataVec->reserve(1024);

  for (auto iCh = 0; iCh < MAX_CHANNELS; iCh++) {
    fPreviousTime[iCh] = 0;
    fTimeOffset[iCh] = 0;
  }
}

TDigitizer::~TDigitizer()
{
  for (auto &&ele : *fDataVec) delete ele;
  delete fDataVec;
  fDataVec = nullptr;
};

void TDigitizer::Open()
{
  auto errCode = CAEN_DGTZ_OpenDigitizer(fDigiPar.LinkType, fDigiPar.LinkNum,
                                         fDigiPar.ConetNode,
                                         fDigiPar.VMEBaseAddress, &fHandler);
  CheckErrCode(errCode, "Open");
}

void TDigitizer::Close()
{
  auto errCode = CAEN_DGTZ_CloseDigitizer(fHandler);
  CheckErrCode(errCode, "Close");
}

void TDigitizer::GetInfo()
{
  CAEN_DGTZ_BoardInfo_t info;
  CAEN_DGTZ_GetInfo(fHandler, &info);

  if (info.FamilyCode == 11) {
    fTimeSample = 2;
    fDigiModel = 730;
  } else if (info.FamilyCode == 14) {
    fTimeSample = 4;
    fDigiModel = 725;
  } else {
    std::cerr << "ERROR: Unknown digitizer model\n" << std::endl;
    exit(0);
  }

  uint32_t majorNumber = atoi(info.AMC_FirmwareRel);  // xxx.yyy x is major
  if (majorNumber == 136) {
    fDigiType = DigiType::DPP_PSD;
  } else if (majorNumber == 139) {
    fDigiType = DigiType::DPP_PHA;
  } else {
    fDigiType = DigiType::Waveform;
  }

  fNChs = info.Channels;

  std::cout << "\nHandler number:\t" << fHandler << "\n"
            << "Model name:\t" << info.ModelName << "\n"
            << "Model number:\t" << info.Model << "\n"
            << "No. channels:\t" << info.Channels << "\n"
            << "Format factor:\t" << info.FormFactor << "\n"
            << "Family code:\t" << info.FamilyCode << "\n"
            << "Firmware revision of the FPGA on the mother board (ROC):\t"
            << info.ROC_FirmwareRel << "\n"
            << "Firmware revision of the FPGA on the daughter board (AMC):\t"
            << info.AMC_FirmwareRel << "\n"
            << "Serial number:\t" << info.SerialNumber << "\n"
            << "PCB revision:\t" << info.PCB_Revision << "\n"
            << "No. bits of the ADC:\t" << info.ADC_NBits << "\n"
            << "Device handler of CAENComm:\t" << info.CommHandle << "\n"
            << "Device handler of CAENVME:\t" << info.VMEHandle << std::endl;

  auto license = std::string(info.License);
  if (fDigiType == DigiType::Waveform) {
    license = "Using std, wave record, free FW";
  } else if (fDigiType == DigiType::DPP_PSD || fDigiType == DigiType::DPP_PHA) {
    uint32_t val;
    CAEN_DGTZ_ReadRegister(fHandler, 0x8158, &val);
    if (val == 0x53D4) {
      license = "The DPP is licensed";
    } else {
      if (val > 0) {
        auto rest = (int)((float)val / 0x53D4 * 30);
        license = "WARNING: DPP not licensed: " + std::to_string(rest) +
                  " minutes remaining";
      } else {
        std::cout << "ERROR: DPP not licensed: time expired" << std::endl;
        exit(0);
      }
    }
  }

  std::cout << "License:\t" << license << std::endl;
}

void TDigitizer::CheckErrCode(CAEN_DGTZ_ErrorCode errCode, std::string funcName)
{
  if (errCode != CAEN_DGTZ_Success) {
    std::cout << funcName << ":\t" << ErrorCodeMap.at(errCode) << std::endl;
  }
};

void TDigitizer::Reset()
{
  auto errCode = CAEN_DGTZ_Reset(fHandler);
  CheckErrCode(errCode, "Reset");
  Calibrate();
}

void TDigitizer::Calibrate()
{
  auto errCode = CAEN_DGTZ_Calibrate(fHandler);
  CheckErrCode(errCode, "Calibrate");
}

void TDigitizer::SendSWTrg(unsigned int nTrg)
{
  for (unsigned int i = 0; i < nTrg; i++) {
    auto errCode = CAEN_DGTZ_SendSWtrigger(fHandler);
    CheckErrCode(errCode, "Send SWTrigger");
  }
}

void TDigitizer::TrgConf()
{
  CAEN_DGTZ_ErrorCode errCode;

  errCode = CAEN_DGTZ_SetSWTriggerMode(fHandler, fDigiPar.SWTrgMode);
  CheckErrCode(errCode, "SetSWTriggerMode");

  errCode = CAEN_DGTZ_SetExtTriggerInputMode(fHandler, fDigiPar.ExtTrgMode);
  CheckErrCode(errCode, "SetExtTriggerInputMode");

  // This sets global (for all channels ) trigger and/or TRG-OUT.
  // DPP Self trigger is set by another function.
  // For waveform, probably, this is the only way for self trigger.
  if (fDigiPar.TrgPropagation || fDigiType == DigiType::Waveform) {
    errCode = CAEN_DGTZ_SetChannelSelfTrigger(
        fHandler, fDigiPar.ChSelfTrg, (fDigiPar.ChMask) & ((1 << fNChs) - 1));
    CheckErrCode(errCode, "SetChannelSelfTrigger");
  }

  for (auto iCh = 0; iCh < fNChs; iCh++) {
    auto offset = fDigiPar.DCOffset[iCh];
    if (fDigiPar.Polarity[iCh] == CAEN_DGTZ_PulsePolarityNegative)
      offset = 100 - fDigiPar.DCOffset[iCh];
    auto zeroLevel = (1 << 14) * offset / 100;
    int th;
    if (fDigiPar.Polarity[iCh] == CAEN_DGTZ_PulsePolarityPositive) {
      th = zeroLevel + fDigiPar.TrgTh[iCh];
    } else {
      th = zeroLevel - fDigiPar.TrgTh[iCh];
    }
    errCode = CAEN_DGTZ_SetChannelTriggerThreshold(fHandler, iCh, th);
    CheckErrCode(errCode, "SetChannelTriggerThreshold");
  }

  for (auto iCh = 0; iCh < fNChs; iCh++) {
    auto offset = (1 << 16) * fDigiPar.DCOffset[iCh] / 100;
    if (fDigiPar.Polarity[iCh] == CAEN_DGTZ_PulsePolarityPositive)
      offset = (1 << 16) * (100 - fDigiPar.DCOffset[iCh]) / 100;
    errCode = CAEN_DGTZ_SetChannelDCOffset(fHandler, iCh, offset);
    CheckErrCode(errCode, "SetChannelDCOffset");
  }

  errCode = CAEN_DGTZ_SetRunSynchronizationMode(fHandler, fDigiPar.RunSyncMode);
  CheckErrCode(errCode, "SetRunSynchronizationMode");

  errCode = CAEN_DGTZ_SetIOLevel(fHandler, fDigiPar.IOLevel);
  CheckErrCode(errCode, "SetIOLevel");

  if (fDigiType == DigiType::Waveform) {
    for (auto iCh = 0; iCh < fNChs; iCh++) {
      errCode = CAEN_DGTZ_SetTriggerPolarity(fHandler, iCh,
                                             fDigiPar.TrgPolarity[iCh]);
      CheckErrCode(errCode, "SetTriggerPolarity");
    }
  }
}

void TDigitizer::AcqConf()
{
  CAEN_DGTZ_ErrorCode errCode;

  errCode = CAEN_DGTZ_SetChannelEnableMask(
      fHandler, (fDigiPar.ChMask) & ((1 << fNChs) - 1));
  CheckErrCode(errCode, "SetChannelEnableMask");

  for (auto iCh = 0; iCh < fNChs; iCh++) {
    if ((fDigiModel != 730 && fDigiModel != 725) || (iCh % 2) == 0) {
      auto recLength = fDigiPar.RecordLength[iCh] / fTimeSample;
      errCode = CAEN_DGTZ_SetRecordLength(fHandler, recLength, iCh);
      CheckErrCode(errCode, "SetRecordLength");
    }
  }

  if (fDigiType == DigiType::Waveform) {
    // if preTrigger = recLength, still the trigger position of waveform is
    // not at the tail of waveform. I need the offset (128)
    auto recLength = fDigiPar.RecordLength[0];
    auto preTrigger = fDigiPar.PreTriggerSize[0] + 128;
    if (preTrigger > recLength) preTrigger = recLength;
    uint32_t postTrgSize = (100 * (recLength - preTrigger)) / recLength;
    errCode = CAEN_DGTZ_SetPostTriggerSize(fHandler, postTrgSize);
    CheckErrCode(errCode, "SetPostTriggerSize");
  }

  errCode = CAEN_DGTZ_SetAcquisitionMode(fHandler, fDigiPar.StartMode);
  CheckErrCode(errCode, "SetAcquisitionMode");
}

void TDigitizer::DPPConf()
{
  CAEN_DGTZ_ErrorCode errCode;

  for (auto iCh = 0; iCh < fNChs; iCh++) {
    uint32_t preTrigger = fDigiPar.PreTriggerSize[iCh] / fTimeSample;
    errCode = CAEN_DGTZ_SetDPPPreTriggerSize(fHandler, iCh, preTrigger);
    CheckErrCode(errCode, "SetDPPPreTriggerSize");

    errCode = CAEN_DGTZ_SetChannelPulsePolarity(fHandler, iCh,
                                                fDigiPar.Polarity[iCh]);
    CheckErrCode(errCode, "SetChannelPulsePolarity");
  }

  errCode = CAEN_DGTZ_SetDPPAcquisitionMode(
      fHandler, fDigiPar.AcqMode, CAEN_DGTZ_DPP_SAVE_PARAM_EnergyAndTime);
  CheckErrCode(errCode, "SetDPPAcquisitionMode");

  if (fDigiType == DigiType::DPP_PSD) {
    errCode = CAEN_DGTZ_SetDPPTriggerMode(fHandler, fDigiPar.Coincidence);
    CheckErrCode(errCode, "SetDPPTriggerMode");
  }
}

void TDigitizer::BufferConf()
{
  CAEN_DGTZ_ErrorCode errCode;

  // uint32_t tmp;
  // CAEN_DGTZ_GetNumEventsPerAggregate(fHandler, &tmp);
  // std::cout << tmp << std::endl;

  if (fDigiType == DigiType::Waveform) {
    errCode = CAEN_DGTZ_SetMaxNumEventsBLT(fHandler, 1023);
    CheckErrCode(errCode, "SetMaxNumEventsBLT");
  } else {
    errCode = CAEN_DGTZ_SetMaxNumAggregatesBLT(fHandler, 1023);
    CheckErrCode(errCode, "SetMaxNumAggregatesBLT");

    errCode =
        CAEN_DGTZ_SetDPPEventAggregation(fHandler, fDigiPar.EventBuffering, 0);
    CheckErrCode(errCode, "SetDPPEventAggregation");
  }
}

void TDigitizer::RegisterSettings()
{
  for (auto iCh = 0; iCh < fNChs; iCh++) {
    if ((fDigiModel == 730) || (fDigiModel == 725)) {
      auto errCode = CAEN_DGTZ_WriteRegister(
          fHandler, 0x1028 + (iCh << 8),
          fDigiPar.DynamicRange[iCh] & 1);  // 0=2Vpp, 1=0.5Vpp
      CheckErrCode(errCode, "SetDynamicRange");
    }
  }
}

void TDigitizer::RegisterSetBits(uint16_t addr, int start_bit, int end_bit,
                                 int val)
{
  uint32_t mask = 0, reg;
  uint16_t ch;
  CAEN_DGTZ_ErrorCode errCode;

  if (((addr & 0xFF00) == 0x8000) && (addr != 0x8000) && (addr != 0x8004) &&
      (addr != 0x8008)) {
    // broadcast access to channel individual registers (loop over channels)
    for (ch = 0; ch < fNChs; ch++) {
      errCode = CAEN_DGTZ_ReadRegister(
          fHandler, 0x1000 | (addr & 0xFF) | (ch << 8), &reg);
      CheckErrCode(errCode, "ReadRegister");

      for (auto i = start_bit; i <= end_bit; i++) mask |= 1 << i;
      reg = (reg & ~mask) | ((val << start_bit) & mask);
      errCode = CAEN_DGTZ_WriteRegister(
          fHandler, 0x1000 | (addr & 0xFF) | (ch << 8), reg);
      CheckErrCode(errCode, "WriteRegister");
    }
  } else {  // access to channel individual register or mother board register
    errCode = CAEN_DGTZ_ReadRegister(fHandler, addr, &reg);
    CheckErrCode(errCode, "ReadRegister");

    for (auto i = start_bit; i <= end_bit; i++) mask |= 1 << i;
    reg = (reg & ~mask) | ((val << start_bit) & mask);
    errCode = CAEN_DGTZ_WriteRegister(fHandler, addr, reg);
    CheckErrCode(errCode, "WriteRegister");
  }
}

void TDigitizer::Start()
{
  if (fDigiPar.RunSync == SyncType::StandAlone) {
    auto errCode = CAEN_DGTZ_SWStartAcquisition(fHandler);
    CheckErrCode(errCode, "CAEN_DGTZ_SWStartAcquisition");
  } else {
    // Synchronization mode NYI
  }
}
void TDigitizer::Stop()
{
  auto errCode = CAEN_DGTZ_SWStopAcquisition(fHandler);
  CheckErrCode(errCode, "CAEN_DGTZ_SWStopAcquisition");
}
