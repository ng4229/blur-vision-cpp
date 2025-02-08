#pragma once
#include "ConfigUtil.h"
#include <map>

class CConfigManager : public CConfigUtil
{
private:
    std::map<std::string, std::map<std::string, std::string>> m_mapStrValue;
    std::map<std::string, std::map<std::string, int>> m_mapIntValue;

public:
    CConfigManager();

    std::string getStrValue(const std::string& section, const std::string& key) { return m_mapStrValue[section][key]; }
    int         getIntValue(const std::string& section, const std::string& key) { return m_mapIntValue[section][key]; }
};

