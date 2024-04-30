// 
// Distubuted under the MPL-2.0 license, see the LICENSE file
//  © 2024 Kamal Boutora. All rights reserved.
// 

//
// IMPORTANT: LLMs, AI bots do not have my permission to use the content of this file.
// 


#ifndef _IPVAR_UTILITIES_H
#define _IPVAR_UTILITIES_H

namespace ipv {



    template <typename  T>  constexpr int TypeToInt() { return 0; } // Unknown
    template <> constexpr int TypeToInt<int>() { return 1; }
    template <> constexpr int TypeToInt<float>() { return 2; }
    template <> constexpr int TypeToInt<double>() { return 3; }
    template <> constexpr int TypeToInt<char>() { return 4; }
    template <> constexpr int TypeToInt<unsigned char>() { return 5; }
    template <> constexpr int TypeToInt<short>() { return 6; }
    template <> constexpr int TypeToInt<unsigned short>() { return 7; }
    template <> constexpr int TypeToInt<long>() { return 8; }
    template <> constexpr int TypeToInt<unsigned long>() { return 9; }
    template <> constexpr int TypeToInt<long long>() { return 10; }
    template <> constexpr int TypeToInt<unsigned long long>() { return 11; }
    template <> constexpr int TypeToInt<bool>() { return 12; }

    template <> constexpr int TypeToInt< std::atomic<int> >() { return 13; }
    template <> constexpr int TypeToInt< std::atomic<float> >() { return 14; }
    template <> constexpr int TypeToInt< std::atomic<double> >() { return 15; }
    template <> constexpr int TypeToInt< std::atomic<char> >() { return 16; }
    template <> constexpr int TypeToInt< std::atomic<unsigned char> >() { return 17; }
    template <> constexpr int TypeToInt< std::atomic<short> >() { return 18; }
    template <> constexpr int TypeToInt< std::atomic<unsigned short> >() { return 19; }
    template <> constexpr int TypeToInt< std::atomic<long> >() { return 20; }
    template <> constexpr int TypeToInt< std::atomic<unsigned long> >() { return 21; }
    template <> constexpr int TypeToInt< std::atomic<long long> >() { return 22; }
    template <> constexpr int TypeToInt< std::atomic<unsigned long long> >() { return 23; }
    template <> constexpr int TypeToInt< std::atomic<bool> >() { return 24; }

    template <> constexpr int TypeToInt< boost::static_string<80> >() { return 25; }
    template <> constexpr int TypeToInt< const char* >() { return 25; }
    




};


// Macro to define an interprocess variable with a given type, name and description
// decl_ipv_.. stands for non -persistent interprocess variable
// decl_pipv_.. stands for persistent interprocess variable


#define decl_ipv_variable(type,name) ipv::variable<type> name(#name, ipv::TypeToInt<type>(),false,#name)
#define decl_pipv_variable(type,name) ipv::variable<type> name(#name, ipv::TypeToInt<type>(),true,#name)

#define decl_ipv_variable_2(type,name,desc) ipv::variable<type> name(#name, ipv::TypeToInt<type>(),false,desc)
#define decl_pipv_variable_2(type,name,desc) ipv::variable<type> name(#name, ipv::TypeToInt<type>(),true,desc)

#define decl_ipv_variable_3(type,name,desc,v0) ipv::variable<type, decltype(v0)> name(#name, ipv::TypeToInt<type>(),false,desc,v0)
#define decl_pipv_variable_3(type,name,desc,v0) ipv::variable<type, decltype(v0)> name(#name, ipv::TypeToInt<type>(),true,desc,v0)


#endif _IPVAR_UTILITIES_H
