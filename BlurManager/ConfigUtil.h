#pragma once
#include <windows.h>
#include <string>

namespace CONFIG {
	// path
	const static std::string CONFIG_FOLDER = ".\\";
	const static std::string CONFIG_FILENAME = "config.ini";

	const static std::string DEFAULT_LOAD_IMAGE_PATH = "..\\LoadImage\\";
	const static std::string DEFAULT_SAVE_IMAGE_PATH = "..\\SaveImage\\";

	// System
	const static std::string SECTION_SYSTEM = "System";
	const static std::string SYSTEM_LOAD_IMAGE_PATH = "Load image path";
	const static std::string SYSTEM_KERNEL_SIZE = "Kernel size";
	const static std::string SYSTEM_SAVE_IMAGE_PATH = "Save image path";

}

// Section, Key, Value
// �ҷ��� �̹��� ���(string), Ŀ�� ������(int), ������ �̹��� ���(string)
class CConfigUtil
{
private:
	std::string m_strIniPath;

public:
	CConfigUtil() {};
	CConfigUtil(std::string strIniPath);

	// �� �ҷ�����
	std::string getValue(const std::string& section, const std::string& key, const std::string& defaultValue = "");
	int getValue(const std::string& section, const std::string& key, const int defaultValue = 0);

	// �� ����
	void setValue(const std::string& section, const std::string& key, const std::string& value);
	void setValue(const std::string& section, const std::string& key, const int value);
};

