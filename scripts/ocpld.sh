#!/bin/bash

# OCPLD -- Ossim CoPy Library Dependencies
# Adapted from code written by Hemanth.HM

# Uncomment to step/debug
#set -x; trap read debug

function do_cpld {
    echo; echo "Scanning dependencies of $1"
    deps=$(ldd $1 | awk 'BEGIN{ORS=" "}$1~/^\//{print $1}$3~/^\//{print $3}' | sed 's/,$/\n/')

    for dep in $deps
    do
       b=$(basename $dep)
       if [ -e $2/$b ]; then
          echo "   $b already there."
       else
          echo "   copying $dep to $2"
          cp -n $dep $2
       fi
    done
}
export -f do_cpld

if [[ $# < 2 ]]; then
   s=$(basename $0)
   echo; echo "Usage: $s <path to OSSIM libraries> <destination path for dependencies>"; echo
   exit 1
fi

if [ ! -d $1 ]; then
   if [ ! -f $1 ] ; then
     echo; echo "<$1> is not a valid input directory or file. Aborting..."; echo
     exit 1
   fi
fi

if [ ! -d $2 ]; then
   echo; echo "Output directory <$2> does not exist. Creating..."; echo
   mkdir -p $2
fi

if [ -f $1 ] ; then
   do_cpld $1 $2
else
  find $1 -type f -name "*.so*" -exec bash -c "do_cpld {} $2" \;
fi
echo; echo "All dependencies were copied to $2. Done!"; echo