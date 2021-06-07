#ifndef TDigitizer_hpp
#define TDigitizer_hpp 1

#include <CAENDigitizer.h>
#include <CAENDigitizerType.h>

#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "DigiPar.hpp"
#include "EveData.hpp"

enum class DigiType {
  DPP_PHA,
  DPP_PSD,
  Waveform,
};

class TDigitizer
{
 public:
  TDigitizer();
  virtual ~TDigitizer();
  void LoadPar(DigiPar par) { fDigiPar = par; };

  void Open();
  void Close();

  void GetInfo();

  void Reset();
  void Calibrate();
  virtual void Config() = 0;

  virtual void AllocateMemory() = 0;
  virtual void FreeMemory() = 0;

  void Start();
  void Stop();

  virtual std::vector<EveData *> *GetEvents() = 0;

  virtual void EnableFineTS() = 0;

  void SendSWTrg(unsigned int nTrg = 1);

 protected:
  std::vector<EveData *> *fDataVec = nullptr;

  int fHandler;
  int fNChs;
  int fTimeSample = 2;

  DigiPar fDigiPar;

  DigiType fDigiType = DigiType::DPP_PSD;
  int fDigiModel = 730;

  void CheckErrCode(CAEN_DGTZ_ErrorCode errCode, std::string funcName = "");

  void AcqConf();
  void TrgConf();
  void DPPConf();
  void BufferConf();  // Should be called at the end of configuration
  void RegisterSettings();
  void RegisterSetBits(uint16_t addr, int start_bit, int end_bit, int val);

  // For Fine TS
  bool fFlagFineTS = false;
  uint64_t fPreviousTime[MAX_CHANNELS];
  uint64_t fTimeOffset[MAX_CHANNELS];
};

#endif
