#include <bsoncxx/exception/exception.hpp>
#include <iostream>

#include "TParHandler.hpp"

TParHandler::TParHandler()
    : fPool(mongocxx::uri("mongodb://daq:nim2camac@172.18.4.56/ELIADE"))
{
}

TParHandler::~TParHandler() {}

void TParHandler::UploadParameters(std::string name)
{
  auto doc = GetParameterBson(name);
  auto conn = fPool.acquire();
  auto collection = (*conn)["ELIADE"]["Digitizer"];
  collection.insert_one(doc.view());
}

bsoncxx::document::value TParHandler::GetParameterBson(std::string name)
{
  fParameters.resize(2);
  auto builder = bsoncxx::builder::stream::document{};
  builder << "Name" << name;
  builder << "Time" << time(nullptr);
  for (auto iBrd = 0; iBrd < fParameters.size(); iBrd++) {
    builder << "Digitizer" + std::to_string(iBrd)
            << bsoncxx::builder::stream::open_document;

    builder << "LinkType" << fParameters[iBrd].LinkType;

    builder << "LinkNum" << fParameters[iBrd].LinkNum;

    builder << "ConetNode" << fParameters[iBrd].ConetNode;

    builder << "VMEBaseAddress"
            << static_cast<int32_t>(fParameters[iBrd].VMEBaseAddress);

    builder << "BrdNum" << fParameters[iBrd].BrdNum;

    builder << "RunSyncMode" << fParameters[iBrd].RunSyncMode;

    builder << "StartMode" << fParameters[iBrd].StartMode;

    builder << "RunSync" << fParameters[iBrd].RunSync;

    builder << "ChMask" << static_cast<int32_t>(fParameters[iBrd].ChMask);

    auto arrDCOffset = bsoncxx::builder::stream::array{};
    for (auto iCh = 0; iCh < MAX_CHANNELS; iCh++)
      arrDCOffset << static_cast<int32_t>(fParameters[iBrd].DCOffset[iCh]);
    builder << "DCOffset" << arrDCOffset;

    auto arrPolarity = bsoncxx::builder::stream::array{};
    for (auto iCh = 0; iCh < MAX_CHANNELS; iCh++)
      arrPolarity << fParameters[iBrd].Polarity[iCh];
    builder << "Polarity" << arrPolarity;

    auto arrTrgPolarity = bsoncxx::builder::stream::array{};
    for (auto iCh = 0; iCh < MAX_CHANNELS; iCh++)
      arrTrgPolarity << fParameters[iBrd].TrgPolarity[iCh];
    builder << "TrgPolarity" << arrTrgPolarity;

    auto arrChargeSens = bsoncxx::builder::stream::array{};
    for (auto iCh = 0; iCh < MAX_CHANNELS; iCh++)
      arrChargeSens << fParameters[iBrd].ChargeSens[iCh];
    builder << "ChargeSens" << arrChargeSens;

    builder << "SWTrgMode" << fParameters[iBrd].SWTrgMode;

    builder << "ExtTrgMode" << fParameters[iBrd].ExtTrgMode;

    builder << "ChSelfTrg" << fParameters[iBrd].ChSelfTrg;

    builder << "TrgPropagation" << fParameters[iBrd].TrgPropagation;

    auto arrTrgTh = bsoncxx::builder::stream::array{};
    for (auto iCh = 0; iCh < MAX_CHANNELS; iCh++)
      arrTrgTh << static_cast<int32_t>(fParameters[iBrd].TrgTh[iCh]);
    builder << "TrgTh" << arrTrgTh;

    auto arrTrgHoldOff = bsoncxx::builder::stream::array{};
    for (auto iCh = 0; iCh < MAX_CHANNELS; iCh++)
      arrTrgHoldOff << fParameters[iBrd].TrgHoldOff[iCh];
    builder << "TrgHoldOff" << arrTrgHoldOff;

    auto arrShortGate = bsoncxx::builder::stream::array{};
    for (auto iCh = 0; iCh < MAX_CHANNELS; iCh++)
      arrShortGate << fParameters[iBrd].ShortGate[iCh];
    builder << "ShortGate" << arrShortGate;

    auto arrLongGate = bsoncxx::builder::stream::array{};
    for (auto iCh = 0; iCh < MAX_CHANNELS; iCh++)
      arrLongGate << fParameters[iBrd].LongGate[iCh];
    builder << "LongGate" << arrLongGate;

    auto arrCoincWindow = bsoncxx::builder::stream::array{};
    for (auto iCh = 0; iCh < MAX_CHANNELS; iCh++)
      arrCoincWindow << fParameters[iBrd].CoincWindow[iCh];
    builder << "CoincWindow" << arrCoincWindow;

    auto arrNSBaseLine = bsoncxx::builder::stream::array{};
    for (auto iCh = 0; iCh < MAX_CHANNELS; iCh++)
      arrNSBaseLine << fParameters[iBrd].NSBaseLine[iCh];
    builder << "NSBaseLine" << arrNSBaseLine;

    builder << "PileUpRejection" << fParameters[iBrd].PileUpRejection;

    builder << "PURGap" << fParameters[iBrd].PURGap;

    auto arrDiscrMode = bsoncxx::builder::stream::array{};
    for (auto iCh = 0; iCh < MAX_CHANNELS; iCh++)
      arrDiscrMode << fParameters[iBrd].DiscrMode[iCh];
    builder << "DiscrMode" << arrDiscrMode;

    auto arrCFDDelay = bsoncxx::builder::stream::array{};
    for (auto iCh = 0; iCh < MAX_CHANNELS; iCh++)
      arrCFDDelay << fParameters[iBrd].CFDDelay[iCh];
    builder << "CFDDelay" << arrCFDDelay;

    auto arrCFDFraction = bsoncxx::builder::stream::array{};
    for (auto iCh = 0; iCh < MAX_CHANNELS; iCh++)
      arrCFDFraction << fParameters[iBrd].CFDFraction[iCh];
    builder << "CFDFraction" << arrCFDFraction;

    auto arrRecordLength = bsoncxx::builder::stream::array{};
    for (auto iCh = 0; iCh < MAX_CHANNELS; iCh++)
      arrRecordLength
          << static_cast<int32_t>(fParameters[iBrd].RecordLength[iCh]);
    builder << "RecordLength" << arrRecordLength;

    auto arrPreTriggerSize = bsoncxx::builder::stream::array{};
    for (auto iCh = 0; iCh < MAX_CHANNELS; iCh++)
      arrPreTriggerSize
          << static_cast<int32_t>(fParameters[iBrd].PreTriggerSize[iCh]);
    builder << "PreTriggerSize" << arrPreTriggerSize;

    builder << "IOLevel" << fParameters[iBrd].IOLevel;

    builder << "AcqMode" << fParameters[iBrd].AcqMode;

    builder << "Coincidence" << fParameters[iBrd].Coincidence;

    builder << "EventBuffering"
            << static_cast<int32_t>(fParameters[iBrd].EventBuffering);

    auto arrDynamicRange = bsoncxx::builder::stream::array{};
    for (auto iCh = 0; iCh < MAX_CHANNELS; iCh++)
      arrDynamicRange
          << static_cast<int32_t>(fParameters[iBrd].DynamicRange[iCh]);
    builder << "DynamicRange" << arrDynamicRange;

    auto arrTrapPoleZero = bsoncxx::builder::stream::array{};
    for (auto iCh = 0; iCh < MAX_CHANNELS; iCh++)
      arrTrapPoleZero << fParameters[iBrd].TrapPoleZero[iCh];
    builder << "TrapPoleZero" << arrTrapPoleZero;

    auto arrTrapFlatTop = bsoncxx::builder::stream::array{};
    for (auto iCh = 0; iCh < MAX_CHANNELS; iCh++)
      arrTrapFlatTop << fParameters[iBrd].TrapFlatTop[iCh];
    builder << "TrapFlatTop" << arrTrapFlatTop;

    auto arrTrapRiseTime = bsoncxx::builder::stream::array{};
    for (auto iCh = 0; iCh < MAX_CHANNELS; iCh++)
      arrTrapRiseTime << fParameters[iBrd].TrapRiseTime[iCh];
    builder << "TrapRiseTime" << arrTrapRiseTime;

    auto arrPeakingTime = bsoncxx::builder::stream::array{};
    for (auto iCh = 0; iCh < MAX_CHANNELS; iCh++)
      arrPeakingTime << fParameters[iBrd].PeakingTime[iCh];
    builder << "PeakingTime" << arrPeakingTime;

    auto arrTTFSmoothing = bsoncxx::builder::stream::array{};
    for (auto iCh = 0; iCh < MAX_CHANNELS; iCh++)
      arrTTFSmoothing << fParameters[iBrd].TTFSmoothing[iCh];
    builder << "TTFSmoothing" << arrTTFSmoothing;

    auto arrSignalRiseTime = bsoncxx::builder::stream::array{};
    for (auto iCh = 0; iCh < MAX_CHANNELS; iCh++)
      arrSignalRiseTime << fParameters[iBrd].SignalRiseTime[iCh];
    builder << "SignalRiseTime" << arrSignalRiseTime;

    auto arrNSPeak = bsoncxx::builder::stream::array{};
    for (auto iCh = 0; iCh < MAX_CHANNELS; iCh++)
      arrNSPeak << fParameters[iBrd].NSPeak[iCh];
    builder << "NSPeak" << arrNSPeak;

    auto arrPeakHoldOff = bsoncxx::builder::stream::array{};
    for (auto iCh = 0; iCh < MAX_CHANNELS; iCh++)
      arrPeakHoldOff << fParameters[iBrd].PeakHoldOff[iCh];
    builder << "PeakHoldOff" << arrPeakHoldOff;

    auto arrBaseLineHoldOff = bsoncxx::builder::stream::array{};
    for (auto iCh = 0; iCh < MAX_CHANNELS; iCh++)
      arrBaseLineHoldOff << fParameters[iBrd].BaseLineHoldOff[iCh];
    builder << "BaseLineHoldOff" << arrBaseLineHoldOff;

    auto arrZeroCrossAccWindow = bsoncxx::builder::stream::array{};
    for (auto iCh = 0; iCh < MAX_CHANNELS; iCh++)
      arrZeroCrossAccWindow << fParameters[iBrd].ZeroCrossAccWindow[iCh];
    builder << "ZeroCrossAccWindow" << arrZeroCrossAccWindow;

    auto arrTrgAccWindow = bsoncxx::builder::stream::array{};
    for (auto iCh = 0; iCh < MAX_CHANNELS; iCh++)
      arrTrgAccWindow << fParameters[iBrd].TrgAccWindow[iCh];
    builder << "TrgAccWindow" << arrTrgAccWindow;

    auto arrDigitalGain = bsoncxx::builder::stream::array{};
    for (auto iCh = 0; iCh < MAX_CHANNELS; iCh++)
      arrDigitalGain << fParameters[iBrd].DigitalGain[iCh];
    builder << "DigitalGain" << arrDigitalGain;

    auto arrEneFineGain = bsoncxx::builder::stream::array{};
    for (auto iCh = 0; iCh < MAX_CHANNELS; iCh++)
      arrEneFineGain << fParameters[iBrd].EneFineGain[iCh];
    builder << "EneFineGain" << arrEneFineGain;

    auto arrDecimation = bsoncxx::builder::stream::array{};
    for (auto iCh = 0; iCh < MAX_CHANNELS; iCh++)
      arrDecimation << fParameters[iBrd].Decimation[iCh];
    builder << "Decimation" << arrDecimation;

    builder << bsoncxx::builder::stream::close_document;
  }

  bsoncxx::document::value doc = builder << bsoncxx::builder::stream::finalize;
  return doc;
}

void TParHandler::DownloadParameters()
{
  auto conn = fPool.acquire();
  auto collection = (*conn)["ELIADE"]["Digitizer"];

  auto order = bsoncxx::builder::stream::document{}
               << "Time" << -1 << bsoncxx::builder::stream::finalize;
  auto opts = mongocxx::options::find{};
  opts.sort(order.view());
  auto doc = collection.find_one({}, opts);

  fParameters.clear();
  for (auto iBrd = 0; iBrd < 16; iBrd++) {  // 16 has no reason
    auto digiName = "Digitizer" + std::to_string(iBrd);
    auto digitizer = doc->view()[digiName];
    if (digitizer.length() == 0) continue;

    DigiPar par;
    par.LinkType = (CAEN_DGTZ_ConnectionType)GetInt32(digitizer, "LinkType");

    par.LinkNum = GetInt32(digitizer, "LinkNum");

    par.ConetNode = GetInt32(digitizer, "ConetNode");

    par.VMEBaseAddress = GetInt32(digitizer, "VMEBaseAddress");

    par.BrdNum = GetInt32(digitizer, "BrdNum");

    par.RunSyncMode =
        (CAEN_DGTZ_RunSyncMode_t)GetInt32(digitizer, "RunSyncMode");

    par.StartMode = (CAEN_DGTZ_AcqMode_t)GetInt32(digitizer, "StartMode");

    par.RunSync = (SyncType)GetInt32(digitizer, "RunSync");

    par.ChMask = GetInt32(digitizer, "ChMask");

    for (auto iCh = 0; iCh < MAX_CHANNELS; iCh++)
      par.DCOffset[iCh] = GetInt32(digitizer, "DCOffset", iCh);

    for (auto iCh = 0; iCh < MAX_CHANNELS; iCh++)
      par.Polarity[iCh] =
          (CAEN_DGTZ_PulsePolarity_t)GetInt32(digitizer, "Polarity", iCh);

    for (auto iCh = 0; iCh < MAX_CHANNELS; iCh++)
      par.TrgPolarity[iCh] =
          (CAEN_DGTZ_TriggerPolarity_t)GetInt32(digitizer, "TrgPolarity", iCh);

    for (auto iCh = 0; iCh < MAX_CHANNELS; iCh++)
      par.ChargeSens[iCh] = GetInt32(digitizer, "ChargeSens", iCh);

    par.SWTrgMode = (CAEN_DGTZ_TriggerMode_t)GetInt32(digitizer, "SWTrgMode");

    par.ExtTrgMode = (CAEN_DGTZ_TriggerMode_t)GetInt32(digitizer, "ExtTrgMode");

    par.ChSelfTrg = (CAEN_DGTZ_TriggerMode_t)GetInt32(digitizer, "ChSelfTrg");

    par.TrgPropagation = GetBool(digitizer, "TrgPropagation");

    for (auto iCh = 0; iCh < MAX_CHANNELS; iCh++)
      par.TrgTh[iCh] = GetInt32(digitizer, "TrgTh", iCh);

    for (auto iCh = 0; iCh < MAX_CHANNELS; iCh++)
      par.TrgHoldOff[iCh] = GetInt32(digitizer, "TrgHoldOff", iCh);

    for (auto iCh = 0; iCh < MAX_CHANNELS; iCh++)
      par.ShortGate[iCh] = GetInt32(digitizer, "ShortGate", iCh);

    for (auto iCh = 0; iCh < MAX_CHANNELS; iCh++)
      par.LongGate[iCh] = GetInt32(digitizer, "LongGate", iCh);

    for (auto iCh = 0; iCh < MAX_CHANNELS; iCh++)
      par.CoincWindow[iCh] = GetInt32(digitizer, "CoincWindow", iCh);

    for (auto iCh = 0; iCh < MAX_CHANNELS; iCh++)
      par.NSBaseLine[iCh] = GetInt32(digitizer, "NSBaseLine", iCh);

    par.PileUpRejection =
        (CAEN_DGTZ_DPP_PUR_t)GetInt32(digitizer, "PileUpRejection");

    par.PURGap = GetInt32(digitizer, "PURGap");

    for (auto iCh = 0; iCh < MAX_CHANNELS; iCh++)
      par.DiscrMode[iCh] = GetInt32(digitizer, "DiscrMode", iCh);

    for (auto iCh = 0; iCh < MAX_CHANNELS; iCh++)
      par.CFDDelay[iCh] = GetInt32(digitizer, "CFDDelay", iCh);

    for (auto iCh = 0; iCh < MAX_CHANNELS; iCh++)
      par.CFDFraction[iCh] = GetInt32(digitizer, "CFDFraction", iCh);

    for (auto iCh = 0; iCh < MAX_CHANNELS; iCh++)
      par.RecordLength[iCh] = GetInt32(digitizer, "RecordLength", iCh);

    for (auto iCh = 0; iCh < MAX_CHANNELS; iCh++)
      par.PreTriggerSize[iCh] = GetInt32(digitizer, "PreTriggerSize", iCh);

    par.IOLevel = (CAEN_DGTZ_IOLevel_t)GetInt32(digitizer, "IOLevel");

    par.AcqMode = (CAEN_DGTZ_DPP_AcqMode_t)GetInt32(digitizer, "AcqMode");

    par.Coincidence =
        (CAEN_DGTZ_DPP_TriggerMode_t)GetInt32(digitizer, "Coincidence");

    par.EventBuffering = GetInt32(digitizer, "EventBuffering");

    for (auto iCh = 0; iCh < MAX_CHANNELS; iCh++)
      par.DynamicRange[iCh] = GetInt32(digitizer, "DynamicRange", iCh);

    for (auto iCh = 0; iCh < MAX_CHANNELS; iCh++)
      par.TrapPoleZero[iCh] = GetInt32(digitizer, "TrapPoleZero", iCh);

    for (auto iCh = 0; iCh < MAX_CHANNELS; iCh++)
      par.TrapFlatTop[iCh] = GetInt32(digitizer, "TrapFlatTop", iCh);

    for (auto iCh = 0; iCh < MAX_CHANNELS; iCh++)
      par.TrapRiseTime[iCh] = GetInt32(digitizer, "TrapRiseTime", iCh);

    for (auto iCh = 0; iCh < MAX_CHANNELS; iCh++)
      par.PeakingTime[iCh] = GetInt32(digitizer, "PeakingTime", iCh);

    for (auto iCh = 0; iCh < MAX_CHANNELS; iCh++)
      par.TTFSmoothing[iCh] = GetInt32(digitizer, "TTFSmoothing", iCh);

    for (auto iCh = 0; iCh < MAX_CHANNELS; iCh++)
      par.SignalRiseTime[iCh] = GetInt32(digitizer, "SignalRiseTime", iCh);

    for (auto iCh = 0; iCh < MAX_CHANNELS; iCh++)
      par.NSPeak[iCh] = GetInt32(digitizer, "NSPeak", iCh);

    for (auto iCh = 0; iCh < MAX_CHANNELS; iCh++)
      par.PeakHoldOff[iCh] = GetInt32(digitizer, "PeakHoldOff", iCh);

    for (auto iCh = 0; iCh < MAX_CHANNELS; iCh++)
      par.BaseLineHoldOff[iCh] = GetInt32(digitizer, "BaseLineHoldOff", iCh);

    for (auto iCh = 0; iCh < MAX_CHANNELS; iCh++)
      par.ZeroCrossAccWindow[iCh] =
          GetInt32(digitizer, "ZeroCrossAccWindow", iCh);

    for (auto iCh = 0; iCh < MAX_CHANNELS; iCh++)
      par.TrgAccWindow[iCh] = GetInt32(digitizer, "TrgAccWindow", iCh);

    for (auto iCh = 0; iCh < MAX_CHANNELS; iCh++)
      par.DigitalGain[iCh] = GetInt32(digitizer, "DigitalGain", iCh);

    for (auto iCh = 0; iCh < MAX_CHANNELS; iCh++)
      par.EneFineGain[iCh] = GetDouble(digitizer, "EneFineGain", iCh);

    for (auto iCh = 0; iCh < MAX_CHANNELS; iCh++)
      par.Decimation[iCh] = GetInt32(digitizer, "Decimation", iCh);

    fParameters.push_back(par);
  }
}

bool TParHandler::GetBool(bsoncxx::document::element &ele, std::string eleName,
                          int32_t ch)
{
  bool retVal = 0;
  try {
    if (ch < 0)
      retVal = ele[eleName].get_bool().value;
    else
      retVal = ele[eleName][ch].get_bool().value;
  } catch (bsoncxx::exception e) {
    std::cout << "ERROR: " << e.what() << " for " << eleName << std::endl;
  }

  return retVal;
}

int32_t TParHandler::GetInt32(bsoncxx::document::element &ele,
                              std::string eleName, int32_t ch)
{
  int32_t retVal = 0;
  try {
    if (ch < 0)
      retVal = ele[eleName].get_int32().value;
    else
      retVal = ele[eleName][ch].get_int32().value;
  } catch (bsoncxx::exception e) {
    std::cout << "ERROR: " << e.what() << " for " << eleName << std::endl;
  }

  return retVal;
}

double TParHandler::GetDouble(bsoncxx::document::element &ele,
                              std::string eleName, int32_t ch)
{
  double retVal = 0.;
  try {
    if (ch < 0)
      retVal = ele[eleName].get_double().value;
    else
      retVal = ele[eleName][ch].get_double().value;
  } catch (bsoncxx::exception e) {
    std::cout << "ERROR: " << e.what() << " for " << eleName << std::endl;
  }

  return retVal;
}
