#!/bin/bash

INCLUDE="/home/sdasgup3/Github/FindingHavoc/collectingProfiles/instrumentor/include/"
LIB="/home/sdasgup3/Github/FindingHavoc/collectingProfiles/instrumentor/build/"
gcc -std=c++11 -I ${INCLUDE} -L ${LIB}  $@ -linstrumentor
