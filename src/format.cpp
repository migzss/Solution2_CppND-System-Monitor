#include <iostream>
#include <sstream>
#include <string>

#include "format.h"

using std::string;

string Format::ElapsedTime(long totalSeconds) {
  int hours = totalSeconds / 3600;
  int minutes = (totalSeconds % 3600) / 60;
  int seconds = (totalSeconds % 3600) % 60;

  std::string hourString =
      (hours < 10) ? "0" + std::to_string(hours) : std::to_string(hours);

  std::string minuteString =
      (minutes < 10) ? "0" + std::to_string(minutes) : std::to_string(minutes);

  std::string secondString =
      (seconds < 10) ? "0" + std::to_string(seconds) : std::to_string(seconds);

  std::ostringstream stringStream;
  stringStream << hourString << ":" << minuteString << ":" << secondString;

  return stringStream.str();
}