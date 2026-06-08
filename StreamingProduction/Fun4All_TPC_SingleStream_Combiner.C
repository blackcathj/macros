#include <QA.C>

#include <fun4all/Fun4AllDstOutputManager.h>
#include <fun4all/Fun4AllInputManager.h>
#include <fun4all/Fun4AllOutputManager.h>
#include <fun4all/PHTFileServer.h>
#include <fun4all/Fun4AllServer.h>
#include <fun4allraw/Fun4AllStreamingInputManager.h>
#include <fun4allraw/InputManagerType.h>
#include <fun4allraw/SingleGl1PoolInput.h>
#include <fun4allraw/SingleInttPoolInput.h>
#include <fun4allraw/SingleMicromegasPoolInput.h>
#include <fun4allraw/SingleMvtxPoolInput.h>
#include <fun4allraw/SingleTpcPoolInput.h>
#include <fun4allraw/SingleTpcTimeFrameInput.h>

#include <phool/recoConsts.h>

#include <ffarawmodules/InttCheck.h>
#include <ffarawmodules/StreamingCheck.h>
#include <ffarawmodules/TpcCheck.h>

#include <ffamodules/FlagHandler.h>
#include <ffamodules/HeadReco.h>
#include <ffamodules/SyncReco.h>

R__LOAD_LIBRARY(libfun4all.so)
R__LOAD_LIBRARY(libffamodules.so)
R__LOAD_LIBRARY(libfun4allraw.so)
R__LOAD_LIBRARY(libffarawmodules.so)

bool isGood(const string &infile); 

// void Fun4All_TPC_SingleStream_Combiner(int nEvents = 5,
//                                        const int runnumber = 68424,
//                                        const string &outdir = "./data",
//                                        const string &type = "streaming",
//                                        const string &input_gl1file = "data/gl1daq-00068424.list",
//                                        const string &input_tpcfile00 = "data/tpc-00068424-00_0.list")
// void Fun4All_TPC_SingleStream_Combiner(int nEvents = 100000,
//                                        const int runnumber = 69260,
//                                        const string &outdir = "./data",
//                                        const string &type = "streaming",
//                                        const string &input_gl1file = "data/gl1daq-00069260.list",
//                                        const string &input_tpcfile00 = "data/tpc-00069260-05_1.list")
// void Fun4All_TPC_SingleStream_Combiner(int nEvents = 100,
//                                        const int runnumber = 69413,
//                                        const string &outdir = "./data",
//                                        const string &type = "streaming",
//                                        const string &input_gl1file = "data/gl1daq-00069413.list",
//                                        const string &input_tpcfile00 = "data/tpc-00069413-17_1.list")
void Fun4All_TPC_SingleStream_Combiner(int nEvents = 5e5,
                                       const int runnumber = 79523,
                                       const string &outdir = "./data",
                                       const string &type = "streaming",
                                       const string &input_gl1file = "data/gl1daq-00079523.list",
                                       const string &input_tpcfile00 = "data/tpc-00079523-13_0.list")
{
  // GL1 which provides the beam clock reference (if we ran with GL1)
  vector<string> gl1_infile;
  gl1_infile.push_back(input_gl1file);

  vector<string> tpc_infile;
  tpc_infile.push_back(input_tpcfile00);

  Fun4AllServer *se = Fun4AllServer::instance();
  se->Verbosity(1);
  recoConsts *rc = recoConsts::instance();
  // rc -> set_StringFlag("CDB_GLOBALTAG", "ProdA_2024"); // needed for QA output
  rc -> set_StringFlag("CDB_GLOBALTAG", "newcdbtag");
  rc -> set_IntFlag("RUNNUMBER", runnumber); // needed only during testing
  rc->set_uint64Flag("TIMESTAMP",runnumber);
  Fun4AllStreamingInputManager *in = new Fun4AllStreamingInputManager("Comb");
  //  in->Verbosity(3);

  // create and register input managers
  int i = 0;

  std::string readoutNumber = "";

  for (auto iter : gl1_infile)
  {
    if (isGood(iter))
    {
      SingleGl1PoolInput *gl1_sngl = new SingleGl1PoolInput("GL1_" + to_string(i));
      //    gl1_sngl->Verbosity(3);
      gl1_sngl->AddListFile(iter);
      in->registerStreamingInput(gl1_sngl, InputManagerType::GL1);
      i++;
    }
  }
  i = 0;

  i = 0;
  for (auto iter : tpc_infile)
  {
    if (isGood(iter))
    {
      /// find the ebdc number from the filename
      std::string filepath, ebdc;
      std::ifstream ifs(iter);
      while (std::getline(ifs, filepath))
      {
        auto pos = filepath.find("ebdc");
        ebdc = filepath.substr(pos + 4, 2);
        break;
      }
      readoutNumber = "TPC" + ebdc;

      // SingleTpcPoolInput *tpc_sngl = new SingleTpcPoolInput("TPC_" + to_string(i));
      //    tpc_sngl->Verbosity(0);
      // //   tpc_sngl->DryRun();
      // tpc_sngl->SetBcoRange(5);
      // tpc_sngl->setHitContainerName("TPCRAWHIT_" + ebdc);
      // tpc_sngl->AddListFile(iter);
      // in->registerStreamingInput(tpc_sngl, InputManagerType::TPC);

      SingleTpcTimeFrameInput *tpc_sngl = new SingleTpcTimeFrameInput("SingleTpcTimeFrameInput_" + to_string(i));
      tpc_sngl->setHitContainerName("TPCRAWHIT_" + ebdc);
      tpc_sngl->AddListFile(iter);
      tpc_sngl->setDigitalCurrentDebugTTreeName(iter + "_DigitalCurrentDebugTTree.root");
      tpc_sngl->Verbosity(1);
      // tpc_sngl->AddPacketID(4180);

      in->registerStreamingInput(tpc_sngl, InputManagerType::TPC);
      
      i++;
    }
  }
  i = 0;

  se->registerInputManager(in);
  // in->Verbosity(1);
  in->Print("ALL");

  // StreamingCheck *scheck = new StreamingCheck();
  // scheck->SetTpcBcoRange(130);
  // se->registerSubsystem(scheck);
  // TpcCheck *tpccheck = new TpcCheck();
  // tpccheck->Verbosity(3);
  // tpccheck->SetBcoRange(130);
  // se->registerSubsystem(tpccheck);
  SyncReco *sync = new SyncReco();
  se->registerSubsystem(sync);

  HeadReco *head = new HeadReco();
  se->registerSubsystem(head);

  FlagHandler *flag = new FlagHandler();
  se->registerSubsystem(flag);

  char outfile[500];  
  sprintf(outfile, "%s/DST_%s-%s.root", outdir.c_str(), type.c_str(), readoutNumber.c_str());
  // sprintf(outfile, "%s/%s-%s.root",  outdir.c_str(), type.c_str(),readoutNumber.c_str());

  Fun4AllOutputManager *out = new Fun4AllDstOutputManager("out", outfile);
  out->UseFileRule();
  out->SetNEvents(100000);
  se->registerOutputManager(out);

  gSystem->ListLibraries();

  if (nEvents < 0)
  {
    return;
  }
  se->run(nEvents);

  se->End();
  se->PrintTimer();
  in->Print("ALL");

  char histoutfile[500];
  sprintf(histoutfile, "%s/HIST_%s-%s-%08i-%05i.root", outdir.c_str(), type.c_str(), readoutNumber.c_str(), runnumber, 0);
  QAHistManagerDef::saveQARootFile(histoutfile);

  delete se;
  PHTFileServer::close();
  cout << "all done" << endl;
  gSystem->Exit(0);
}

bool isGood(const string &infile)
{
  ifstream intest;
  intest.open(infile);
  bool goodfile = false;
  if (intest.is_open())
  {
    if (intest.peek() != std::ifstream::traits_type::eof())  // is it non zero?
    {
      goodfile = true;
    }
    intest.close();
  }
  return goodfile;
}
