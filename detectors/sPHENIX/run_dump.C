#pragma once

#include <nodedump/Dumper.h>
#include <fun4all/Fun4AllServer.h>
#include <fun4all/Fun4AllDstInputManager.h>

R__LOAD_LIBRARY(libfun4all.so)
R__LOAD_LIBRARY(libphnodedump.so)

void run_dump(const char *infile, const int evts=100)
{
  gSystem->Load("libfun4all.so");
  gSystem->Load("libphnodedump.so");
  gSystem->Load("libg4dst.so");
  Fun4AllServer* se = Fun4AllServer::instance();

  Dumper *dmp = new Dumper();
  gSystem->Exec("mkdir /phenix/scratch/pinkenbu/g4dump");
  dmp->SetOutDir("/phenix/scratch/pinkenbu/g4dump");

  se->registerSubsystem(dmp);

  Fun4AllInputManager *in = new Fun4AllDstInputManager("DSTin");
  se->registerInputManager(in);
  se->fileopen("DSTin",infile);
  se->run(evts);
  se->End();
  delete se;
}
