#ifndef COMPILER_CODE_GEN_VARIABLE_TABLE_HPP
#define COMPILER_CODE_GEN_VARIABLE_TABLE_HPP

#include <fstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <list>
#include <iterator>

std::string makeLabel(const char* _name);

struct varInfo {
    long offset;
    int numBytes=0;  // number of bytes per element
    long length=0;  // number of elements
    int isFP=0;
    int isPtr=0;
    long initValue=0;
    std::string type="";
};

struct functionInfo {
    int argCount=0;
    std::string returnType="";
    std::vector<varInfo> argList;
};

struct VarLUT {
    long size=0;
    long slider=0;
    long FP=0;
    std::vector<std::unordered_map<std::string,varInfo>> lut;
};

struct Context {
    VarLUT stack;
    std::unordered_map<std::string,functionInfo> ftable;
    std::unordered_map<std::string,functionInfo>::iterator ftEntry;
    std::string LoopStartPoint="";
    std::string LoopEndPoint="";
    std::string BranchEndPoint="";
    std::string FuncRetnPoint="";
    varInfo tempVarInfo;
    std::list<std::string> Case_label;
    int isFunc=0;
    int isLoop=0;
    int isSwitch=0;
    long LoopInitSP=0;
    int ArgCount=0;
};


#endif