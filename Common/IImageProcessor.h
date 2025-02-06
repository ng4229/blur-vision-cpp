#pragma once
#include "ImageObject.h"

class IImageProcessor
{
	virtual ~IImageProcessor() = default;
	virtual bool ImageBlur(const ImageObject* src, ImageObject* dst, const int kernelSize) = 0;
};
