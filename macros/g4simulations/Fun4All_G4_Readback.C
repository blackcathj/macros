// $Id: $

/*!
 * \file Fun4All_Readback.C
 * \brief 
 * \author Jin Huang <jhuang@bnl.gov>
 * \version $Revision:   $
 * \date $Date: $
 */

#if ROOT_VERSION_CODE >= ROOT_VERSION(6, 00, 0)

//#include <anatutorial/AnaTutorial.h>
#include <g4eval/JetEvaluator.h>
#include <g4eval/SvtxEvaluator.h>

#include <qa_modules/QAG4SimulationCalorimeter.h>
#include <qa_modules/QAG4SimulationCalorimeterSum.h>
#include <qa_modules/QAG4SimulationJet.h>
#include <qa_modules/QAG4SimulationTracking.h>
#include <qa_modules/QAHistManagerDef.h>

#include <fun4all/Fun4AllDstInputManager.h>
#include <fun4all/Fun4AllDstOutputManager.h>
#include <fun4all/Fun4AllDummyInputManager.h>
#include <fun4all/Fun4AllInputManager.h>
#include <fun4all/Fun4AllNoSyncDstInputManager.h>
#include <fun4all/Fun4AllOutputManager.h>
#include <fun4all/Fun4AllServer.h>
#include <fun4all/SubsysReco.h>

#include <phhepmc/Fun4AllHepMCInputManager.h>

#include <hfjettruthgeneration/HFJetTruthTrigger.h>

//R__LOAD_LIBRARY(libanatutorial.so)
R__LOAD_LIBRARY(libfun4all.so)
R__LOAD_LIBRARY(libg4dst.so)
R__LOAD_LIBRARY(libg4eval.so)
R__LOAD_LIBRARY(libqa_modules.so)
R__LOAD_LIBRARY(libHFJetTruthGeneration.so)

#endif

using namespace std;

void Fun4All_G4_Readback(
    const int nEvents = 0,
    const char *inputFile = "/phenix/u/jinhuang/work/sPHENIX/simulation//test/HF-production-bjet-pp200-reco.50.lst",
    const char *outputFile = "G4sPHENIX_G4_Readback.root")
{
  //---------------
  // Load libraries
  //---------------

  gSystem->Load("libfun4all.so");
  gSystem->Load("libg4dst.so");
  gSystem->Load("libqa_modules");

  //---------------
  // Fun4All server
  //---------------

  Fun4AllServer *se = Fun4AllServer::instance();
  se->Verbosity(01);

  //--------------
  // IO management
  //--------------
  // Hits file
  Fun4AllInputManager *hitsin = new Fun4AllDstInputManager("DSTin");
  hitsin->AddListFile(inputFile);
  se->registerInputManager(hitsin);

  {
    const int n_maps_layer = 3;  // must be 0-3, setting it to zero removes Mvtx completely, n < 3 gives the first n layers

    int n_intt_layer = 4;  // must be 4 or 0, setting to zero removes INTT completely

    int n_tpc_layer_inner = 16;
    int tpc_layer_rphi_count_inner = 1152;
    int n_tpc_layer_mid = 16;
    int n_tpc_layer_outer = 16;
    int n_gas_layer = n_tpc_layer_inner + n_tpc_layer_mid + n_tpc_layer_outer;

    const bool g4eval_use_initial_vertex = false;  // if true, g4eval uses initial vertices in SvtxVertexMap, not final vertices in SvtxVertexMapRefit

    //----------------
    // Tracking evaluation
    //----------------
    SvtxEvaluator *eval;
    eval = new SvtxEvaluator("SVTXEVALUATOR", string(outputFile) + "_SvtxEvaluator.root", "SvtxTrackMap", n_maps_layer, n_intt_layer, n_gas_layer);
    eval->do_cluster_eval(false);
    eval->do_g4hit_eval(false);
    eval->do_hit_eval(false);  // enable to see the hits that includes the chamber physics...
    eval->do_gpoint_eval(false);
    eval->do_eval_light(true);
    eval->set_use_initial_vertex(g4eval_use_initial_vertex);
    eval->scan_for_embedded(false);  // take all tracks if false - take only embedded tracks if true
    eval->Verbosity(0);
//    se->registerSubsystem(eval);
  }

  {
    JetEvaluator *eval = new JetEvaluator("JETEVALUATOR",
                                          "AntiKt_Tower_r04",
                                          "AntiKt_Truth_r04",
                                          string(outputFile) + "_JetEvaluator.root");
    se->registerSubsystem(eval);
  }

  // HF jet trigger moudle
  // build https://github.com/sPHENIX-Collaboration/analysis/tree/master/HF-Jet/TruthGeneration locally
  assert(gSystem->Load("libHFJetTruthGeneration") == 0);
  {
    HFJetTruthTrigger *jt = new HFJetTruthTrigger(
        string(outputFile) + "_HFJetTruthTrigger.root", 5, "AntiKt_Truth_r04");
    //            jt->Verbosity(HFJetTruthTrigger::VERBOSITY_MORE);
    se->registerSubsystem(jt);
  }

  // QA parts
  {
    //  if (do_cemc)
    se->registerSubsystem(new QAG4SimulationCalorimeter("CEMC",
                                                        static_cast<QAG4SimulationCalorimeter::enu_flags>(QAG4SimulationCalorimeter::kProcessTower | QAG4SimulationCalorimeter::kProcessCluster)));
    //  if (do_hcalin)
    se->registerSubsystem(new QAG4SimulationCalorimeter("HCALIN",
                                                        static_cast<QAG4SimulationCalorimeter::enu_flags>(QAG4SimulationCalorimeter::kProcessTower | QAG4SimulationCalorimeter::kProcessCluster)));

    //  if (do_hcalout)
    se->registerSubsystem(new QAG4SimulationCalorimeter("HCALOUT",
                                                        static_cast<QAG4SimulationCalorimeter::enu_flags>(QAG4SimulationCalorimeter::kProcessTower | QAG4SimulationCalorimeter::kProcessCluster)));

    //  if (do_tracking && do_cemc && do_hcalin && do_hcalout)
    {
      QAG4SimulationCalorimeterSum *calo_qa =
          new QAG4SimulationCalorimeterSum();
      //    calo_qa->Verbosity(10);
      se->registerSubsystem(calo_qa);
    }

    //  if (do_jet_reco)
    {
      QAG4SimulationJet *calo_jet7 = new QAG4SimulationJet(
          "AntiKt_Truth_r07");
      calo_jet7->add_reco_jet("AntiKt_Tower_r07");
      calo_jet7->add_reco_jet("AntiKt_Cluster_r07");
      calo_jet7->add_reco_jet("AntiKt_Track_r07");
      //    calo_jet7->Verbosity(20);
      se->registerSubsystem(calo_jet7);

      QAG4SimulationJet *calo_jet4 = new QAG4SimulationJet(
          "AntiKt_Truth_r04");
      calo_jet4->add_reco_jet("AntiKt_Tower_r04");
      calo_jet4->add_reco_jet("AntiKt_Cluster_r04");
      calo_jet4->add_reco_jet("AntiKt_Track_r04");
      se->registerSubsystem(calo_jet4);

      QAG4SimulationJet *calo_jet2 = new QAG4SimulationJet(
          "AntiKt_Truth_r02");
      calo_jet2->add_reco_jet("AntiKt_Tower_r02");
      calo_jet2->add_reco_jet("AntiKt_Cluster_r02");
      calo_jet2->add_reco_jet("AntiKt_Track_r02");
      se->registerSubsystem(calo_jet2);
    }

    //  if (do_tracking)
    {
      QAG4SimulationTracking *qa = new QAG4SimulationTracking();
      se->registerSubsystem(qa);
    }
  }

  if (nEvents < 0)
  {
    return 0;
  }

  se->run(nEvents);
  // se->run(0);

  // QA outputs
  {
    QAHistManagerDef::saveQARootFile(string(outputFile) + "_qa.root");
  }
  //-----
  // Exit
  //-----

  se->End();
  std::cout << "All done" << std::endl;
  delete se;
  gSystem->Exit(0);
  return 0;
}
