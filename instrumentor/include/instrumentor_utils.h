#ifndef INSTRUMENTATOR_UTILS
#define INSTRUMENTATOR_UTILS

#include <cassert>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <map>
#include <stack>

using namespace std;
using namespace std::chrono;

namespace instrumentor_utils {

class profileInfo {
public:
  string funcname;
  double elapsedTime;
  double childTime;

  profileInfo(const string &fn)
      : funcname(fn), elapsedTime(0.00), childTime(0.00) {}
  profileInfo() : elapsedTime(0.00), childTime(0.00) {}
};

class stackVal {
public:
  string funcname;
  std::chrono::high_resolution_clock::time_point start_timepoint;
  stackVal(const string &fn,
           const std::chrono::high_resolution_clock::time_point &tp)
      : funcname(fn), start_timepoint(tp) {}
};

typedef map<const string, profileInfo> funcProfMapType;
typedef stack<stackVal> callStackType;

class bookkeeper {
public:
  static funcProfMapType funcProfMap;
  static callStackType callStack;
};
};     // namespace instrumentor_utils
#endif // INSTRUMENTATOR_UTILS
