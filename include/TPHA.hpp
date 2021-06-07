#ifndef TPHA_hpp
#define TPHA_hpp 1

#include "TDigitizer.hpp"

class TPHA : public TDigitizer
{
 public:
  TPHA();
  ~TPHA();

  void Config();

  void AllocateMemory();
  void FreeMemory();

  std::vector<EveData *> *GetEvents();

  void EnableFineTS(){};  // NYI

 private:
  // Memory and readout
  char *fpReadoutBuffer = nullptr;                         // readout buffer
  CAEN_DGTZ_DPP_PHA_Event_t **fppPHAEvents = nullptr;      // events buffer
  CAEN_DGTZ_DPP_PHA_Waveforms_t *fpPHAWaveform = nullptr;  // waveforms buffer
  void ReadEvents();

  void PHAConfig();
  void PHARegisterSettings();
  void SetProbes();
};

#endif
