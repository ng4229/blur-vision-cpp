#pragma once
#include "IImageProcessor.h"

class CCustomBlurProcessor: public IImageProcessor
{
public:
	~CCustomBlurProcessor() {};
	bool ImageBlur(const ImageObject* src, ImageObject* dst, const int kernelSize) override;
};

extern "C" 
{
	__declspec(dllexport) IImageProcessor* CreateBlurInstance();
	__declspec(dllexport) void DestroyBlurInstance(IImageProcessor* instance);
}


