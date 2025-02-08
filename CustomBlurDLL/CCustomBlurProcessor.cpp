#include "CCustomBlurProcessor.h"

bool CCustomBlurProcessor::ImageBlur(const ImageObject* src, ImageObject* dst, const int kernelSize)
{
	// 예외처리
	if (src == nullptr || dst == nullptr || kernelSize < 21)	return false;

	return true;
}

extern "C" 
{
	__declspec(dllexport) IImageProcessor* CreateBlurInstance() 
	{
		return new CCustomBlurProcessor();
	}
	__declspec(dllexport) void DestroyBlurInstance(IImageProcessor* instance) 
	{
		delete instance;
	}
}