

// Copyright (c) 2019-2021, Kamal Boutora
// Distubuted under the MPL-2.0 license, see the LICENSE file

#include <iostream>
#include <thread>
#include <chrono>
#include "../ipvar/ipvar.h"
#include "../ipvar/ipvar_util.h"

#include "SharedStructs.h"


// This program is a simple example of how to use the Ipvar class.
// It will monitor the content of all the interprocess variables each 2 seconds, tries to dump the content.
// Check the SharedStructs.h file for the implementation for custom structures.


void GetAllManagedVariables(std::vector<boost::tuple<std::string, std::string, int, void*>>& variablesInfo) {
    ipv::SharedMemoryManager::GetInstance().ListAllVariables(variablesInfo);
}



#define SUPPORT_TYPE_T(_TYPENAME) case ipv::TypeToInt< _TYPENAME>(): return std::to_string(*static_cast<_TYPENAME*>(vContent)); break
std::string GetVariableToString(int codeVariable, void* vContent) {

    switch (codeVariable)
    {

    case ipv::TypeToInt<float>(): return std::to_string(*static_cast<float*>(vContent)); break;
    case ipv::TypeToInt<double>(): return std::to_string(*static_cast<double*>(vContent)); break;
    case ipv::TypeToInt<char>(): return std::to_string(*static_cast<char*>(vContent)); break;
    case ipv::TypeToInt<unsigned char>(): return std::to_string(*static_cast<unsigned char*>(vContent)); break;
    case ipv::TypeToInt<short>(): return std::to_string(*static_cast<short*>(vContent)); break;
    case ipv::TypeToInt<unsigned short>(): return std::to_string(*static_cast<unsigned short*>(vContent)); break;
    case ipv::TypeToInt<long>(): return std::to_string(*static_cast<long*>(vContent)); break;
    case ipv::TypeToInt<unsigned long>(): return std::to_string(*static_cast<unsigned long*>(vContent)); break;
    case ipv::TypeToInt<long long>(): return std::to_string(*static_cast<long long*>(vContent)); break;
    case ipv::TypeToInt<unsigned long long>(): return std::to_string(*static_cast<unsigned long long*>(vContent)); break;
    case ipv::TypeToInt<bool>(): return std::to_string(*static_cast<bool*>(vContent)); break;
    case ipv::TypeToInt< std::atomic<int> >(): return std::to_string(*static_cast<std::atomic<int>*>(vContent)); break;
    case ipv::TypeToInt< std::atomic<float> >(): return std::to_string(*static_cast<std::atomic<float>*>(vContent)); break;
    case ipv::TypeToInt< std::atomic<double> >(): return std::to_string(*static_cast<std::atomic<double>*>(vContent)); break;
    case ipv::TypeToInt< std::atomic<char> >(): return std::to_string(*static_cast<std::atomic<char>*>(vContent)); break;
    case ipv::TypeToInt< std::atomic<unsigned char> >(): return std::to_string(*static_cast<std::atomic<unsigned char>*>(vContent)); break;
    case ipv::TypeToInt< std::atomic<short> >(): return std::to_string(*static_cast<std::atomic<short>*>(vContent)); break;
    case ipv::TypeToInt< std::atomic<unsigned short> >(): return std::to_string(*static_cast<std::atomic<unsigned short>*>(vContent)); break;
    case ipv::TypeToInt< std::atomic<long> >(): return std::to_string(*static_cast<std::atomic<long>*>(vContent)); break;
    case ipv::TypeToInt< std::atomic<unsigned long> >(): return std::to_string(*static_cast<std::atomic<unsigned long>*>(vContent)); break;
    case ipv::TypeToInt< std::atomic<long long> >(): return std::to_string(*static_cast<std::atomic<long long>*>(vContent)); break;
    case ipv::TypeToInt< std::atomic<unsigned long long> >(): return std::to_string(*static_cast<std::atomic<unsigned long long>*>(vContent)); break;
    case ipv::TypeToInt< std::atomic<bool> >(): return std::to_string(*static_cast<std::atomic<bool>*>(vContent)); break;
    case ipv::TypeToInt< boost::static_string<80> >(): return (const char*)vContent; break;


        // Extendede types: check the SharedStructs.h file for the implementation
    case ipv::TypeToInt< SharedStructExample>(): return ipv::try_to_string<SharedStructExample>(*static_cast<SharedStructExample*>(vContent)); break;
    case ipv::TypeToInt< SharedStructExample2>(): return std::to_string(*static_cast<SharedStructExample2*>(vContent)); break;


    default:
        return "Unknown";
    }
}


int main()
{
    std::vector<boost::tuple<std::string, std::string, int, void*>> variablesInfo;

    decl_ipv_variable(SharedStructExample, customStructVariableUsingAsString);
    decl_ipv_variable(SharedStructExample2, customStructVariableUsingStdToString);
    try {
        while (true) {
            GetAllManagedVariables(variablesInfo);
            for (const auto& varInfo : variablesInfo) {

                // ipv::try_to_string(*static_cast<int*>(boost::get<3>(varInfo)))
                std::cout << "Variable name: " << boost::get<0>(varInfo) << "  Description: " << boost::get<1>(varInfo) << "  Type: " << boost::get<2>(varInfo) << "  Value: " << GetVariableToString(boost::get<2>(varInfo), boost::get<3>(varInfo)) << std::endl;
            }
            std::cout << "-----------------------------------" << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(2000));
        }
    }
    catch (std::exception& e) {
        std::cout << "Error: " << e.what() << std::endl;
    }
    catch (...) {
        std::cout << "Unknown error" << std::endl;
    }
}

