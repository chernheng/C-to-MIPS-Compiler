#ifndef COMPILER_CODE_GEN_VARIABLE_TABLE_HPP
#define COMPILER_CODE_GEN_VARIABLE_TABLE_HPP

#include <fstream>
#include <string>
#include <vector>
#include <unordered_map>

std::string makeLabel(const char* _name);

struct varInfo {
    long offset;
    int numBytes;  // number of bytes per element
    long length;  // number of elements
    int isFP=0;
    std::string type="";
};

struct VarLUT {
    long size=0;
    long slider=0;
    std::vector<std::unordered_map<std::string,varInfo>> lut;
};

struct Context {
    VarLUT stack;
    std::string LoopStartPoint="";
    std::string LoopEndPoint="";
    std::string BranchEndPoint="";
    std::string FuncRetnPoint="";
    int isFunc=0;
    int isLoop=0;
    long LoopInitSP=0;
    long LoopInitSL=0;
    int LoopScopeCount=0;
};


#endif