#pragma once
#include "IImageProcessor.h"

class CCustomBlurProcessor: public IImageProcessor
{
public:
	~CCustomBlurProcessor() {};
	bool ImageBlur(const ImageObject* src, ImageObject* dst, const int kernelSize) override;
};

