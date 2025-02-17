#include "pch.h"
#include "ConfigManager.h"

using namespace CONFIG;

CConfigManager::CConfigManager() :CConfigUtil(CONFIG::CONFIG_FOLDER + CONFIG::CONFIG_FILENAME)
{
	m_mapStrValue[SECTION_SYSTEM][SYSTEM_LOAD_IMAGE_PATH] = getValue(SECTION_SYSTEM, SYSTEM_LOAD_IMAGE_PATH, DEFAULT_LOAD_IMAGE_PATH);
	m_mapIntValue[SECTION_SYSTEM][SYSTEM_KERNEL_SIZE] = getValue(SECTION_SYSTEM, SYSTEM_KERNEL_SIZE, (int)21);
	m_mapStrValue[SECTION_SYSTEM][SYSTEM_SAVE_IMAGE_PATH] = getValue(SECTION_SYSTEM, SYSTEM_SAVE_IMAGE_PATH, DEFAULT_SAVE_IMAGE_PATH);
}

