#ifndef TWaveform_hpp
#define TWaveform_hpp 1

#include "TDigitizer.hpp"

class TWaveform : public TDigitizer
{
 public:
  TWaveform();
  ~TWaveform();

  void Config();

  void AllocateMemory();
  void FreeMemory();

  std::vector<EveData *> *GetEvents();

  void EnableFineTS();

 private:
  // Memory and readout
  char *fpReadoutBuffer = nullptr;                 // readout buffer
  CAEN_DGTZ_UINT16_EVENT_t *fpEventStd = nullptr;  // events buffer
  void ReadEvents();
};

#endif
