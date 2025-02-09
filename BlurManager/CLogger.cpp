#include "pch.h"
#include "CLogger.h"

std::ofstream CLogger::logFile;
std::mutex CLogger::logMtx;