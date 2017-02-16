#!/bin/bash

export FORCE_COLOR=1
PAINT=`which paint`

while [ true ]
do
   echo
   echo
   echo --------------------------------------------
   date
   echo --------------------------------------------

   export BOOST_TEST_COLOR_OUTPUT=0

   make -s -C build/test all 2>&1 | head -15 | paint
   if [ ${PIPESTATUS[0]} -ne 0 ]
   then
      sleep 5
      echo --------------------------------------------
      continue
   fi

   make -s -C build  2>&1 | head -15 | paint
   if [ ${PIPESTATUS[0]} -ne 0 ]
   then
      sleep 5
      echo --------------------------------------------
      continue
   fi

   rm -f build/test/Testing/Temporary/LastTest.log
   make -s -C build/test test | paint
   if [ ${PIPESTATUS[0]} -ne 0 ]
   then
      echo
      echo --------------------------------------------

      grep -C3 -e Entering -e Leaving \
               -e context -e 'error: in ' ./build/test/Testing/Temporary/LastTest.log |
         sed -e 's,/.*/test/,,' |
         grep -e context -e 'error: in ' -C1 |
         paint

      sleep 5
      echo --------------------------------------------
      continue
   fi

   echo --------------------------------------------
   (
      cd ./build && ./myclient
      rm -f core.*
   )
   echo --------------------------------------------
   sleep 3;
done
