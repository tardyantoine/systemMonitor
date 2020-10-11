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
  string os, version, kernel;
  string line;
  std::ifstream stream(kProcDirectory + kVersionFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> os >> version >> kernel;
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
  float totalMem, freeMem;
  int nbFound = 0;
  string line;
  string key;
  string value;

  std::ifstream filestream(kProcDirectory + kMeminfoFilename);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "MemTotal:") {
          totalMem = std::stof(value);    
          nbFound++;
        }
        else if (key == "MemFree:") {
          freeMem = std::stof(value);
          nbFound++;
        }

        if(nbFound == 2) {
          return (totalMem - freeMem) / totalMem;
        }
      }
    }
  }
  return 0;
}

// Read and return the system uptime
long LinuxParser::UpTime() { 
  string uptime;
  string line;
  std::ifstream stream(kProcDirectory + kUptimeFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> uptime;
  }

  if(uptime != "") {
    return std::stol(uptime);
  }
  else {
    return 0;
  }
}

// Read and return CPU utilization
float LinuxParser::CpuUtilization() { 
  string cpu, a, b, c, i;
  string line;
  string key;
  string value;
  float total, idle;

  std::ifstream filestream(kProcDirectory + kStatFilename);
  if (filestream.is_open()) {
    std::getline(filestream, line);
    std::istringstream linestream(line);
    linestream >> cpu >> a >> b >> c >> i;
    
    idle = stof(i);
    total = stof(a) + stof(b) + stof(c) + idle;

    return (total - idle) / total;
  }
  return 0;
}

float LinuxParser::CpuUtilization(int pid) { 
  float uptime = UpTime();
  float starttime = UpTime(pid);
  float  ut, st, cut, cst;
  string line, val;

  std::ifstream stream(kProcDirectory + to_string(pid) + kStatFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    for (int i = 0; i < 17; ++i) {
      linestream >> val;
      
      switch(i) {
      case 13:
        ut = stof(val);
        break;

       case 14:
        st = stof(val);
        break;

       case 15:
        cut = stof(val);
        break;

       case 16:
        cst = stof(val);
        break;
      } 

    }
    return (ut + cut + st + cst) / sysconf(_SC_CLK_TCK) / (uptime - starttime/sysconf(_SC_CLK_TCK));
  }


  return 0; 
}


// Read and return the total number of processes
int LinuxParser::TotalProcesses() {
  string line;
  string key;
  string value;

  std::ifstream filestream(kProcDirectory + kStatFilename);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "processes") {
          return std::stoi(value);
        }
      }
    }
  }
  return 0; 
}

// Read and return the number of running processes
int LinuxParser::RunningProcesses() { 
  string line;
  string key, value;

  std::ifstream filestream(kProcDirectory + kStatFilename);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "procs_running") {
          return std::stoi(value);
        }
      }
    }
  }
  return 0; 
}

// Read and return the command associated with a process
string LinuxParser::Command(int pid) { 
  string line;
  string key, value;

  std::ifstream filestream(kProcDirectory + to_string(pid) + kCmdlineFilename);
  if (filestream.is_open()) {
    std::getline(filestream, line);
  }

  return line;  
}

// Read and return the memory used by a process
string LinuxParser::Ram(int pid) { 
  string line;
  string key, value;

  std::ifstream filestream(kProcDirectory + to_string(pid) + kStatusFilename);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "VmSize:") {
          return to_string(stoi(value) / 1000);
        }
      }
    }
  }
  return string(); 
}

// Read and return the user ID associated with a process
string LinuxParser::Uid(int pid[[maybe_unused]]) {
  string line;
  string key, value;

  std::ifstream filestream(kProcDirectory + to_string(pid) + kStatusFilename);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "Uid:") {
          return value;
        }
      }
    }
  }
  return string(); 
}

// Read and return the user associated with a process
string LinuxParser::User(int pid) { 
  string line;
  string usr, x, value;
  string uid = Uid(pid);

  std::ifstream filestream(kPasswordPath);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::replace(line.begin(), line.end(), ':', ' ');
      std::istringstream linestream(line);
      while (linestream >> usr >> x >> value) {
        if (value == uid) {
          return usr;
        }
      }
    }
  }
  return string(); 
}

// Read and return the uptime of a process
long LinuxParser::UpTime(int pid) { 
  string uptime;
  string line;
  std::ifstream stream(kProcDirectory + to_string(pid) + kStatFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    for (int i = 0; i < 22; ++i) {
      linestream >> uptime;
    }
  }

  if(uptime != "") {
    return UpTime() - std::stol(uptime) / sysconf(_SC_CLK_TCK);
  }
  else {
    return 0;
  }
}