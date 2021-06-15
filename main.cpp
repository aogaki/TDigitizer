#include <TApplication.h>
#include <TCanvas.h>
#include <TGraph.h>
#include <TH1D.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>

#include <memory>

#include "TPHA.hpp"
#include "TPSD.hpp"
#include "TParHandler.hpp"
#include "TWaveform.hpp"

int InputCheck(void)
{
  struct termios oldt, newt;
  int ch;
  int oldf;

  tcgetattr(STDIN_FILENO, &oldt);
  newt = oldt;
  newt.c_lflag &= ~(ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSANOW, &newt);
  oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
  fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

  ch = getchar();

  tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
  fcntl(STDIN_FILENO, F_SETFL, oldf);

  if (ch != EOF) {
    ungetc(ch, stdin);
    return 1;
  }

  return 0;
}

int main()
{
  TParHandler parHandler;
  parHandler.DownloadParameters();

  auto par = parHandler.GetParameters();
  std::cout << par.size() << std::endl;

  return 0;
}

// int main(int argc, char *argv[])
// {
//   std::unique_ptr<TApplication> app(new TApplication("app", &argc, argv));
//
//   std::unique_ptr<TCanvas> canvGr(new TCanvas());
//   std::unique_ptr<TGraph> gr1(new TGraph());
//   gr1->SetMinimum(0);
//   gr1->SetMaximum(18000);
//   std::unique_ptr<TGraph> gr2(new TGraph());
//   gr2->SetLineColor(kBlue);
//
//   std::unique_ptr<TCanvas> canvHist(new TCanvas());
//   std::unique_ptr<TH1D> hist(new TH1D("hist", "test", 30000, 0.5, 30000.5));
//
//   // std::unique_ptr<TDigitizer> digitizer{new TPSD()};
//   std::unique_ptr<TDigitizer> digitizer{new TPHA()};
//   // std::unique_ptr<TDigitizer> digitizer{new TWaveform()};
//   DigiPar par;
//   digitizer->LoadPar(par);
//   digitizer->Open();
//   digitizer->Reset();
//
//   digitizer->GetInfo();
//   digitizer->Config();
//   // digitizer->EnableFineTS();
//   digitizer->AllocateMemory();
//
//   digitizer->Start();
//
//   while (true) {
//     if (InputCheck()) break;
//     usleep(1000);
//
//     auto dataVec = digitizer->GetEvents();
//     if (dataVec->size() > 0) {
//       auto nPoints = dataVec->at(0)->RecordLength;
//       for (auto iData = 0; iData < nPoints; iData++) {
//         gr1->SetPoint(iData, iData * 4, dataVec->at(0)->Trace1[iData]);
//         gr2->SetPoint(iData, iData * 4,
//                       (dataVec->at(0)->Trace2[iData] / 4) + 2000);
//       }
//
//       for (auto &&data : *dataVec) hist->Fill(data->ChargeLong);
//
//       canvGr->cd();
//       gr1->Draw("AL");
//       gr2->Draw("SAME");
//       canvGr->Modified();
//       canvGr->Update();
//
//       canvHist->cd();
//       hist->Draw();
//       canvHist->Modified();
//       canvHist->Update();
//
//       // std::cout << dataVec->at(0)->ChargeLong << std::endl;
//     }
//   }
//
//   digitizer->Stop();
//
//   digitizer->Reset();
//   digitizer->FreeMemory();
//
//   digitizer->Close();
//
//   app->Run();
//
//   return 0;
// }
