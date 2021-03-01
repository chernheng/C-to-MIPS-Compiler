#include <unordered_map>
#include <string>

std::unordered_map<std::string,long> labelNameList;

std::string makeLabel(const char* _name) {
    std::string name = std::string(_name);
    std::unordered_map<std::string,long>::iterator it;
    it=labelNameList.find(name);
    if(it==labelNameList.end())    {
        labelNameList.insert(std::pair<std::string,long>(name,1));
        std::string outString = "_"+name+std::to_string(0)+"_";
        return outString;
    }
    else    {
        std::string outString = "_"+name+std::to_string(it->second)+"_";
        it->second++;
        return outString;   
    }
}
