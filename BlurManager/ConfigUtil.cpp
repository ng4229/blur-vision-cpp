#include "pch.h"
#include "ConfigUtil.h"
#include <filesystem>

namespace fs = std::filesystem;
using namespace CONFIG;

CConfigUtil::CConfigUtil(std::string strIniPath) : m_strIniPath(strIniPath)
{
    // 파일이 없으면 기본 설정으로 생성
    if (!fs::exists(m_strIniPath)) {  // Modern C++17 방식으로 파일 존재 여부 확인
        setValue(SECTION_SYSTEM, SYSTEM_LOAD_IMAGE_PATH, DEFAULT_LOAD_IMAGE_PATH);
        setValue(SECTION_SYSTEM, SYSTEM_KERNEL_SIZE, (int)21);
        setValue(SECTION_SYSTEM, SYSTEM_SAVE_IMAGE_PATH, DEFAULT_SAVE_IMAGE_PATH);
    }
}

std::string CConfigUtil::getValue(const std::string& section, const std::string& key, const std::string& defaultValue /*= ""*/)
{
    char buffer[256];
    GetPrivateProfileStringA(section.c_str(), key.c_str(), defaultValue.c_str(), buffer, sizeof(buffer), m_strIniPath.c_str());
    return std::string(buffer);
}

int CConfigUtil::getValue(const std::string& section, const std::string& key, const int defaultValue /*= 0*/)
{
    return GetPrivateProfileIntA(section.c_str(), key.c_str(), defaultValue, m_strIniPath.c_str());
}

void CConfigUtil::setValue(const std::string& section, const std::string& key, const std::string& value)
{
    WritePrivateProfileStringA(section.c_str(), key.c_str(), value.c_str(), m_strIniPath.c_str());
}

void CConfigUtil::setValue(const std::string& section, const std::string& key, const int value)
{
    WritePrivateProfileStringA(section.c_str(), key.c_str(), std::to_string(value).c_str(), m_strIniPath.c_str());
}
