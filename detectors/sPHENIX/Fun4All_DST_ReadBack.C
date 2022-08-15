// $Id: $

/*!
 * \file Fun4All_DST_ReadBack.C
 * \brief 
 * \author Jin Huang <jhuang@bnl.gov>
 * \version $Revision:   $
 * \date $Date: $
 */

#include <fun4all/Fun4AllDstInputManager.h>
#include <fun4all/Fun4AllInputManager.h>
#include <fun4all/Fun4AllServer.h>
#include <fun4all/SubsysReco.h>

#include <g4eval/PHG4DSTReader.h>
#include <TSystem.h>
#include <phool/recoConsts.h>


R__LOAD_LIBRARY(libfun4all.so)
R__LOAD_LIBRARY(libg4eval.so)

// void Fun4All_DST_ReadBack(const int nevnt = 100, const char *inputfile = "data/MDC2_Run41_FTFP_BERT_HP.g4hits.list")
// void Fun4All_DST_ReadBack(const int nevnt = 100, const char *inputfile = "data/MDC2_Run40_FTFP_BERT.g4hits.list")
void Fun4All_DST_ReadBack(const int nevnt = 10000, const char *inputfile = "data/MDC2_pp_Run42_FTFP_BERT.g4hits.list")
{
  gSystem->Load("libg4dst");
  Fun4AllServer *se = Fun4AllServer::instance();
  se->Verbosity(1);
//  recoConsts *rc = recoConsts::instance();
//  rc->set_IntFlag("RUNNUMBER", 40);

  Fun4AllInputManager *in = new Fun4AllDstInputManager("DSTin");
  in->AddListFile(inputfile);
  se->registerInputManager(in);



  // save a comprehensive  evaluation file
  PHG4DSTReader *ana = new PHG4DSTReader(string(inputfile) + ".DSTReader.root");
  ana->set_save_particle(true);
  ana->set_load_all_particle(true);
  ana->set_load_active_particle(true);
  ana->set_save_vertex(true);
  ana->AddNode("TPC");
  se->registerSubsystem(ana);

  se->run(nevnt);
  se->End();
  delete se;
  gSystem->Exit(0);
}
