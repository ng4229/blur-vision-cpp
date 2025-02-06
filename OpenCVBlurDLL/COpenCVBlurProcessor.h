#pragma once
#include <opencv2/opencv.hpp>
#include "IImageProcessor.h"

class COpenCVBlurProcessor : public IImageProcessor
{
public:
	~COpenCVBlurProcessor() {};
	bool ImageBlur(const ImageObject* src, ImageObject* dst, const int kernelSize) override;

};

