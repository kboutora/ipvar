// 
// Distubuted under the MPL-2.0 license, see the LICENSE file
//  © 2024 Kamal Boutora. All rights reserved.
// 

#include "../ipvar/ipvar.h"

#include <iostream>


void ShowIPVars()
{
    std::vector<boost::tuple<std::string, std::string, int, void*>> variablesInfo;

    ipv::SharedMemoryManager::GetInstance().ListAllVariables(variablesInfo);
    for (const auto& varInfo : variablesInfo) {
        std::cout << "Variable name: " << boost::get<0>(varInfo) << "  Description: " << boost::get<1>(varInfo) << "  Type: " << boost::get<2>(varInfo) << std::endl;
    }
}


int main() {
    ShowIPVars();
    return 0;
}



