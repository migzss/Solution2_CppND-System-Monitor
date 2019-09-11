#include <dirent.h>
#include <unistd.h>
#include <string>
#include <vector>

#include "linux_parser.h"

using std::stof;
using std::string;
using std::to_string;
using std::vector;

// DONE: An example of how to read data from the filesystem
string LinuxParser::OperatingSystem() {
  string line;
  string key;
  string value;
  std::ifstream filestream(kOSPath);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::replace(line.begin(), line.end(), ' ', '_');
      std::replace(line.begin(), line.end(), '=', ' ');
      std::replace(line.begin(), line.end(), '"', ' ');
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "PRETTY_NAME") {
          std::replace(value.begin(), value.end(), '_', ' ');
          return value;
        }
      }
    }
  }
  return value;
}

// DONE: An example of how to read data from the filesystem
string LinuxParser::Kernel() {
  string os, versionText, kernel;
  string line;
  std::ifstream stream(kProcDirectory + kVersionFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> os >> versionText >> kernel;
  }
  return kernel;
}

// BONUS: Update this to use std::filesystem
vector<int> LinuxParser::Pids() {
  vector<int> pids;
  DIR* directory = opendir(kProcDirectory.c_str());
  struct dirent* file;
  while ((file = readdir(directory)) != nullptr) {
    // Is this a directory?
    if (file->d_type == DT_DIR) {
      // Is every character of the name a digit?
      string filename(file->d_name);
      if (std::all_of(filename.begin(), filename.end(), isdigit)) {
        int pid = stoi(filename);
        pids.push_back(pid);
      }
    }
  }
  closedir(directory);
  return pids;
}

// Read and return the system memory utilization
float LinuxParser::MemoryUtilization() {
  string line;
  float memutil = 0.f;
  float memTotal = 0.f;
  float memFree = 0.f;
  string key;
  string value;
  std::ifstream filestream(kProcDirectory + kMeminfoFilename);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::replace(line.begin(), line.end(), ':', ' ');
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "MemTotal") {
          memTotal = stof(value);
          break;
        } else if (key == "MemFree") {
          memFree = stof(value);
          break;
        }
      }

      if (memTotal > 0 && memFree > 0) break;
    }

    // memory usage
    memutil = (memTotal - memFree) / memTotal;
  }

  return memutil;
}

// Read and return the system uptime
long LinuxParser::UpTime() {
  std::string line, first, second;
  long uptime = 0;
  std::ifstream filestream(kProcDirectory + kUptimeFilename);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::istringstream linestream(line);
      while (linestream >> first >> second) {
        uptime = static_cast<long>(std::stof(first));
        break;
      }
    }
  }

  return uptime;
}

// Read and return the number of jiffies for the system
long LinuxParser::Jiffies() {
    vector<string> values = CpuUtilization();
    long result = ActiveJiffies() + IdleJiffies();

    return result;
}

// TODO: Read and return the number of active jiffies for a PID
// REMOVE: [[maybe_unused]] once you define the function
long LinuxParser::ActiveJiffies(int pid[[maybe_unused]]) { return 0; }

long LinuxParser::ActiveJiffies() {
    vector<string> values = CpuUtilization();

    return (stol(values[kUser_]) +
            stol(values[kNice_]) +
            stol(values[kSystem_]) +
            stol(values[kIRQ_]) +
            stol(values[kSoftIRQ_]) +
            stol(values[kSteal_]) +
            stol(values[kGuest_]) +
            stol(values[kGuestNice_]));
}

// Read and return the number of idle jiffies for the system
long LinuxParser::IdleJiffies() {
    vector<string> values = CpuUtilization();

    return (stol(values[kIdle_]) + stol(values[kIOwait_]));
}

// Read and return CPU utilization
vector<string> LinuxParser::CpuUtilization() {
    string line;
    string name = "cpu";
    std::ifstream filestream(kProcDirectory + kStatFilename);

    while (std::getline(filestream, line)) {
        if (line.compare(0, name.size(), name) == 0) {
            std::istringstream buf(line);
            std::istream_iterator<string> beg(buf), end;
            vector<string> values(++beg, end);

            return values;
        }
    }

    return {};
}

// Read and return the total number of processes
int LinuxParser::TotalProcesses() {
    string line;
    int result = 0;
    string name = "processes";
    std::ifstream filestream(kProcDirectory + kStatFilename);

    while (getline(filestream, line)) {
        if (line.compare(0, name.size(), name) == 0) {
            std::istringstream buf(line);
            std::istream_iterator<string> beg(buf), end;
            vector<string> values(beg, end);
            result += stoi(values[1]);
            break;
        }
    }

    return result;
}

//  Read and return the number of running processes
int LinuxParser::RunningProcesses() {
    string line;
    int result = 0;
    string name = "procs_running";
    std::ifstream filestream(kProcDirectory + kStatFilename);

    while (getline(filestream, line)) {
        if (line.compare(0, name.size(), name) == 0) {
            std::istringstream buf(line);
            std::istream_iterator<string> beg(buf), end;
            vector<string> values(beg, end);
            result += stoi(values[1]);
            break;
        }
    }

    return result;
}

// Read and return the command associated with a process
string LinuxParser::Command(int pid) {
    string line;
    std::ifstream filestream(kProcDirectory + std::to_string(pid) + kCmdlineFilename);
    getline(filestream, line);

    return line;
}

// Read and return the memory used by a process
string LinuxParser::Ram(int pid) {
    string line;
    string name = "VmData";
    string result;

    std::ifstream filestream(kProcDirectory + std::to_string(pid) + kStatusFilename);
    while (getline(filestream, line)) {
        if (line.compare(0, name.size(), name) == 0) {
            std::istringstream buf(line);
            std::istream_iterator<string> beg(buf), end;
            vector<string> values(beg, end);
            // conversion kB -> GB
            result = std::to_string((stof(values[1]) / float(1024)));
            break;
        }
    }

    return result;
}

// Read and return the user ID associated with a process
string LinuxParser::Uid(int pid) {
    string line;
    string name = "Uid:";
    string result;

    std::ifstream filestream(kProcDirectory + std::to_string(pid) + kStatusFilename);
    while (getline(filestream, line)) {
        if (line.compare(0, name.size(), name) == 0) {
            std::istringstream buf(line);
            std::istream_iterator<string> beg(buf), end;
            vector<string> values(beg, end);
            result = values[1];
        }
    }

    return result;
}

// Read and return the user associated with a process
string LinuxParser::User(int pid) {
    std::ifstream filestream(kPasswordPath);
    string name = ("x:" + Uid(pid));
    string result;
    string line;

    while (getline(filestream, line)) {
        if (line.find(name) != string::npos)
            result = line.substr(0, line.find(':'));
    }

    return result;
}

// Read and return the uptime of a process
long LinuxParser::UpTime(int pid) {
    string line;
    std::ifstream filestream(kProcDirectory + std::to_string(pid) + kStatFilename);
    std::getline(filestream, line);
    std::istringstream buf(line);
    std::istream_iterator<string> beg(buf), end;
    vector<string> values(beg, end);

    return long(stof(values[13])) / sysconf(_SC_CLK_TCK);
}
