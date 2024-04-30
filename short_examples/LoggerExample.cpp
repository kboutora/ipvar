// 
// Distubuted under the MPL-2.0 license, see the LICENSE file
//  © 2024 Kamal Boutora. All rights reserved.
// 

#include <thread>
#include <iostream>
#include <chrono>
#include "../ipvar/ipvar.h"
#include "../ipvar/ipvar_util.h"

// Description: This program is a simple example of how to use the Ipvar class.
// It periodically logs messages based on the logger level.
// The logger level can be changed at runtime using the ControlLogger program.
// The program will stop when the stopRunning variable is set to true.
// Use VariablesMonitor to see the variables in the shared memory.



decl_ipv_variable_3(std::atomic<int>, loggerLevel, "Logger level (TRACE=0.. ERROR=4)", 0);
decl_ipv_variable_3(bool, stopRunning, "Stop Running the demo", false);

int main(int, char**)
{

    while (!*stopRunning)
    {
        if (*loggerLevel <= 0) std::cout << "TRACE: This is a trace message" << std::endl;
        if (*loggerLevel <= 1) std::cout << "DEBUG: This is a debug message" << std::endl;
        if (*loggerLevel <= 2) std::cout << "INFO: This is an info message" << std::endl;
        if (*loggerLevel <= 3) std::cout << "WARN: This is a warning message" << std::endl;
        if (*loggerLevel <= 4) std::cout << "ERROR: This is an error message" << std::endl;
        std::cout << "-----------------------------------\n";
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    std::cout << "Stop instruction received" << std::endl;
    return 0;
}