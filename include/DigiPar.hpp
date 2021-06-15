#ifndef DigiPar_hpp
#define DigiPar_hpp 1

#include <CAENDigitizer.h>
#include <CAENDigitizerType.h>

constexpr int MAX_CHANNELS = 16;

// To convert to bson, enum class needs tricks
// Using old enum is easy but can be the bug source
enum SyncType {
  Master,
  Slave,
  StandAlone,
};

class DigiPar
{
 public:
  DigiPar()
  {
    for (auto iCh = 0; iCh < MAX_CHANNELS; iCh++) {
      TrgTh[iCh] = 500;
      DCOffset[iCh] = 20;
      Polarity[iCh] = CAEN_DGTZ_PulsePolarityPositive;
      TrgPolarity[iCh] = CAEN_DGTZ_TriggerOnRisingEdge;
      RecordLength[iCh] = 20000;
      PreTriggerSize[iCh] = 10 * RecordLength[iCh] / 100;
      TrgHoldOff[iCh] = 1024;
      ChargeSens[iCh] = 2;
      ShortGate[iCh] = 64;
      LongGate[iCh] = 256;
      CoincWindow[iCh] = 128;
      NSBaseLine[iCh] = 1;
      DiscrMode[iCh] = CAEN_DGTZ_DPP_DISCR_MODE_LED;
      CFDDelay[iCh] = 8;
      CFDFraction[iCh] = 0;  // same as digiTes manual

      DynamicRange[iCh] = 0;  // 0 = 2Vpp, 1 = 0.5Vpp

      TrapPoleZero[iCh] = 6800;
      TrapFlatTop[iCh] = 992;
      TrapRiseTime[iCh] = 4992;
      PeakingTime[iCh] = 80 * TrapFlatTop[iCh] / 100;
      TTFSmoothing[iCh] = 2;
      SignalRiseTime[iCh] = 96;
      NSPeak[iCh] = 0;
      PeakHoldOff[iCh] = 992;
      BaseLineHoldOff[iCh] = 128;
      ZeroCrossAccWindow[iCh] = 0;
      TrgAccWindow[iCh] = 0;
      DigitalGain[iCh] = 0;
      EneFineGain[iCh] = 1.0;
      Decimation[iCh] = 0;
    }
  };

  ~DigiPar(){};

  // PAR_DEF_START
  // For Communication with digitizer
  CAEN_DGTZ_ConnectionType LinkType = CAEN_DGTZ_USB;
  int LinkNum = 0;
  int ConetNode = 0;
  uint32_t VMEBaseAddress = 0x0;
  int BrdNum = 0;

  // Sync Settings
  CAEN_DGTZ_RunSyncMode_t RunSyncMode = CAEN_DGTZ_RUN_SYNC_Disabled;
  CAEN_DGTZ_AcqMode_t StartMode = CAEN_DGTZ_SW_CONTROLLED;
  SyncType RunSync = SyncType::StandAlone;

  // Channel input settings
  uint32_t ChMask = 0b1111111111111111;
  // uint32_t ChMask = 0b00001000;
  uint32_t DCOffset[MAX_CHANNELS];
  CAEN_DGTZ_PulsePolarity_t Polarity[MAX_CHANNELS];
  CAEN_DGTZ_TriggerPolarity_t TrgPolarity[MAX_CHANNELS];
  int ChargeSens[MAX_CHANNELS];

  // Trigger configuration
  CAEN_DGTZ_TriggerMode_t SWTrgMode = CAEN_DGTZ_TRGMODE_ACQ_AND_EXTOUT;
  CAEN_DGTZ_TriggerMode_t ExtTrgMode = CAEN_DGTZ_TRGMODE_ACQ_AND_EXTOUT;
  CAEN_DGTZ_TriggerMode_t ChSelfTrg = CAEN_DGTZ_TRGMODE_ACQ_AND_EXTOUT;
  // CAEN_DGTZ_TriggerMode_t ChSelfTrg = CAEN_DGTZ_TRGMODE_ACQ_ONLY;
  // CAEN_DGTZ_TriggerMode_t ChSelfTrg = CAEN_DGTZ_TRGMODE_DISABLED;
  bool TrgPropagation = false;
  uint32_t TrgTh[MAX_CHANNELS];
  int TrgHoldOff[MAX_CHANNELS];
  int ShortGate[MAX_CHANNELS];
  int LongGate[MAX_CHANNELS];
  int CoincWindow[MAX_CHANNELS];
  int NSBaseLine[MAX_CHANNELS];
  // CAEN_DGTZ_DPP_PUR_t PileUpRejection = CAEN_DGTZ_DPP_PSD_PUR_Enabled;
  CAEN_DGTZ_DPP_PUR_t PileUpRejection = CAEN_DGTZ_DPP_PSD_PUR_DetectOnly;
  int PURGap = 500;
  int DiscrMode[MAX_CHANNELS];
  int CFDDelay[MAX_CHANNELS];
  int CFDFraction[MAX_CHANNELS];

  // Acquisition configuration
  uint32_t RecordLength[MAX_CHANNELS];
  uint32_t PreTriggerSize[MAX_CHANNELS];
  CAEN_DGTZ_IOLevel_t IOLevel = CAEN_DGTZ_IOLevel_NIM;
  CAEN_DGTZ_DPP_AcqMode_t AcqMode = CAEN_DGTZ_DPP_ACQ_MODE_Mixed;
  CAEN_DGTZ_DPP_TriggerMode_t Coincidence = CAEN_DGTZ_DPP_TriggerMode_Normal;
  uint32_t EventBuffering = 0;  // 0 means automatic
  uint32_t DynamicRange[MAX_CHANNELS];

  // PHA parameters
  int TrapPoleZero[MAX_CHANNELS];
  int TrapFlatTop[MAX_CHANNELS];
  int TrapRiseTime[MAX_CHANNELS];
  int PeakingTime[MAX_CHANNELS];
  int TTFSmoothing[MAX_CHANNELS];
  int SignalRiseTime[MAX_CHANNELS];
  int NSPeak[MAX_CHANNELS];
  int PeakHoldOff[MAX_CHANNELS];
  int BaseLineHoldOff[MAX_CHANNELS];
  int ZeroCrossAccWindow[MAX_CHANNELS];
  int TrgAccWindow[MAX_CHANNELS];
  int DigitalGain[MAX_CHANNELS];
  float EneFineGain[MAX_CHANNELS];
  int Decimation[MAX_CHANNELS];

  //PAR_DEF_STOP

 private:
};

#endif
