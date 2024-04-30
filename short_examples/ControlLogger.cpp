
// 
// Distubuted under the MPL-2.0 license, see the LICENSE file
//  © 2024 Kamal Boutora. All rights reserved.
// 

#include "../ipvar/ipvar.h"
#include "../ipvar/ipvar_util.h"
#include <iostream>

// Description: This program is a simple example of how to use the Ipvar class.
// You can change the logger level at runtime using the ControlLogger program.
// You can stop the LoggerExample program using the keyword "stop" as an argument in the command line.

int main(int argc, char** argv)
{
    decl_ipv_variable_3(std::atomic<int>, loggerLevel, "Logger level (TRACE=0.. ERROR=4)", 0);
    decl_ipv_variable_3(bool, stopRunning, "Stop Running the demo", false);

    if (argc != 2) {
        std::cout << "Usage: " << argv[0] << " <logger level>  : to change logging level (0: TRACE .. 4:ERROR)" << std::endl;
        std::cout << "Usage: " << argv[0] << " stop   : to stop the remote program" << std::endl;
        return 1;
    }
    // set the logger level
    if (std::string(argv[1]) == "stop") {
        *stopRunning = true;
        std::cout << "Stopping the logger" << std::endl;
    }
    else {
        std::cout << "Setting logger level to " << argv[1] << std::endl;
        try
        {
            *loggerLevel = std::stoi(argv[1]);
        }
        catch (const std::exception&)
        {
            std::cout << "Invalid logger level: " << argv[1] << std::endl;
        }
    }

    return 0;
}
