#ifndef SHARED_STRUCTS_H
#define SHARED_STRUCTS_H

#include <string>

struct SharedStructExample {
    int a = 1;
    int b = 2;
    int c = 3;
    std::string AsString() {
        return "a: " + std::to_string(a) + " b: " + std::to_string(b) + " c: " + std::to_string(c);
    }
};



struct SharedStructExample2 {
    int a = 5;
    int b = 6;
    int c = 7;
};
namespace std {
    std::string to_string(const SharedStructExample2& obj) {
        return "a: " + std::to_string(obj.a) + " b: " + std::to_string(obj.b) + " c: " + std::to_string(obj.c);
    }
};

template <> constexpr int ipv::TypeToInt< SharedStructExample>() { return 26; }
template <> constexpr int ipv::TypeToInt< SharedStructExample2>() { return 27; }
#endif
