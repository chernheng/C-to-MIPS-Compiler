#ifndef COMPILER_CODE_GEN_VARIABLE_TABLE_HPP
#define COMPILER_CODE_GEN_VARIABLE_TABLE_HPP

#include <fstream>
#include <string>
#include <vector>
#include <unordered_map>

struct varInfo {
    long offset;
    int numBytes;  // number of bytes per element
    long length;  // number of elements
    int isFP=0;
    std::string type="";
};

struct VarLUT {
    long size=0;
    std::vector<std::unordered_map<std::string,varInfo>> lut;
};

struct Context {
    VarLUT stack;
    std::string startPoint="";
    std::string endPoint="";
};


#endif