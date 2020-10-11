#include <string>
#include <sstream>
#include <iostream>
#include <iomanip>
#include "format.h"

std::string Format::ElapsedTime(long seconds) { 
  int hours, minutes;
  std::ostringstream out_stream;

  minutes = seconds / 60;
  hours = minutes / 60;

  out_stream << std::setfill('0') << std::setw(2) << hours << ":" 
             << std::setfill('0') << std::setw(2) << minutes%60 << ":" 
             << std::setfill('0') << std::setw(2) << seconds%60;

  return out_stream.str();
}