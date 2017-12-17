#!/bin/bash

INS="${HOME}/Github/FindingHavoc/collectingProfiles/instrumentor/build/instrumentor"
CLA=$1
HEADER_INCLUDE="${HOME}/Install/llvm.release.install/lib/clang/6.0.0/include/"
DEFINES="-DNDEBUG  -O3 -DNDEBUG   -w -Werror=date-time -DSPEC_CPU -DSPEC_CPU_LINUX -DSPEC_CPU_X64 -DSPEC_CPU_LITTLEENDIAN -DSPEC_CPU_LP64"
#DEFINES=
if [ "${CLA}" == "--file"  ] ; then
  file=$2
  ${INS} $file -- -I ${HEADER_INCLUDE} > $file.tmp;
  mv $file.tmp $file;
  exit;
fi

for file in $(find . -name "*.c" -o -name "*.cc" -o -name "*.cpp"); do
  if [ "${CLA}" == "--echo" ] ; then
    echo
    echo "Processing $file";
    echo "${INS} $file -- -I ${HEADER_INCLUDE}";
    echo mv $file.tmp $file;
    continue;
  fi
  ${INS} $file -- -I ${HEADER_INCLUDE} ${DEFINES} > $file.tmp;
  mv $file.tmp $file;
done
