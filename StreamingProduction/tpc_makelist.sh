#!/usr/bin/bash

if [ $# -eq 0 ]
then
  echo "No runnumber supplied"
  exit 0
fi

if [ $# -eq 1 ]
then
  echo "No type or runnumber supplied"
  exit 0
fi

type=$1
runnumber=$(printf "%08d" $2)

for i in {0..23}
do
    for j in {0..1}
    do
      ebdc=$(printf "%02d_%01d" $i $j)
      /bin/ls -1 /sphenix/lustre01/sphnxpro/physics/tpc/${type}/TPC_ebdc${ebdc}*-${runnumber}-* > data/tpc-${runnumber}-${ebdc}.list
      if [ ! -s data/tpc-${runnumber}-${ebdc}.list ]
      then
          echo data/tpc-${runnumber}-${ebdc}.list empty, removing it
          rm  data/tpc-${runnumber}-${ebdc}.list
      else
        echo "Created data/tpc-${runnumber}-${ebdc}.list with contents:"
        wc -l data/tpc-${runnumber}-${ebdc}.list
      fi
    done
done
