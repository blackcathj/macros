#!/usr/bin/bash

if [ $# -eq 0 ]
then
  echo "No type and runnumber supplied"
  exit 0
fi

if [ $# -eq 1 ]
then
  echo "No type or runnumber supplied"
  exit 0
fi

type=$1
runnumber=$(printf "%08d" $2)

/bin/ls -1 /sphenix/lustre01/sphnxpro/physics/GL1/${type}/GL1_*-${runnumber}-* > data/gl1daq-${runnumber}.list
if [ ! -s data/gl1daq-${runnumber}.list ]
then
  echo data/gl1daq-${runnumber}.list empty, removing it
  rm  data/gl1daq-${runnumber}.list
else
  echo "Created data/gl1daq-${runnumber}.list with contents:"
  cat data/gl1daq-${runnumber}.list
fi
