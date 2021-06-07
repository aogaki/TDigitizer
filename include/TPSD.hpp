#ifndef TPSD_hpp
#define TPSD_hpp 1

#include "TDigitizer.hpp"

class TPSD : public TDigitizer
{
 public:
  TPSD();
  ~TPSD();

  void Config();

  void AllocateMemory();
  void FreeMemory();

  std::vector<EveData *> *GetEvents();

  void EnableFineTS();

 private:
  // Memory and readout
  char *fpReadoutBuffer = nullptr;                         // readout buffer
  CAEN_DGTZ_DPP_PSD_Event_t **fppPSDEvents = nullptr;      // events buffer
  CAEN_DGTZ_DPP_PSD_Waveforms_t *fpPSDWaveform = nullptr;  // waveforms buffer
  void ReadEvents();

  void PSDConfig();
};

#endif
