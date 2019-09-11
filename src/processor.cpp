#include "processor.h"
#include "linux_parser.h"

// Return the aggregate CPU utilization
float Processor::Utilization() {
    float result = (float) LinuxParser::ActiveJiffies() / LinuxParser::Jiffies();

    return result;
}