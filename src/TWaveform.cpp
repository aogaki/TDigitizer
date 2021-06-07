#include "TWaveform.hpp"

TWaveform::TWaveform() {}

TWaveform::~TWaveform() { FreeMemory(); }

void TWaveform::Config()
{
  TrgConf();
  AcqConf();
  RegisterSettings();

  BufferConf();
}

void TWaveform::AllocateMemory()
{
  CAEN_DGTZ_ErrorCode errCode;
  uint32_t size;

  errCode = CAEN_DGTZ_MallocReadoutBuffer(fHandler, &(fpReadoutBuffer), &size);
  CheckErrCode(errCode, "MallocReadoutBuffer");

  errCode = CAEN_DGTZ_AllocateEvent(fHandler, (void **)&fpEventStd);
  CheckErrCode(errCode, "AllocateEvent");
}

void TWaveform::FreeMemory()
{
  CAEN_DGTZ_ErrorCode errCode;

  if (fpReadoutBuffer != nullptr) {
    errCode = CAEN_DGTZ_FreeReadoutBuffer(&(fpReadoutBuffer));
    CheckErrCode(errCode, "FreeReadoutBuffer");
    fpReadoutBuffer = nullptr;
  }

  if (fpEventStd != nullptr) {
    errCode = CAEN_DGTZ_FreeEvent(fHandler, (void **)&fpEventStd);
    CheckErrCode(errCode, "FreeEvent");
    fpEventStd = nullptr;
  }
}

std::vector<EveData *> *TWaveform::GetEvents()
{
  for (auto &&ele : *fDataVec) delete ele;
  fDataVec->clear();

  ReadEvents();

  return fDataVec;
}

void TWaveform::ReadEvents()
{
  CAEN_DGTZ_ErrorCode errCode;
  uint32_t bufferSize;
  errCode =
      CAEN_DGTZ_ReadData(fHandler, CAEN_DGTZ_SLAVE_TERMINATED_READOUT_MBLT,
                         fpReadoutBuffer, &bufferSize);
  CheckErrCode(errCode, "ReadData");

  uint32_t nEvents;
  errCode =
      CAEN_DGTZ_GetNumEvents(fHandler, fpReadoutBuffer, bufferSize, &nEvents);
  CheckErrCode(errCode, "GetNumEvents");

  for (auto iEve = 0; iEve < nEvents; iEve++) {
    CAEN_DGTZ_EventInfo_t eventInfo;
    char *pEventPtr;

    errCode = CAEN_DGTZ_GetEventInfo(fHandler, fpReadoutBuffer, bufferSize,
                                     iEve, &eventInfo, &pEventPtr);
    CheckErrCode(errCode, "GetEventInfo");
    // std::cout << "Event number:\t" << iEve << '\n'
    //           << "Event size:\t" << eventInfo.EventSize << '\n'
    //           << "Board ID:\t" << eventInfo.BoardId << '\n'
    //           << "Pattern:\t" << eventInfo.Pattern << '\n'
    //           << "Ch mask:\t" << eventInfo.ChannelMask << '\n'
    //           << "Event counter:\t" << eventInfo.EventCounter << '\n'
    //           << "Trigger time tag:\t" << eventInfo.TriggerTimeTag
    //           << std::endl;

    errCode = CAEN_DGTZ_DecodeEvent(fHandler, pEventPtr, (void **)&fpEventStd);
    CheckErrCode(errCode, "DecodeEvent");

    uint64_t timeStamp =
        (eventInfo.TriggerTimeTag + fTimeOffset[0]) * fTimeSample;
    if (timeStamp < fPreviousTime[0]) {
      constexpr uint32_t maxTime = 0xFFFFFFFF / 2;  // Check manual
      timeStamp += maxTime * fTimeSample;
      fTimeOffset[0] += maxTime;
    }
    fPreviousTime[0] = timeStamp;

    for (uint32_t iCh = 0; iCh < fNChs; iCh++) {
      if (!((fDigiPar.ChMask >> iCh) & 0x1)) continue;

      const uint32_t size = fpEventStd->ChSize[iCh];
      auto dataEle = new EveData(size);
      dataEle->ModNumber = 0;
      dataEle->ChNumber = iCh;
      dataEle->TimeStamp = timeStamp;
      dataEle->RecordLength = size;
      dataEle->Trace1.assign(&(fpEventStd->DataChannel[iCh])[0],
                             &(fpEventStd->DataChannel[iCh])[size]);

      fDataVec->push_back(dataEle);
    }
  }
}

void TWaveform::EnableFineTS() { fFlagFineTS = true; }
